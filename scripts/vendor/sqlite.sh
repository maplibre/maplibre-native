#!/usr/bin/env bash
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

NAME=sqlite
VERSION=3.45.3
ROOT=sqlite-amalgamation-3450300

download "https://www.sqlite.org/2024/$ROOT.zip"
init
extract_zip "$ROOT/sqlite3.*"
mkdir -p include src
mv sqlite3.h include
mv sqlite3.c src
file_list include src -name "*.h" -o -name "*.c"

# We need this patch for compiling on Android
# See https://www.sqlite.org/src/info/f18b2524da6bbbcf
# should be removed when we update SQLite
# and this is included in the release
git apply $PWD/../../scripts/vendor/sqlite-ndk.patch
