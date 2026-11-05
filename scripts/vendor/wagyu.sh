#!/usr/bin/env bash
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

NAME=wagyu
VERSION=9c87e553a51170e4ce85fd2ef3160bc3434eb751

if git -C "$VENDOR" ls-files --stage -- "$NAME" | grep -q '^160000'; then
    echo "vendor/$NAME is a git submodule; remove it first: git rm -f vendor/$NAME" >&2
    exit 1
fi

ROOT=wagyu-$VERSION

download "https://github.com/mapbox/wagyu/archive/$VERSION.tar.gz"
init
extract_gzip "$ROOT/include" "$ROOT/LICENSE"
file_list include -name "*.hpp"
