#!/usr/bin/env bash

set -e
set -o pipefail
set -u

function step { >&2 echo -e "\033[1m\033[36m* $@\033[0m"; }
function finish { >&2 echo -en "\033[0m"; }
trap finish EXIT

VERSION_TAG=${VERSION_TAG:-''}
if [[ -z ${VERSION_TAG} ]]; then
    step "Determining version number from most recent relevant git tag…"
    VERSION_TAG=$( git describe --tags --match=android-v*.*.* --abbrev=0 )
    echo "Found tag: ${VERSION_TAG}"
fi

if [[ $( echo ${VERSION_TAG} | grep --invert-match android-v ) ]]; then
    echo "Error: ${VERSION_TAG} is not a valid android version tag"
    echo "VERSION_TAG should be in format: android-vX.X.X-pre.X"
    exit 1
fi

step "Checking out source code at ${VERSION_TAG}"
git checkout ${VERSION_TAG}