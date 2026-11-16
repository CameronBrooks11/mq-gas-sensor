"""MkDocs hooks for the docs site.

- Publishes datasheets/README.md and analysis/README.md as the pages
  datasheets.md and curve-tracing.md, so they are written once, beside what they
  describe, and still appear on the site.
- Rewrites every link that leaves docs/ (into analysis/, datasheets/, examples/,
  src/ ...) to the file on GitHub, so pages keep linking to the repository the
  way they do when read there, and a link to a file that does not exist fails
  the build.
- Fills the sensors table from the repository, at a line of its own:
    <!-- table: sensors -->   one row per sensor in the fitted-curves table of
                              docs/coefficients.md, with its gases and examples
- Fails the build when api.md and src/MQGasSensor.h disagree about the public
  methods, or examples.md and examples/ disagree about the sketches; links each
  sketch named in examples.md to its folder.
"""
import os
import re

from mkdocs.exceptions import PluginError
from mkdocs.structure.files import File

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DOCS = os.path.join(ROOT, "docs")
BLOB = "https://github.com/CameronBrooks11/mq-gas-sensor/blob/main/"
TREE = "https://github.com/CameronBrooks11/mq-gas-sensor/tree/main/"

# Pages whose source lives outside docs/: site page -> file in the repository.
IMPORTED = {
    "datasheets.md": "datasheets/README.md",
    "curve-tracing.md": "analysis/README.md",
}
MARKER = re.compile(r"^<!-- table: (\S+) -->$", re.M)
# [text](target) and ![alt](target); the target ends at whitespace or ")".
LINK = re.compile(r"(!?\[[^\]]*\]\()([^)\s]+)(\))")


def fail(msg):
    raise PluginError(msg)


# ---- pages from outside docs/ ---------------------------------------------------

def on_files(files, config):
    for page, source in IMPORTED.items():
        with open(os.path.join(ROOT, source), encoding="utf-8") as f:
            files.append(File.generated(config, page, content=f.read()))
    return files


def source_of(page):
    """The page's file in the repository, relative to ROOT."""
    return IMPORTED.get(page.file.src_uri, "docs/" + page.file.src_uri)


# ---- links that leave docs/ ------------------------------------------------------

def rewrite(target, source):
    if re.match(r"^[a-z][a-z0-9+.-]*:", target) or target.startswith(("#", "/")):
        return target
    path, _, anchor = target.partition("#")
    if not path:
        return target
    repo_path = os.path.normpath(os.path.join(os.path.dirname(source), path))
    if repo_path.startswith(".."):
        fail(f"{source}: link {target} leaves the repository")
    anchor = "#" + anchor if anchor else ""
    for page, imported in IMPORTED.items():
        if repo_path == imported:
            return page + anchor
    if repo_path.startswith("docs" + os.sep):
        # Every page sits at the top of the site, so a link is relative to docs/.
        return os.path.relpath(repo_path, "docs") + anchor
    full = os.path.join(ROOT, repo_path)
    if os.path.isdir(full):
        return TREE + repo_path + anchor
    if os.path.isfile(full):
        return BLOB + repo_path + anchor
    fail(f"{source}: link {target} names {repo_path}, which does not exist")


def on_page_markdown(markdown, page, config, files):
    source = source_of(page)
    out, fence = [], False
    for line in markdown.split("\n"):
        if line.lstrip().startswith("```"):
            fence = not fence
        if not fence:
            line = LINK.sub(lambda m: m.group(1) + rewrite(m.group(2), source) + m.group(3), line)
        out.append(line)
    markdown = "\n".join(out)

    def table(m):
        name = m.group(1)
        if name not in TABLES:
            fail(f"{source}: unknown table {name}")
        return TABLES[name](markdown)

    markdown = MARKER.sub(table, markdown)
    if page.file.src_uri == "api.md":
        check_api(markdown)
    if page.file.src_uri == "examples.md":
        markdown = link_examples(markdown)
    return markdown


# ---- the API page ------------------------------------------------------------------

def public_api():
    """Names of the public methods and constructors in src/MQGasSensor.h, and enum values."""
    with open(os.path.join(ROOT, "src", "MQGasSensor.h"), encoding="utf-8") as f:
        text = f.read()
    body = text[text.index("class MQGasSensor"):]
    public = body[body.index("public:"):body.index("private:")]
    public = re.sub(r"//[^\n]*", "", public)
    methods = set(re.findall(r"\b([A-Za-z_]\w*)\s*\(", public))
    enums = {}
    for name, values in re.findall(r"enum class (\w+)\s*:\s*\w+\s*\{([^}]*)\}", text):
        values = re.sub(r"//[^\n]*", "", values)
        enums[name] = [v.strip() for v in values.split(",") if v.strip()]
    return methods, enums


def check_api(markdown):
    methods, enums = public_api()
    documented = set(re.findall(r"^#{3,4} `([A-Za-z_]\w*)\(", markdown, re.M))
    missing, extra = methods - documented, documented - methods
    if missing or extra:
        fail("api.md and src/MQGasSensor.h disagree:"
             + (f" not documented: {sorted(missing)}." if missing else "")
             + (f" not in the header: {sorted(extra)}." if extra else ""))
    for enum, values in enums.items():
        for v in values:
            if f"{enum}::{v}" not in markdown:
                fail(f"api.md does not mention {enum}::{v}")


# ---- generated tables ------------------------------------------------------------

def sketches():
    """Each example sketch directory, relative to examples/, as CI finds them."""
    found = set()
    for dirpath, _, names in os.walk(os.path.join(ROOT, "examples")):
        if any(n.endswith(".ino") for n in names):
            found.add(os.path.relpath(dirpath, os.path.join(ROOT, "examples")))
    return sorted(found, key=str.lower)


def sensor_examples():
    """{sensor name: [sketch]} from the names the sketches give their sensors."""
    out = {}
    for sketch in sketches():
        if sketch == "UnitTesting":
            continue  # it builds every sensor to test the library, not to show one
        path = os.path.join(ROOT, "examples", sketch, os.path.basename(sketch) + ".ino")
        text = open(path, encoding="utf-8", errors="replace").read()
        names = set(re.findall(r'^#define\s+[Tt]ype\s+\(?"(MQ-\w+)"', text, re.M))
        names |= set(re.findall(r'^MQGasSensor \w+\([^;]*"(MQ-\w+)"\);', text, re.M))
        for n in names:
            out.setdefault(n, []).append(sketch)
    return out


def fitted_curves():
    """[(sensor, gas)] from the fitted-curves table of the committed coefficients page."""
    text = open(os.path.join(DOCS, "coefficients.md"), encoding="utf-8").read()
    section = text[text.index("## Fitted curves"):]
    section = section[:section.index("\n## ", 1)]
    rows = []
    for line in section.splitlines():
        cells = [c.strip() for c in line.strip().strip("|").split("|")]
        if len(cells) > 2 and cells[0].startswith("MQ-"):
            rows.append((cells[0], cells[1]))
    if not rows:
        fail("no fitted curves found in docs/coefficients.md")
    return rows


def sensor_key(name):
    m = re.match(r"MQ-(\d+)(\w*)", name)
    return (int(m.group(1)), m.group(2)) if m else (10**6, name)


def sensors_table(_markdown):
    gases = {}
    for sensor, gas in fitted_curves():
        gases.setdefault(sensor, [])
        if gas not in gases[sensor]:
            gases[sensor].append(gas)
    examples = sensor_examples()
    rows = ["| Sensor | Gases with a fitted curve | Examples |", "|---|---|---|"]
    for sensor in sorted(gases, key=sensor_key):
        ex = ", ".join(f"[{s}]({TREE}examples/{s})" for s in examples.get(sensor, [])) or "none yet"
        rows.append(f"| {sensor} | {', '.join(sorted(gases[sensor], key=str.lower))} | {ex} |")
    return "\n".join(rows)


EXAMPLE = re.compile(r"^- `([^`]+)`: ", re.M)


def link_examples(markdown):
    described = EXAMPLE.findall(markdown)
    found = sketches()
    missing = [s for s in found if s not in described]
    stale = [s for s in described if s not in found]
    twice = sorted({s for s in described if described.count(s) > 1})
    if missing or stale or twice:
        fail("examples.md and examples/ disagree:"
             + (f" no description for {missing}." if missing else "")
             + (f" no sketch for {stale}." if stale else "")
             + (f" described twice: {twice}." if twice else ""))
    return EXAMPLE.sub(lambda m: f"- [`{m.group(1)}`]({TREE}examples/{m.group(1)}): ", markdown)


TABLES = {"sensors": sensors_table}
