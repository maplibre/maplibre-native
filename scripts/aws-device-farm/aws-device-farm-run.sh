#!/bin/bash

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

# Retry logic: reschedule when result is SKIPPED up to 3 times
max_retries=3
for attempt in $(seq 1 $max_retries); do
  echo "Scheduling Device Farm run (attempt $attempt)" >&2
  arn=$(aws devicefarm schedule-run \
    --project-arn "$AWS_DEVICE_FARM_PROJECT_ARN" \
    --name "MapLibre Native $name" \
    --app-arn "$app_arn" \
    --device-pool-arn "$AWS_DEVICE_FARM_DEVICE_POOL_ARN" \
    --test type=$testType,testPackageArn=$test_package_arn${testFilter:+,filter=$testFilter}${testSpecArn:+,testSpecArn=$testSpecArn} \
    --execution-configuration videoCapture=false \
    --output text --query run.arn)

  echo "ARN: $arn" >&2

  if [[ -z "$arn" ]]; then
    echo "Error: Failed to schedule Device Farm run or got empty ARN" >&2
    exit 1
  fi

  if [[ "$wait_for_completion" != "true" ]]; then
    echo "Not waiting for run to complete" >&2
    exit 0
  fi

  # Wait for run result
  while true; do
    sleep 30
    result=$(aws devicefarm get-run --arn "$arn" --output text --query "run.result")
    case $result in
      FAILED|ERRORED|STOPPED)
        echo "$arn"
        echo "Run $result" >&2
        exit 1
        ;;
      SKIPPED)
        echo "Run skipped on attempt $attempt" >&2
        if [[ $attempt -lt $max_retries ]]; then
          echo "Retrying..." >&2
          break  # break wait loop and retry
        else
          echo "$arn"
          echo "Max retries ($max_retries) reached. Exiting." >&2
          exit 1
        fi
        ;;
      PASSED)
        echo "$arn"
        echo "Run $result" >&2
        exit 0
        ;;
      PENDING)
        continue
        ;;
      *)
        echo "Unexpected run result $result" >&2
        exit 1
        ;;
    esac
  done

done

# If for-loop exits without passing or failing, exit with error
echo "Retries exhausted without a definitive result." >&2
exit 1
