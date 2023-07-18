#!/usr/bin/env bash

set -euo pipefail

cd ../../

genstrings -u -o macos/sdk/Base.lproj darwin/src/*.{m,mm}
genstrings -u -o macos/sdk/Base.lproj macos/src/*.{m,mm}
genstrings -u -o ios/resources/Base.lproj ios/src/*.{m,mm}
find ios/resources macos/sdk -path '*/Base.lproj/*.strings' -exec \
	textutil -convert txt -extension strings -inputencoding UTF-16 -encoding UTF-8 {} \;
mv macos/sdk/Base.lproj/Foundation.strings darwin/resources/Base.lproj/