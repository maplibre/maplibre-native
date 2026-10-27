#!/bin/bash
# Resolves the artifact names of $BRANCH from the JSON mapping in $CONFIG, or from
# $DEFAULT_CONFIG_FILE if $CONFIG is empty. See README.md.

set -euo pipefail

: "${BRANCH:?BRANCH must be set}"
: "${DEFAULT_CONFIG_FILE:?DEFAULT_CONFIG_FILE must be set}"

if [[ -n "${CONFIG//[[:space:]]/}" ]]; then
  config="$CONFIG"
  origin="the RELEASE_ARTIFACTS repository variable"
else
  config="$(<"$DEFAULT_CONFIG_FILE")"
  origin="the default configuration (RELEASE_ARTIFACTS is not set)"
fi

if ! jq -e 'type == "object"' <<<"$config" >/dev/null 2>&1; then
  echo "::error::Cannot read $origin: expected a JSON object mapping branch names to artifact names"
  exit 1
fi

if ! jq -e --arg branch "$BRANCH" 'has($branch)' <<<"$config" >/dev/null; then
  branches="$(jq -r 'keys | map("'"'"'" + . + "'"'"'") | join(", ")' <<<"$config")"
  echo "::error::No release artifacts are configured for branch '$BRANCH' in $origin. Configured branches: ${branches:-none}. Releases from an unconfigured branch are refused so that they cannot overwrite the artifacts of another branch."
  exit 1
fi

entry="$(jq -c --arg branch "$BRANCH" '.[$branch]' <<<"$config")"

# Every value of a platform has to be a non-empty string, e.g. an empty artifact ID would
# publish under the wrong name.
if ! jq -e 'type == "object" and ([.android, .ios] | map(select(. != null)) | all(type == "object" and (to_entries | all(.value | type == "string" and length > 0))))' <<<"$entry" >/dev/null; then
  echo "::error::The entry of branch '$BRANCH' in $origin is invalid: it must be an object whose 'android' and 'ios' objects only have non-empty string values"
  exit 1
fi

field() {
  jq -r "$1 // \"\"" <<<"$entry"
}

{
  echo "branch=$BRANCH"
  echo "config=$entry"
  echo "android-artifact-id=$(field .android.artifactId)"
  echo "ios-swift-package=$(field .ios.swiftPackage)"
  echo "ios-plugins-swift-package=$(field .ios.pluginsSwiftPackage)"
  echo "ios-pod-name=$(field .ios.podName)"
} >>"${GITHUB_OUTPUT:-/dev/stdout}"

echo "Release artifacts of branch '$BRANCH' from $origin:"
jq . <<<"$entry"
