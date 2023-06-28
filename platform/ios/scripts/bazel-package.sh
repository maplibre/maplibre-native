#!/bin/bash

set -euo pipefail

# "static" or "dynamic"
build_type="static"

# "release" or "debug"
release_type="release"

# Renderer flavor
flavor="legacy"

# Provisioning team ID
teamid="0000000000"

# Provisioning profile name/UUID
uuid="iOS Team Provisioning Profile: *"

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
      echo "Build the maplibre xcframework using the bazel build files. You must install Baselisk to build using this method."
      echo "Usage: .bazel-package.sh --static|--dynamic --release|--debug"
      echo "Defaults to \"--static --release\""
      exit 1
      ;;
   --flavor)
      shift
      flavor="$1"
      shift
      ;;
   --teamid)
      shift
      teamid="$1"
      shift
      ;;
   --profile-uuid)
      shift
      uuid="$1"
      shift
      ;;
   -*|--*)
      echo "Unknown option $1"
      exit 1
      ;;
   esac
done

echo "------ Build type: $build_type Release type: $release_type ------"

target="Mapbox.$build_type"

compilation_mode="opt"
if [ "$release_type" = "debug" ]; then
   compilation_mode="dbg"
fi;

echo "------ Determining Maplibre version and hash ------"

sem_version=$(git describe --tags --match=ios-v*.*.* --abbrev=0 | sed 's/^ios-v//')
hash=$(git log | head -1 | awk '{ print $2 }' | cut -c 1-10) && true

# Move and configure the info plists with the semantic version and hash.
temp_info_static_plist="platform/ios/build/Info-static.plist"
temp_info_plist="platform/ios/build/Info.plist"

if [ ! -d platform/ios/build ]; then
   mkdir platform/ios/build
fi

cp platform/ios/framework/Info-static.plist "$temp_info_static_plist"
cp platform/ios//framework/Info.plist "$temp_info_plist"

plutil -replace MLNSemanticVersionString -string "$sem_version" "$temp_info_static_plist"
plutil -replace MLNCommitHash -string "$hash" "$temp_info_static_plist"
plutil -replace MLNSemanticVersionString -string "$sem_version" "$temp_info_plist"
plutil -replace MLNCommitHash -string "$hash" "$temp_info_plist"

echo "------ Building Maplibre version: $sem_version hash: $hash ------"

# Generate provisioning team ID
cat > platform/ios/bazel/config.bzl <<EOF
APPLE_MOBILE_PROVISIONING_PROFILE_TEAM_ID = "$teamid"
APPLE_MOBILE_PROVISIONING_PROFILE_UUID = "$uuid"
EOF

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
   --//:renderer=$flavor \
   --//:maplibre_platform=ios

echo "Done."
echo "Package will be available in \"/bazel-bin/platform/ios/$target.xcframework.zip\""
