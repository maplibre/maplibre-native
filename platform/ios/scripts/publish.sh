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

GITHUB_REPO=${GITHUB_REPO:-'mapbox-gl-native'}

#
# zip
#
cd build/ios/pkg
ZIP_FILENAME="mapbox-ios-sdk-${PUBLISH_VERSION}${PUBLISH_STYLE}.zip"
step "Compressing ${ZIP_FILENAME}…"
rm -f ../${ZIP_FILENAME}
zip -yr ../${ZIP_FILENAME} *
cd ..

#
# report file sizes
#
step "Echoing file sizes…"
du -sh ${ZIP_FILENAME}
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

step "Uploading ${ZIP_FILENAME} to s3… ${DRYRUN}"

aws s3 cp ${ZIP_FILENAME} s3://mapbox/mapbox-gl-native/ios/builds/ --acl public-read ${PROGRESS} ${DRYRUN}
S3_URL=https://mapbox.s3.amazonaws.com/mapbox-gl-native/ios/builds/${ZIP_FILENAME}
echo "URL: ${S3_URL}"
echo "mapbox-gl-native is currently hardcoded"

#
# upload & update snapshot
#
if [[ ${PUBLISH_VERSION} =~ "snapshot" ]]; then
    step "Updating ${PUBLISH_VERSION} to ${PUBLISH_STYLE}…"
    GENERIC_ZIP_FILENAME="mapbox-ios-sdk-${PUBLISH_VERSION}.zip"
    aws s3 cp \
        s3://mapbox/mapbox-gl-native/ios/builds/${ZIP_FILENAME} \
        s3://mapbox/mapbox-gl-native/ios/builds/${GENERIC_ZIP_FILENAME} --acl public-read ${PROGRESS} ${DRYRUN}
fi

#
# verify upload integrity
#

step "Validating local and remote checksums…"

if [[ ! ${SKIP_S3-} ]]; then
    curl --output remote-${ZIP_FILENAME} ${S3_URL}
    LOCAL_CHECKSUM=$( shasum -a 256 -b ${ZIP_FILENAME} | cut -d ' ' -f 1 )
    REMOTE_CHECKSUM=$( shasum -a 256 -b remote-${ZIP_FILENAME} | cut -d ' ' -f 1 )

    if [ "${LOCAL_CHECKSUM}" == "${REMOTE_CHECKSUM}" ]; then
        echo "Checksums match: ${LOCAL_CHECKSUM}"
    else
        echo "Checksums did not match: ${LOCAL_CHECKSUM} != ${REMOTE_CHECKSUM}"
        exit 1
    fi
fi
