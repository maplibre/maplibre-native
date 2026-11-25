#!/usr/bin/env bash

# Sets the ci-passed commit status of commit $1, the one check for branch protection
# to require. A skipped job passes a required check, so every workflow run on the
# commit must succeed or be skipped, and so must every device test (benchmarks aside).
#
# android-ci and ios-ci start a device test workflow when they complete. It runs on
# the default branch, and its run name ends with the commit. Until the one that
# follows a completed android-ci or ios-ci run completes, the device tests it may
# start are unknown, so the status stays pending.

set -euo pipefail
shopt -s inherit_errexit

sha=$1
[[ $sha =~ ^[0-9a-f]{40}$ ]]
repo=repos/$GITHUB_REPOSITORY

state() {
  local runs
  runs=$(gh api --paginate "$repo/actions/runs?head_sha=$sha&per_page=100" --jq '.workflow_runs[]')
  {
    jq -c '{name, status, conclusion}' <<< "$runs"
    gh api --paginate "$repo/commits/$sha/check-runs?per_page=100" --jq '.check_runs[] |
      select(.app.slug != "github-actions" and (.name | contains("Benchmark") | not)) | {name, status, conclusion}'
    jq -r 'select(.status == "completed" and (.name == "android-ci" or .name == "ios-ci")) |
      "\(.name | sub("ci$"; "device-test")) \(.run_started_at)"' <<< "$runs" |
      while read -r workflow since; do
        gh api --paginate -X GET "$repo/actions/workflows/$workflow.yml/runs" -f created=">=$since" -F per_page=100 --jq '.workflow_runs[]' |
          jq -sc --arg title "$workflow $sha" 'map(select(.display_title == $title)) |
            if . == [] then [{name: $title, status: "missing"}] else . end | .[] | {name, status, conclusion}'
      done
  } | tee /dev/stderr | jq -rs '
    if any(.status == "completed" and (.conclusion | IN("success", "skipped", "neutral", "action_required") | not)) then "failure"
    elif all(.status == "completed" and .conclusion != "action_required") then "success"
    else "pending" end'
}

# Runs for the same commit can overlap, so check again after setting the status, until
# it is up to date.
posted=
while true; do
  state=$(state)
  [[ $state != "$posted" ]] || break
  echo "ci-passed: $state"
  gh api "$repo/statuses/$sha" --silent -f context=ci-passed -f state="$state" \
    -f target_url="$GITHUB_SERVER_URL/$GITHUB_REPOSITORY/actions/runs/$GITHUB_RUN_ID"
  posted=$state
  sleep 10
done
