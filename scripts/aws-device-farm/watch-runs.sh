#!/bin/bash

# Starts the device-test-finish workflow for each completed AWS Device Farm run that
# aws-device-farm-run.sh scheduled, until none of those runs is in progress.

set -euo pipefail

finished="$(gh run list --workflow device-test-finish.yml --limit 500 --json displayTitle --jq '.[].displayTitle')"

while true; do
  # the AWS credentials are valid for four hours
  if ((SECONDS > 3 * 3600)); then
    gh workflow run device-test-watch.yml
    exit
  fi

  runs="$(aws devicefarm list-runs --arn "$AWS_DEVICE_FARM_PROJECT_ARN" --max-items 500 --output json)" || { sleep 30; continue; }
  in_progress=false
  while IFS=$'\t' read -r run status name; do
    if [[ "$status" != COMPLETED ]]; then
      in_progress=true
    elif [[ "$finished" != *"$run"* ]]; then
      gh workflow run device-test-finish.yml -f run="$run" -f name="$name"
      finished+=" $run"
    fi
  done < <(jq -r --arg since "$(date -d '1 day ago' -Is)" '.runs[] | select(.status != "COMPLETED" or .stopped > $since) |
    (.name | capture("^MapLibre Native (?<test>.*) \\|.* check=[0-9]+$")) as $name |
    [(.arn | split("/")[-1]), .status, $name.test] | @tsv' <<< "$runs")
  $in_progress || exit 0

  sleep 30
done
