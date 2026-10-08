#!/usr/bin/env python3
"""Check that every shipped architecture exposes the public plugin C API."""

import pathlib
import plistlib
import subprocess
import sys


def verify(root):
    with (root / "Info.plist").open("rb") as source:
        libraries = plistlib.load(source)["AvailableLibraries"]
    expected = {("ios", ""): {"arm64"}, ("ios", "simulator"): {"arm64", "x86_64"}}
    found = {}
    for library in libraries:
        platform = (library["SupportedPlatform"], library.get("SupportedPlatformVariant", ""))
        architectures = set(library["SupportedArchitectures"])
        if platform in found:
            raise ValueError(f"Duplicate platform: {platform}")
        found[platform] = architectures
        framework = root / library["LibraryIdentifier"] / library["LibraryPath"]
        if not (framework / "Headers/plugin_api.h").is_file():
            raise ValueError(f"Missing public plugin_api.h: {framework}")
        for architecture in sorted(architectures):
            symbols = subprocess.check_output(
                ["xcrun", "nm", "-arch", architecture, "-gU", str(framework / framework.stem)],
                text=True,
            )
            if not any(line.split()[-1:] == ["_mln_plugin_register_v1"] for line in symbols.splitlines()):
                raise ValueError(f"Missing mln_plugin_register_v1: {framework} ({architecture})")
            print(f"Plugin API verified: {library['LibraryIdentifier']} ({architecture})", flush=True)
    if found != expected:
        raise ValueError(f"Unexpected framework slices: {found}; expected {expected}")


if __name__ == "__main__":
    try:
        verify(pathlib.Path(sys.argv[1]))
    except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as error:
        sys.exit(f"Plugin XCFramework verification failed: {error}")
