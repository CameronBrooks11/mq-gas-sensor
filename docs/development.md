# Development

The repository's tasks are [`just`](https://github.com/casey/just) recipes; `just --list` shows them.

| Recipe | What it does | Needs |
|---|---|---|
| `just setup` | Installs the board cores and libraries the examples need, at pinned versions | `arduino-cli` |
| `just check` | Checks tracked assets and Markdown links, checks the coefficients page is current, and compiles every example for its board | `arduino-cli`, `uv` |
| `just test` | Runs the library's unit tests on a simulated Mega 2560, then the scripts' and curve tracer's tests | `simavr`, `uv` |
| `just coefficients` | Regenerates [Calibration coefficients](coefficients.md) from the committed points and the examples | `uv` |
| `just datasheets` | Downloads the datasheets in the [manifest](../datasheets/README.md), checking each by SHA-256 | Python 3.11+ |
| `just reproduce` | Downloads the datasheets, re-derives every traced curve and compares it with the committed points | `uv` |
| `just docs` | Builds this site into `site/`, failing on any warning | `uv` |
| `just serve` | Serves this site at http://127.0.0.1:8000 while editing | `uv` |

## Continuous integration

- **Check**, on every pull request and push to main: `just setup`, `just check` and `just test`. A change that breaks an example, a unit test, a link, or the coefficients page fails here. Two more jobs check the library as the registries see it: `lint` runs `arduino-lint` with the Arduino Library Manager's rules for a new submission, and `platformio` builds an example through `library.json` for an Uno, an ESP32 and an ESP8266.
- **Datasheet mirrors**, monthly and on pull requests that touch the datasheets or the curve tracer: tests every mirror of every datasheet, and re-derives the curves from the downloaded files.
- **Docs site**, on every pull request and push to main: builds this site, and deploys it to GitHub Pages from main.

## The unit tests

[`examples/UnitTesting`](../examples/UnitTesting) is an ArduinoUnit sketch. [`scripts/test.sh`](../scripts/test.sh) builds it for the Mega 2560, runs the firmware in simavr, and reads the result from the simulated serial port. The tests cover the curve for every sensor, the ratio modes, calibration, the correction factor, the supply offset, and 0 V handling.

## This site

The pages are in [`docs/`](../docs). Three things are generated when the site is built, by [`scripts/mkdocs_hooks.py`](../scripts/mkdocs_hooks.py):

- [Datasheets](../datasheets/README.md) and [Curve tracing](../analysis/README.md) are `datasheets/README.md` and `analysis/README.md`, so each is written once, beside what it describes.
- The table on [Sensors](sensors.md) comes from the committed coefficients page and the example sketches.
- Links from a page to a file outside `docs/` become links to that file on GitHub.

The build also fails if [API](api.md) and `src/MQGasSensor.h` disagree about the public methods, or [Examples](examples.md) and `examples/` disagree about the sketches. [Calibration coefficients](coefficients.md) is published as `just coefficients` generated it.

## Releasing

The version is in `library.properties` and `library.json`, following [Semantic Versioning](https://semver.org/); a test fails when the two disagree. A release is a tag `vX.Y.Z` on main with a GitHub release whose notes say what changed for users.
