#!/usr/bin/env python3
"""Check debug information in a released MapLibre iOS XCFramework ZIP."""

import argparse
import plistlib
import re
import subprocess
import sys
import tempfile
import zipfile
from pathlib import Path


def dwarfdump(path, *options):
    return subprocess.run(
        ["xcrun", "dwarfdump", *options, str(path)],
        check=True,
        capture_output=True,
        text=True,
    ).stdout


def uuids(path):
    matches = re.findall(r"UUID: ([0-9A-Fa-f-]+) \(([^)]+)\)", dwarfdump(path, "--uuid"))
    if not matches:
        raise ValueError(f"No UUIDs in {path}")
    return {architecture: uuid.upper() for uuid, architecture in matches}


def check_dwarf(path, architectures):
    for architecture in architectures:
        output = dwarfdump(path, "--debug-info", "--recurse-depth=0", f"--arch={architecture}")
        if "DW_TAG_compile_unit" not in output:
            raise ValueError(f"No DWARF compile units for {architecture} in {path}")


def check_xcframework(root, static=False):
    with (root / "Info.plist").open("rb") as stream:
        libraries = plistlib.load(stream)["AvailableLibraries"]
    covered = set()
    for library in libraries:
        identifier = library["LibraryIdentifier"]
        if library["SupportedPlatform"] != "ios":
            raise ValueError(f"Unexpected platform in {identifier}")
        architectures = set(library["SupportedArchitectures"])
        if not architectures:
            raise ValueError(f"No architectures in {identifier}")
        variant = library.get("SupportedPlatformVariant", "device")
        covered.update((variant, architecture) for architecture in architectures)
        slice_root = root / identifier
        framework = slice_root / library["LibraryPath"]
        with (framework / "Info.plist").open("rb") as stream:
            executable = plistlib.load(stream)["CFBundleExecutable"]
        binary = framework / executable
        if static:
            check_dwarf(binary, architectures)
        else:
            symbols_path = library.get("DebugSymbolsPath")
            if not symbols_path:
                raise ValueError(f"Missing DebugSymbolsPath in {identifier}")
            bundles = list((slice_root / symbols_path).glob("*.dSYM"))
            if len(bundles) != 1:
                raise ValueError(f"Expected one bundled dSYM in {identifier}, found {len(bundles)}")
            with (bundles[0] / "Contents/Info.plist").open("rb") as stream:
                plistlib.load(stream)
            dwarf_files = list((bundles[0] / "Contents/Resources/DWARF").glob("*"))
            if len(dwarf_files) != 1 or not dwarf_files[0].is_file():
                raise ValueError(f"Expected one DWARF file in {bundles[0]}")
            binary_uuids = uuids(binary)
            if set(binary_uuids) != architectures:
                raise ValueError(f"Binary architectures disagree with Info.plist in {identifier}")
            if uuids(dwarf_files[0]) != binary_uuids:
                raise ValueError(f"Binary and dSYM UUIDs do not match in {identifier}")
            check_dwarf(dwarf_files[0], architectures)
        print(f"Verified {identifier}: {', '.join(sorted(architectures))}")
    expected = {("device", "arm64"), ("simulator", "arm64"), ("simulator", "x86_64")}
    if not expected.issubset(covered):
        raise ValueError(f"Missing iOS slices: {sorted(expected - covered)}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archive", type=Path)
    parser.add_argument("--static", action="store_true", help="Check DWARF in static object files")
    args = parser.parse_args()
    try:
        with tempfile.TemporaryDirectory(prefix="maplibre-symbols-") as directory:
            with zipfile.ZipFile(args.archive) as archive:
                archive.extractall(directory)
            check_xcframework(Path(directory) / "MapLibre.xcframework", args.static)
    except (OSError, ValueError, KeyError, zipfile.BadZipFile, subprocess.CalledProcessError) as error:
        print(f"XCFramework debug-symbol check failed: {error}", file=sys.stderr)
        if isinstance(error, subprocess.CalledProcessError):
            print(error.stderr, file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
