#!/usr/bin/env bash

# Build the iOS benchmark for AWS Device Farm: BenchmarkApp in Release for a device, and BenchmarkTests, an XCTest
# bundle that runs inside BenchmarkApp and waits for the benchmark to finish.
#
# Usage:
#   platform/ios/scripts/build-benchmark-for-device-farm.sh OUTPUT_DIR
#
# Writes OUTPUT_DIR/BenchmarkApp.ipa and OUTPUT_DIR/BenchmarkTests.xctest.zip. Bazel signs both with the provisioning
# profile from platform/darwin/bazel/config.bzl (or example_config.bzl), like any other device build.

set -euo pipefail

if [[ $# -ne 1 ]]; then
  echo "Usage: $0 OUTPUT_DIR" >&2
  exit 1
fi

mkdir -p "$1"
output_dir="$(cd "$1" && pwd)"

cd "$(dirname "${BASH_SOURCE[0]}")/../../.."

bazel_flags=(-c opt --//:renderer=metal --ios_multi_cpus=arm64)
targets="//platform/ios:BenchmarkApp + //platform/ios/benchmark:BenchmarkTests"
bazel build "${bazel_flags[@]}" //platform/ios:BenchmarkApp //platform/ios/benchmark:BenchmarkTests
outputs="$(bazel cquery "${bazel_flags[@]}" --output=files "$targets" 2>/dev/null)"
app_ipa="$(grep '/BenchmarkApp\.ipa$' <<< "$outputs")"
tests_zip="$(grep '/BenchmarkTests\.zip$' <<< "$outputs")"

staging_dir="$(mktemp -d)"
unzip -q "$app_ipa" -d "$staging_dir"
# Device Farm runs the test bundle from the app's PlugIns directory.
mkdir -p "$staging_dir/Payload/BenchmarkApp.app/PlugIns"
unzip -q "$tests_zip" -d "$staging_dir/Payload/BenchmarkApp.app/PlugIns"
chmod -R u+w "$staging_dir/Payload"
# BenchmarkApp reports this as the gitRevision of its results.
if git_revision="$(git rev-parse HEAD 2>/dev/null)"; then
  plutil -replace MLNGitRevision -string "$git_revision" "$staging_dir/Payload/BenchmarkApp.app/Info.plist"
fi
# The app has to contain Xcode's testing libraries to load the test bundle, like xcodebuild build-for-testing does
# for RenderTestApp. Not every Xcode version has all of them (Xcode 27 has no XCUnit.framework).
iphoneos_developer_dir="$(xcode-select -p)/Platforms/iPhoneOS.platform/Developer"
for library in Library/Frameworks/{XCTest,XCUIAutomation,Testing}.framework \
  Library/PrivateFrameworks/{XCTestCore,XCTestSupport,XCTAutomationSupport,XCUnit}.framework \
  usr/lib/{libXCTestBundleInject,libXCTestSwiftSupport,lib_TestingInterop}.dylib; do
  if [[ -e "$iphoneos_developer_dir/$library" ]]; then
    ditto "$iphoneos_developer_dir/$library" "$staging_dir/Payload/BenchmarkApp.app/Frameworks/$(basename "$library")"
  fi
done
rm -f "$output_dir/BenchmarkApp.ipa"
(cd "$staging_dir" && zip -qr "$output_dir/BenchmarkApp.ipa" Payload)

rm -f "$output_dir/BenchmarkTests.xctest.zip"
cp "$tests_zip" "$output_dir/BenchmarkTests.xctest.zip"

echo "Wrote $output_dir/BenchmarkApp.ipa and $output_dir/BenchmarkTests.xctest.zip"
