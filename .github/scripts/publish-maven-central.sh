#!/bin/bash

set -euo pipefail

if [ $# -ne 2 ]; then
  echo "Usage: $0 <repository-dir> <deployment-name>"
  echo "  <repository-dir>   Maven repository with the signed publications"
  echo "  <deployment-name>  Name of the deployment in the Central Portal"
  echo ""
  echo "Uploads all publications in <repository-dir> to Maven Central as a single deployment"
  echo "and waits until they are published. Maven Central validates the deployment as a whole,"
  echo "so it publishes either all of the publications or none of them."
  echo "Requires MAVEN_CENTRAL_USERNAME and MAVEN_CENTRAL_PASSWORD (a Central Portal user token)."
  exit 1
fi

repository_dir=$1
deployment_name=$2
api=https://central.sonatype.com/api/v1/publisher
authorization="Authorization: Bearer $(printf '%s:%s' "$MAVEN_CENTRAL_USERNAME" "$MAVEN_CENTRAL_PASSWORD" | base64 | tr -d '\n')"

# The Central Portal accepts bundles of up to 1 GB.
# Like the Gradle plugin that publishes to Maven Central, leave out the maven-metadata.xml files.
bundle="$(mktemp -d)/bundle.zip"
(cd "$repository_dir" && zip -qrD "$bundle" . -x '*maven-metadata*')
echo "Uploading a $(du -h "$bundle" | cut -f1) bundle with $(find "$repository_dir" -name '*.pom' | wc -l) publications"

deployment_id=$(curl --silent --show-error --fail-with-body -H "$authorization" \
  --url-query "name=$deployment_name" --url-query publishingType=AUTOMATIC \
  -F "bundle=@$bundle" "$api/upload")
echo "Deployment ID: $deployment_id"

for _ in $(seq 180); do
  status=$(curl --silent --show-error --fail-with-body --retry 5 --retry-all-errors -X POST -H "$authorization" \
    --url-query "id=$deployment_id" "$api/status")
  state=$(jq -r .deploymentState <<< "$status")
  echo "Deployment state: $state"
  case $state in
    PUBLISHED)
      exit 0
      ;;
    FAILED)
      echo "::error::Maven Central rejected the deployment."
      jq .errors <<< "$status"
      exit 1
      ;;
  esac
  sleep 30
done

if [ "$state" = PUBLISHING ]; then
  echo "::warning::Maven Central validated the deployment, but has not finished publishing it after 90 minutes."
  exit 0
fi
echo "::error::Maven Central has not validated the deployment after 90 minutes (state: $state)."
exit 1
