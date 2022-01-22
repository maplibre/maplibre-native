#!/usr/bin/env bash

set -e
set -o pipefail

export BUILDTYPE=Release

make darwin-check-public-symbols
echo "================================ ios-lint-plist"
make ios-lint-plist
echo "================================ ios-test"
make ios-test
echo "================================ ios-uitest"
make ios-uitest
echo "================================ ios-sanitize"
make ios-sanitize      
echo "================================ ios-sanitize-address"
make ios-sanitize-address
echo "================================ ios-static-analyzer"
make ios-static-analyzer