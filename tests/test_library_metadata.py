"""library.properties (Arduino) and library.json (PlatformIO) describe the same release."""
import json
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def properties():
    out = {}
    for line in (ROOT / "library.properties").read_text().splitlines():
        if "=" in line and not line.lstrip().startswith("#"):
            key, _, value = line.partition("=")
            out[key.strip()] = value.strip()
    return out


class LibraryMetadataTests(unittest.TestCase):
    def setUp(self):
        self.props = properties()
        self.json = json.loads((ROOT / "library.json").read_text())

    def test_same_name(self):
        self.assertEqual(self.json["name"], self.props["name"])

    def test_same_version(self):
        self.assertEqual(self.json["version"], self.props["version"])

    def test_same_licence(self):
        self.assertEqual(self.json["license"], self.props["license"])

    def test_header_exists(self):
        self.assertTrue((ROOT / "src" / self.json["headers"]).is_file())

    def test_platforms_match_architectures(self):
        to_platform = {"avr": "atmelavr", "esp32": "espressif32", "esp8266": "espressif8266"}
        architectures = [a.strip() for a in self.props["architectures"].split(",")]
        self.assertEqual(sorted(self.json["platforms"]), sorted(to_platform[a] for a in architectures))


if __name__ == "__main__":
    unittest.main()
