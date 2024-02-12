#!/bin/bash

# This script is a bit of a hack to generate DocC documentation until Bazel has support for it.
# https://github.com/bazelbuild/rules_apple/issues/2241
# To use this script, make sure the XCFramework is built with Bazel (see ios-ci.yml).
# Then to start a local preview, run:
# $ scripts/docc.sh bazel
# You can also build the documentation locally
# $ scripts/docc.sh
# Then go to build/ios and run
# $ python3 -m http.server
# Go to http://localhost:8000/documentation/maplibre/

cmd="convert"
if [[ "$1" == "preview" ]]; then
  cmd="preview"
fi

SDK_PATH=$(xcrun -sdk iphoneos --show-sdk-path)

build_dir=build

rm -rf "$build_dir"/symbol-graphs
rm -rf "$build_dir"/headers/MapLibre
rm -rf "$build_dir"/MapLibre.xcframework

mkdir -p "$build_dir"/symbol-graphs
mkdir -p "$build_dir"/headers/MapLibre

# unzip built XCFramework in build dir
unzip ../../bazel-bin/platform/ios/MapLibre.dynamic.xcframework.zip -d "$build_dir"

# copy all public headers from XCFramework
cp "$build_dir"/MapLibre.xcframework/ios-arm64/MapLibre.framework/Headers/*.h "$build_dir"/headers/MapLibre

xcrun --toolchain swift clang \
     -extract-api \
     --product-name=MapLibre \
     -isysroot $SDK_PATH \
     -F "$SDK_PATH"/System/Library/Frameworks \
     -I "$PWD" \
     -I "$(realpath ../darwin/src)" \
     -I "$build_dir"/headers \
     -x objective-c-header  \
     -o "$build_dir"/symbol-graphs/MapLibre.symbols.json  \
     "$build_dir"/headers/MapLibre/*.h

export DOCC_HTML_DIR=$(dirname $(xcrun --toolchain swift --find docc))/../share/docc/render
$(xcrun --find docc) "$cmd" MapLibre.docc \
    --fallback-display-name "MapLibre Native for iOS" \
    --fallback-bundle-identifier org.swift.MyProject \
    --fallback-bundle-version 0.0.1  \
    --additional-symbol-graph-dir "$build_dir"/symbol-graphs \
    --output-path "$build_dir"/MapLibre.doccarchive

if [[ "$cmd" == "convert" ]]; then
  rm -rf build/docs
  $(xcrun --find docc) process-archive transform-for-static-hosting "$build_dir"/MapLibre.doccarchive --output-path build/docs
fi