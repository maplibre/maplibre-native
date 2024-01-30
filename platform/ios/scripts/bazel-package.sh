#!/bin/bash

set -eo pipefail

# "static" or "dynamic"
build_type="static"

# "release" or "debug"
release_type="release"

# Renderer flavor
flavor="legacy"

while [[ $# -gt 0 ]]; do
   case $1 in
   --static)
      build_type="static"
      shift
      ;;
   --dynamic)
      build_type="dynamic"
      shift
      ;;
   --release)
      release_type="release"
      shift
      ;;
   --debug)
      release_type="debug"
      shift
      ;;
   --link)
      build_type="link"
      shift
      ;;
   --help)
      echo "Build the maplibre xcframework using the bazel build files. You must install Bazelisk to build using this method."
      echo "Usage: .bazel-package.sh --static|--dynamic --release|--debug"
      echo "Defaults to \"--static --release\""
      exit 1
      ;;
   --flavor)
      shift
      flavor="$1"
      shift
      ;;
   -*|--*)
      echo "Unknown option $1"
      exit 1
      ;;
   esac
done

echo "------ Build type: $build_type Release type: $release_type ------"

target="MapLibre.$build_type"

compilation_mode="opt"
if [ "$release_type" = "debug" ]; then
   compilation_mode="dbg"
fi;

if [ ! -d platform/ios/build ]; then
   mkdir platform/ios/build
fi

echo "------ Building Maplibre ------"

# Build
ncpu=$(sysctl -n hw.ncpu)
bazel build //platform/ios:"$target" --apple_platform_type=ios \
   --apple_generate_dsym \
   --compilation_mode="$compilation_mode" \
   --features=dead_strip \
   --objc_enable_binary_stripping \
   --copt=-Wall --copt=-Wextra --copt=-Wpedantic \
   --copt=-Werror \
   --jobs "$ncpu" \
   --//:renderer=$flavor

echo "Done."
echo "Package will be available in \"/bazel-bin/platform/ios/$target.xcframework.zip\""
