#!/usr/bin/env bash

set -euo pipefail

function step { >&2 echo -e "\033[1m\033[36m* $@\033[0m"; }
function finish { >&2 echo -en "\033[0m"; }
trap finish EXIT

#
# iOS release tag format is `vX.Y.Z`; `X.Y.Z` gets passed in
# In the case of stripped builds, we also append the `-stripped`.
#
PUBLISH_VERSION="$1"

if [[ ${#} -eq 2 ]]; then
    PUBLISH_STYLE="-$2"
else
    PUBLISH_STYLE=""
fi

GITHUB_REPO=${GITHUB_REPO:-'maplibre-gl-native'}

#
# zip
#
cd build/ios/pkg
BINARY_ARTIFACT_ZIP_FILE="mapbox-ios-sdk-${PUBLISH_VERSION}${PUBLISH_STYLE}.zip"
step "Compressing ${BINARY_ARTIFACT_ZIP_FILE}…"
rm -f ../${BINARY_ARTIFACT_ZIP_FILE}
zip -yr ../${BINARY_ARTIFACT_ZIP_FILE} *
cd ..

#
# report file sizes
#
step "Echoing file sizes…"
du -sh ${BINARY_ARTIFACT_ZIP_FILE}
du -sch pkg/*
du -sch pkg/dynamic/*

#
# upload
#
DRYRUN=""
if [[ ${SKIP_S3-} ]]; then
    DRYRUN="--dryrun"
fi

PROGRESS=""
if [ -n "${CI:-}" ]; then
    PROGRESS="--no-progress"
fi

step "Uploading ${BINARY_ARTIFACT_ZIP_FILE} to s3… ${DRYRUN}"

aws s3 cp ${BINARY_ARTIFACT_ZIP_FILE} s3://mapbox-gl-native/ios/builds --acl public-read ${PROGRESS} ${DRYRUN}
BINARY_ARTIFACT_URL=https://mapbox-gl-native.s3.us-east-2.amazonaws.com/ios/builds/${BINARY_ARTIFACT_ZIP_FILE}
export BINARY_ARTIFACT_URL
export BINARY_ARTIFACT_ZIP_FILE
echo "URL: ${BINARY_ARTIFACT_URL}"
echo "mapbox-gl-native is currently hardcoded"

#
# upload & update snapshot
#
if [[ ${PUBLISH_VERSION} =~ "snapshot" ]]; then
    step "Updating ${PUBLISH_VERSION} to ${PUBLISH_STYLE}…"
    GENERIC_BINARY_ARTIFACT_ZIP_FILE="mapbox-ios-sdk-${PUBLISH_VERSION}.zip"
    aws s3 cp \
        s3://mapbox-gl-native/ios/builds/${BINARY_ARTIFACT_ZIP_FILE} \
        s3://mapbox-gl-native/ios/builds/${GENERIC_BINARY_ARTIFACT_ZIP_FILE} --acl public-read ${PROGRESS} ${DRYRUN}
fi

#
# verify upload integrity
#

step "Validating local and remote checksums…"

if [[ ! ${SKIP_S3-} ]]; then
    curl --output remote-${BINARY_ARTIFACT_ZIP_FILE} ${BINARY_ARTIFACT_URL}
    LOCAL_CHECKSUM=$( shasum -a 256 -b ${BINARY_ARTIFACT_ZIP_FILE} | cut -d ' ' -f 1 )
    REMOTE_CHECKSUM=$( shasum -a 256 -b remote-${BINARY_ARTIFACT_ZIP_FILE} | cut -d ' ' -f 1 )

    if [ "${LOCAL_CHECKSUM}" == "${REMOTE_CHECKSUM}" ]; then
        echo "Checksums match: ${LOCAL_CHECKSUM}"
    else
        echo "Checksums did not match: ${LOCAL_CHECKSUM} != ${REMOTE_CHECKSUM}"
        exit 1
    fi
fi
