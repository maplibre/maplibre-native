#!/usr/bin/env python3
"""Exercise the archive checker with small, real iOS binaries and dSYMs."""

import plistlib
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


def run(*args):
    return subprocess.check_output([str(arg) for arg in args], text=True).strip()


def write_plist(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("wb") as stream:
        plistlib.dump(value, stream)


class XCFrameworkSymbolsTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.workspace = tempfile.TemporaryDirectory(prefix="xcframework-symbols-test-")
        cls.addClassCleanup(cls.workspace.cleanup)
        cls.directory = Path(cls.workspace.name)
        source = cls.directory / "sample.c"
        source.write_text("int sample(int value) { return value + 1; }\n")
        libraries = []
        for variant, sdk, architectures in [
            ("device", "iphoneos", ["arm64"]),
            ("simulator", "iphonesimulator", ["arm64", "x86_64"]),
        ]:
            identifier = "ios-" + "_".join(architectures)
            if variant == "simulator":
                identifier += "-simulator"
            library = {
                "LibraryIdentifier": identifier,
                "LibraryPath": "MapLibre.framework",
                "SupportedPlatform": "ios",
                "SupportedArchitectures": architectures,
                "DebugSymbolsPath": "dSYMs",
            }
            if variant == "simulator":
                library["SupportedPlatformVariant"] = variant
            libraries.append(library)
            sdk_path = run("xcrun", "--sdk", sdk, "--show-sdk-path")
            objects, binaries = [], []
            for architecture in architectures:
                target = f"{architecture}-apple-ios15.0"
                if variant == "simulator":
                    target += "-simulator"
                obj = cls.directory / f"{variant}-{architecture}.o"
                binary = cls.directory / f"{variant}-{architecture}.dylib"
                run("xcrun", "clang", "-target", target, "-isysroot", sdk_path,
                    "-g", "-c", source, "-o", obj)
                run("xcrun", "clang", "-target", target, "-isysroot", sdk_path,
                    "-dynamiclib", obj, "-o", binary)
                objects.append(obj)
                binaries.append(binary)
            for kind in ["dynamic", "static"]:
                root = cls.directory / kind / "MapLibre.xcframework"
                framework = root / identifier / "MapLibre.framework"
                write_plist(framework / "Info.plist", {"CFBundleExecutable": "MapLibre"})
                if kind == "dynamic":
                    run("xcrun", "lipo", "-create", *binaries, "-output", framework / "MapLibre")
                    dsym = root / identifier / "dSYMs/MapLibre.framework.dSYM"
                    dsym.parent.mkdir()
                    run("xcrun", "dsymutil", framework / "MapLibre", "-o", dsym)
                else:
                    run("xcrun", "libtool", "-static", "-o", framework / "MapLibre", *objects)
        for kind in ["dynamic", "static"]:
            write_plist(cls.directory / kind / "MapLibre.xcframework/Info.plist",
                        {"AvailableLibraries": libraries})

    def setUp(self):
        temporary = tempfile.TemporaryDirectory(dir=self.directory)
        self.addCleanup(temporary.cleanup)
        self.archive_root = Path(temporary.name) / "archive"
        shutil.copytree(self.directory / "dynamic", self.archive_root)
        self.root = self.archive_root / "MapLibre.xcframework"

    def check_archive(self, expected_error=None, static=False):
        archive = shutil.make_archive(str(self.archive_root), "zip", self.archive_root)
        command = [sys.executable, str(Path(__file__).with_name("check-xcframework-symbols.py")), archive]
        if static:
            command.append("--static")
        result = subprocess.run(command, capture_output=True, text=True)
        if expected_error:
            self.assertNotEqual(result.returncode, 0)
            self.assertIn(expected_error, result.stderr)
        else:
            self.assertEqual(result.returncode, 0, result.stderr)

    def test_dynamic_symbols(self):
        self.check_archive()

    def test_static_symbols(self):
        shutil.rmtree(self.archive_root)
        shutil.copytree(self.directory / "static", self.archive_root)
        self.check_archive(static=True)

    def test_missing_dsym(self):
        shutil.rmtree(self.root / "ios-arm64/dSYMs")
        self.check_archive("Expected one bundled dSYM")

    def test_missing_metadata(self):
        info = self.root / "Info.plist"
        with info.open("rb") as stream:
            plist = plistlib.load(stream)
        del plist["AvailableLibraries"][0]["DebugSymbolsPath"]
        write_plist(info, plist)
        self.check_archive("Missing DebugSymbolsPath")

    def test_mismatched_uuid(self):
        dwarf = "dSYMs/MapLibre.framework.dSYM/Contents/Resources/DWARF/MapLibre"
        run("xcrun", "lipo", self.root / "ios-arm64_x86_64-simulator" / dwarf,
            "-thin", "arm64", "-output", self.root / "ios-arm64" / dwarf)
        self.check_archive("UUIDs do not match")

    def test_missing_dwarf(self):
        # The linked binary has matching UUIDs but no DWARF compile units.
        shutil.copyfile(self.root / "ios-arm64/MapLibre.framework/MapLibre",
                        self.root / "ios-arm64/dSYMs/MapLibre.framework.dSYM/Contents/Resources/DWARF/MapLibre")
        self.check_archive("No DWARF compile units")

    def test_missing_slice(self):
        info = self.root / "Info.plist"
        with info.open("rb") as stream:
            plist = plistlib.load(stream)
        plist["AvailableLibraries"] = plist["AvailableLibraries"][1:]
        write_plist(info, plist)
        self.check_archive("Missing iOS slices")


if __name__ == "__main__":
    unittest.main()
