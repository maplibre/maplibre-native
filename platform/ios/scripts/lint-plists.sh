#!/usr/bin/env bash
set -euo pipefail
echo Linting...

plutil -lint platform/ios/app/Info.plist
plutil -lint platform/ios/bench_UITests/Info.plist
plutil -lint platform/ios/benchmark/Info.plist
plutil -lint platform/ios/framework/Info.plist
plutil -lint platform/ios/framework/Info-static.plist
plutil -lint platform/ios/Integration_Test_Harness/Info.plist
plutil -lint platform/ios/Integration_Tests/Info.plist
plutil -lint platform/ios/iosapp-UITests/Info.plist
plutil -lint platform/ios/test/Info.plist
