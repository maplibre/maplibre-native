#!/bin/bash

set -euo pipefail

echo "------ Determining Maplibre version and hash ------"

sem_version=0.0.0
hash=$(git log | head -1 | awk '{ print $2 }' | cut -c 1-10) && true

# Move and configure the info plists with the semantic version and hash.
temp_info_static_plist="platform/ios/build/Info-static.plist"
temp_info_plist="platform/ios/build/Info.plist"

if [ ! -d platform/ios/build ]; then
   mkdir platform/ios/build
fi

cp platform/ios/framework/Info-static.plist "$temp_info_static_plist"
cp platform/ios/framework/Info.plist "$temp_info_plist"

plutil -replace MLNSemanticVersionString -string "$sem_version" "$temp_info_static_plist"
plutil -replace MLNCommitHash -string "$hash" "$temp_info_static_plist"
plutil -replace MLNSemanticVersionString -string "$sem_version" "$temp_info_plist"
plutil -replace MLNCommitHash -string "$hash" "$temp_info_plist"

echo "------ Building Maplibre version: $sem_version hash: $hash ------"

bazel run //platform/ios:xcodeproj