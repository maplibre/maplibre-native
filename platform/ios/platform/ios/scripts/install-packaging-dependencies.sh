#!/usr/bin/env bash

set -euo pipefail

COCOAPODS_VERSION="1.7.5"
JAZZY_VERSION="0.11.1"
CILAUNCH=${CILAUNCH:-false}

function step { >&2 echo -e "\033[1m\033[36m* $@\033[0m"; }
function finish { >&2 echo -en "\033[0m"; }
trap finish EXIT

step "Installing packaging dependencies…"

##
## aws
##
if [ -z `which aws` ]; then
    brew install awscli
else
    echo "Found awscli"
fi

if [[ "${CILAUNCH}" == true ]]; then
    sudo="sudo"
else
    sudo=""
fi

##
## jazzy
##
if [[ -z `which jazzy` || $(jazzy -v) != "jazzy version: ${JAZZY_VERSION}" ]]; then
    step "Installing jazzy…"

    $sudo gem install jazzy -v $JAZZY_VERSION --no-document

    if [ -z `which jazzy` ]; then
        echo "Unable to install jazzy ($JAZZY_VERSION). See https://github.com/mapbox/mapbox-gl-native-ios/blob/master/platform/ios/INSTALL.md"
        exit 1
    fi
else
    echo "Found jazzy (${JAZZY_VERSION})"
fi

# fix for jazzy on M1/ARM https://github.com/realm/jazzy/issues/1259
if [[ $(uname -p) == 'arm' ]]; then
    step "Installing ffi, to run jazzy on M1/ARM chip…"
    $sudo gem install --user-install ffi -- --enable-libffi-alloc
fi

step "Finished installing packaging dependencies"
