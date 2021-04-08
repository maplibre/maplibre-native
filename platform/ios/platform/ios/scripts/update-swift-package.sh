#!/usr/bin/env bash

set -e
set -o pipefail
set -u

# USE THIS SCRIPT ONLY FOR MANUAL UPDATE
# IN CASE AUTOPMATED DEPLOY FAILS

function step { >&2 echo -e "\033[1m\033[36m* $@\033[0m"; }
function finish { >&2 echo -en "\033[0m"; }
trap finish EXIT

step "Creating Swift package…"

rm -f Package.swift
cp platform/ios/scripts/swift_package_template.swift Package.swift

export GITHUB_USER=maplibre
export GITHUB_REPO=maplibre-gl-native
export BUILDTYPE=Release
export DISTRIBUTION_GITHUB_REPO=https://api.github.com/repos/maplibre/maplibre-gl-native-distribution

VERSION_TAG="ios-v5.11.0"
PUBLISH_VERSION="5.11.0"
BINARY_DIRECTORY='/Volumes/Data/Temp/maplibre'
PUBLISH_PRE_FLAG=''
S3_DISTRIBUTION=false
MAPBOX_ZIP_FILE_URL="https://github.com/maplibre/maplibre-gl-native/releases/download/ios-v5.11.0/Mapbox-5.11.0.zip"
MAPBOX_ZIP_FILE="Mapbox-5.11.0.zip"
DIST_GITHUB_TOKEN="PUT YOUR TOKEN HERE:"

setTarget() {
    local token=$1
    local targetBinaryPath=$2
    local targetUrl=$3
    local checksum=$(swift package compute-checksum ${targetBinaryPath})
    
    echo "token=[${token}]"
    echo "url=[$targetUrl]"
    echo "cheksum=[$checksum]"

    sed -i '' -e 's|'"${token}_PACKAGE_URL"'|'"${targetUrl}"'|g' Package.swift
    sed -i '' -e 's|'"${token}_PACKAGE_CHECKSUM"'|'"${checksum}"'|g' Package.swift
}

setTarget "MAPBOX" "${BINARY_DIRECTORY}/${MAPBOX_ZIP_FILE}" "${MAPBOX_ZIP_FILE_URL}"

step "Publishing Swift package…"

if [[ -z "${DIST_GITHUB_TOKEN}" ]]; then
    echo "DIST_GITHUB_TOKEN is not set"
    exit 1
fi

UPDATED_PACKAGE_CONTENT=$(cat Package.swift | base64)
ORIGINAL_FILE_SHA="$(curl -H "Authorization: token ${DIST_GITHUB_TOKEN}" ${DISTRIBUTION_GITHUB_REPO}/contents/Package.swift | jq -r -c '.sha')"
PAYLOAD="{\"message\": \"New release: ${PUBLISH_VERSION}\",\"content\": \"$UPDATED_PACKAGE_CONTENT\", \"sha\": \"$ORIGINAL_FILE_SHA\"}"

# package definition update
curl -v -X PUT \
    -H "Authorization: token ${DIST_GITHUB_TOKEN}" \
    "${DISTRIBUTION_GITHUB_REPO}"/contents/Package.swift \
    -d "${PAYLOAD}"

# setting up new tag
curl -v -X POST \
    -H "Authorization: token ${DIST_GITHUB_TOKEN}" \
    -H "Accept: application/vnd.github.v3+json" \
    "${DISTRIBUTION_GITHUB_REPO}"/releases \
    -d "{\"tag_name\":\"${PUBLISH_VERSION}\"}"