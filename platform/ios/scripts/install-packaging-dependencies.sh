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

step "Finished installing packaging dependencies"
