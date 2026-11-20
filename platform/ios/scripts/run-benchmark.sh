#!/usr/bin/env bash

# Run the iOS benchmark on AWS Device Farm and print its results.
#
# Usage:
#   platform/ios/scripts/run-benchmark.sh [IPA_DIR]
#
# IPA_DIR contains BenchmarkApp.ipa and BenchmarkUITests-Runner.ipa, from build-benchmark-for-device-farm.sh or from
# the ios-benchmark artifact of an ios-ci run. Without IPA_DIR, the benchmark is built first.
#
# Needs AWS credentials for the Device Farm project and `npm install` in the repository root (to download the results).
#
# Optional environment:
#   AWS_DEVICE_FARM_PROJECT_ARN      Device Farm project (default: MapLibre Native)
#   AWS_DEVICE_FARM_DEVICE_POOL_ARN  device pool (default: the iPhone pool used by ios-device-test.yml)

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "$script_dir/../../.." && pwd)"

if [[ $# -ge 1 ]]; then
  ipa_dir="$(cd "$1" && pwd)"
else
  ipa_dir="$(mktemp -d)"
  "$script_dir/build-benchmark-for-device-farm.sh" "$ipa_dir"
fi

export AWS_DEFAULT_REGION="${AWS_DEFAULT_REGION:-us-west-2}"
export AWS_DEVICE_FARM_PROJECT_ARN="${AWS_DEVICE_FARM_PROJECT_ARN:-arn:aws:devicefarm:us-west-2:373521797162:project:20687d72-0e46-403e-8f03-0941850665bc}"
export AWS_DEVICE_FARM_DEVICE_POOL_ARN="${AWS_DEVICE_FARM_DEVICE_POOL_ARN:-arn:aws:devicefarm:us-west-2:373521797162:devicepool:20687d72-0e46-403e-8f03-0941850665bc/d84a2883-0000-44b5-afc4-af22084caf85}"
export name="iOS Benchmark"
export appType=IOS_APP
export appFile="$ipa_dir/BenchmarkApp.ipa"
export testFile="$ipa_dir/BenchmarkUITests-Runner.ipa"
export testPackageType=XCTEST_UI_TEST_PACKAGE
export testType=XCTEST_UI
export wait_for_completion=true

run_status=0
run_arn="$("$repo_root/scripts/aws-device-farm/aws-device-farm-run.sh")" || run_status=$?
if [[ -z "$run_arn" ]]; then
  exit "$run_status"
fi

results_dir="$(mktemp -d)"
node "$repo_root/scripts/aws-device-farm/store-test-artifacts.mjs" --runArn "$run_arn" --outputDir "$results_dir"

# The benchmark app logs its results with NSLog, so they end up in the device syslog.
find "$results_dir" -name '*.syslog' -print0 | while IFS= read -r -d '' syslog; do
  echo "== $(basename "$(dirname "$syslog")")"
  grep -E 'Benchmark completed|\| .* ms \| .* ms \||Average frame|Cpu usage|Memory usage' "$syslog" || echo "No benchmark results in $syslog"
done

echo "Device Farm artifacts: $results_dir"
exit "$run_status"
