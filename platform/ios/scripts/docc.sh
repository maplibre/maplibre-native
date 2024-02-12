#!/bin/bash

SDK_PATH=$(xcrun -sdk iphoneos --show-sdk-path)

build_dir="$(mktemp -d)"

mkdir "$build_dir"/symbol-graphs
mkdir -p "$build_dir"/headers/MapLibre

# unzip built XCFramework in build dir
unzip ../../bazel-bin/platform/ios/MapLibre.dynamic.xcframework.zip -d "$build_dir"

# copy all public headers from XCFramework
cp "$build_dir"/MapLibre.xcframework/ios-arm64/MapLibre.framework/Headers/*.h "$build_dir"/headers/MapLibre

xcrun --toolchain swift clang                        \
     -extract-api                                    \
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
$(xcrun --find docc) preview MapLibre.docc \
    --fallback-display-name MapLibre \
    --fallback-bundle-identifier org.swift.MyProject \
    --fallback-bundle-version 0.0.1  \
    --additional-symbol-graph-dir .build/symbol-graphs \
    --output-path "$build_dir"/MapLibre.doccarchive

$(xcrun --find docc) process-archive transform-for-static-hosting "$build_dir"/MapLibre.doccarchive --output-path docs