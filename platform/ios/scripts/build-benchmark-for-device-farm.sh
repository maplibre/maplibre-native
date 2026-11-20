#!/usr/bin/env bash

# Build the iOS benchmark (BenchmarkApp and the BenchmarkUITests UI test runner) in Release for a device and package
# both as .ipa files for AWS Device Farm.
#
# Usage:
#   platform/ios/scripts/build-benchmark-for-device-farm.sh OUTPUT_DIR [XCODEBUILD_SETTING=VALUE ...]
#
# Writes OUTPUT_DIR/BenchmarkApp.ipa and OUTPUT_DIR/BenchmarkUITests-Runner.ipa.
#
# Needs device code signing to be set up (see platform/darwin/bazel/example_config.bzl). Bazel signs the app and the
# .xctest, but Xcode signs the generated UI test runner. The generated project uses automatic signing, which only works
# with Xcode-managed profiles; with any other profile, pass
#   CODE_SIGN_STYLE=Manual PROVISIONING_PROFILE_SPECIFIER="<profile name>"
#
# Generates platform/ios/MapLibre.xcodeproj (Metal renderer) if it doesn't exist yet.

set -euo pipefail

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 OUTPUT_DIR [XCODEBUILD_SETTING=VALUE ...]" >&2
  exit 1
fi

mkdir -p "$1"
output_dir="$(cd "$1" && pwd)"
shift

cd "$(dirname "${BASH_SOURCE[0]}")/.."

if [[ ! -d MapLibre.xcodeproj ]]; then
  bazel run //platform/ios:xcodeproj --@rules_xcodeproj//xcodeproj:extra_common_flags="--//:renderer=metal"
fi

build_dir="$(mktemp -d)"
xcodebuild build-for-testing -scheme BenchmarkUITests -configuration Release -project MapLibre.xcodeproj \
  -derivedDataPath "$build_dir" "$@"

for app in BenchmarkApp BenchmarkUITests-Runner; do
  app_dir="$(find "$build_dir" -name "$app.app" -type d -print -quit)"
  if [[ -z "$app_dir" ]]; then
    echo "$app.app not found in $build_dir" >&2
    exit 1
  fi
  staging_dir="$(mktemp -d)"
  mkdir -p "$staging_dir/Payload"
  ditto "$app_dir" "$staging_dir/Payload/$app.app"
  rm -f "$output_dir/$app.ipa"
  (cd "$staging_dir" && zip -qr "$output_dir/$app.ipa" Payload)
done

echo "Wrote $output_dir/BenchmarkApp.ipa and $output_dir/BenchmarkUITests-Runner.ipa"
