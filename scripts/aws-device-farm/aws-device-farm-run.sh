#!/bin/bash

# Schedules a run on AWS Device Farm. The device-test-watch workflow reports its result on the check run.

set -e
set -o xtrace

# List of required environment variables
required_vars=(
  "AWS_DEVICE_FARM_PROJECT_ARN"
  "AWS_DEVICE_FARM_DEVICE_POOL_ARN"
  "appFile"
  "appType"
  "testFile"
  "testType"
  "testPackageType"
  "name"
  "checkId"
)

check_var() {
  var_name=$1
  if [[ -z "${!var_name}" ]]; then
      echo "Error: Environment variable $var_name is not set."
      exit 1
  fi
}

for var in "${required_vars[@]}"; do
  check_var "$var"
done

if [[ ! -f "$appFile" ]]; then
  echo "Error: App file $appFile does not exist."
  exit 1
fi

if [[ ! -f "$testFile" ]]; then
  echo "Error: Test file $testFile does not exist."
  exit 1
fi

# Create upload app
response=$(aws devicefarm create-upload --type "$appType" --name "$(basename "$appFile")" --project-arn "$AWS_DEVICE_FARM_PROJECT_ARN")
echo "$response" >&2
app_arn="$(jq -r '.upload.arn' <<< "$response")"
app_url="$(jq -r '.upload.url' <<< "$response")"

# Create upload test package
response=$(aws devicefarm create-upload --type "$testPackageType" --name "$(basename "$testFile")" --project-arn "$AWS_DEVICE_FARM_PROJECT_ARN")
echo "$response" >&2
test_package_arn="$(jq -r '.upload.arn' <<< "$response")"
test_package_url="$(jq -r '.upload.url' <<< "$response")"

# Upload app and test package
curl -T "$appFile" "$app_url"
curl -T "$testFile" "$test_package_url"

# Wait for uploads to succeed
max_checks=10
sleep_time=5

check_status() {
  aws devicefarm get-upload --arn "$1" | jq -r '.upload.status'
}

while ((max_checks--)); do
  status_app="$(check_status "$app_arn")"
  status_test_package="$(check_status "$test_package_arn")"

  if [[ "$status_app" == "SUCCEEDED" && "$status_test_package" == "SUCCEEDED" ]]; then
    echo "Uploads succeeded" >&2
    break
  elif ((max_checks == 0)); then
    echo "App or test package failed to upload" >&2
    exit 1
  fi

  sleep $sleep_time
done

run_name="MapLibre Native $name |${supersedeKey:+ key=$(sha256sum <<< "$supersedeKey" | head -c 12)} check="

# Stop the runs of this test that an earlier commit of the same pull request scheduled
if [[ -n "$supersedeKey" ]]; then
  aws devicefarm list-runs --arn "$AWS_DEVICE_FARM_PROJECT_ARN" --max-items 500 --output json |
    jq -r --arg name "$run_name" '.runs[] | select(.status != "COMPLETED" and .status != "STOPPING" and (.name | startswith($name))) | .arn' |
    xargs -r -n 1 aws devicefarm stop-run --query run.status --arn
fi

aws devicefarm schedule-run \
  --project-arn "$AWS_DEVICE_FARM_PROJECT_ARN" \
  --name "$run_name$checkId" \
  --app-arn "$app_arn" \
  --device-pool-arn "$AWS_DEVICE_FARM_DEVICE_POOL_ARN" \
  --test type=$testType,testPackageArn=$test_package_arn${testFilter:+,filter=$testFilter}${testSpecArn:+,testSpecArn=$testSpecArn} \
  --execution-configuration '{"videoCapture": false}' \
  --output text --query run.arn

# A watcher that has yet to start will see this run, any other one may be about to exit
if [[ "$(gh run list --workflow device-test-watch.yml --json status --jq 'all(.[]; .status == "in_progress" or .status == "completed")')" == true ]]; then
  gh workflow run device-test-watch.yml
fi
