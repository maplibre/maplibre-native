#!/bin/bash

set -euo pipefail

usage() {
  echo "Usage: $0 upload <repository-dir> <deployment-name> <deployment-id-file>"
  echo "       $0 publish <deployment-id>..."
  echo ""
  echo "  upload   Uploads all publications in <repository-dir>, a Maven repository with signed"
  echo "           publications, to Maven Central as a single deployment. Waits until Maven Central"
  echo "           has validated the deployment, but does not publish it, and writes its ID to"
  echo "           <deployment-id-file>."
  echo "  publish  Publishes the validated deployments and waits until Maven Central has published"
  echo "           them. Skips deployments that are already being published, so it can be rerun."
  echo ""
  echo "Requires MAVEN_CENTRAL_USERNAME and MAVEN_CENTRAL_PASSWORD (a Central Portal user token)."
  exit 1
}

api=https://central.sonatype.com/api/v1/publisher
authorization="Authorization: Bearer $(printf '%s:%s' "$MAVEN_CENTRAL_USERNAME" "$MAVEN_CENTRAL_PASSWORD" | base64 | tr -d '\n')"

deployment_status() {
  curl --silent --show-error --fail-with-body --retry 5 --retry-all-errors -X POST -H "$authorization" \
    --url-query "id=$1" "$api/status"
}

# Waits up to 90 minutes until the deployment reaches the given state
wait_for_state() {
  local deployment_id=$1 expected_state=$2 status state
  for _ in $(seq 180); do
    status=$(deployment_status "$deployment_id")
    state=$(jq -r .deploymentState <<< "$status")
    echo "Deployment $deployment_id: $state"
    if [ "$state" = "$expected_state" ]; then
      return 0
    elif [ "$state" = FAILED ]; then
      echo "::error::Maven Central rejected deployment $deployment_id."
      jq .errors <<< "$status"
      return 1
    fi
    sleep 30
  done
  if [ "$expected_state" = PUBLISHED ] && [ "$state" = PUBLISHING ]; then
    echo "::warning::Maven Central has not finished publishing deployment $deployment_id after 90 minutes."
    return 0
  fi
  echo "::error::Deployment $deployment_id is still $state after 90 minutes."
  return 1
}

case ${1:-} in
  upload)
    [ $# -eq 4 ] || usage
    repository_dir=$2
    deployment_name=$3
    deployment_id_file=$4

    # Like the Gradle plugin that publishes to Maven Central, leave out the maven-metadata.xml files
    bundle="$(mktemp -d)/bundle.zip"
    (cd "$repository_dir" && zip -qrD "$bundle" . -x '*maven-metadata*')
    echo "Uploading a $(du -h "$bundle" | cut -f1) bundle with $(find "$repository_dir" -name '*.pom' | wc -l) publications"

    deployment_id=$(curl --silent --show-error --fail-with-body -H "$authorization" \
      --url-query "name=$deployment_name" --url-query publishingType=USER_MANAGED \
      -F "bundle=@$bundle" "$api/upload")
    echo "Deployment ID: $deployment_id"
    wait_for_state "$deployment_id" VALIDATED
    echo "$deployment_id" > "$deployment_id_file"
    ;;

  publish)
    shift
    [ $# -ge 1 ] || usage

    # Check all deployments before publishing any of them
    for deployment_id in "$@"; do
      state=$(deployment_status "$deployment_id" | jq -r .deploymentState)
      if [[ ! "$state" =~ ^(VALIDATED|PUBLISHING|PUBLISHED)$ ]]; then
        echo "::error::Deployment $deployment_id is $state, not VALIDATED."
        exit 1
      fi
    done

    for deployment_id in "$@"; do
      if [ "$(deployment_status "$deployment_id" | jq -r .deploymentState)" = VALIDATED ]; then
        echo "Publishing deployment $deployment_id"
        curl --silent --show-error --fail-with-body -X POST -H "$authorization" "$api/deployment/$deployment_id"
      fi
    done

    for deployment_id in "$@"; do
      wait_for_state "$deployment_id" PUBLISHED
    done
    ;;

  *)
    usage
    ;;
esac
