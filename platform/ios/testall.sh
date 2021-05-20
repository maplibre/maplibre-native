#!/usr/bin/env bash

set -e
set -o pipefail

export BUILDTYPE=Debug

make darwin-check-public-symbols
make ios-lint-plist
make ios-test
make ios-uitest
make ios-sanitize      
make ios-sanitize-address
make ios-static-analyzer