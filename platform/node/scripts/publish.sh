#!/bin/bash

set -e
set -o pipefail

PACKAGE_JSON_VERSION=`node -e "console.log(require('./package.json').version)"`

if [[ "${CIRCLE_TAG}" == "node-v${PACKAGE_JSON_VERSION}" ]] || [[ "${PUBLISH:-}" == true ]]; then
    # Changes to the version targets here should happen in tandem with updates to the
    # EXCLUDE_NODE_ABIS property in cmake/node.cmake and the "node" engines property in
    # package.json.
    for TARGET in 14.0.0 16.0.0 18.0.0; do
        rm -rf build/stage

        if [[ "${BUILDTYPE}" == "RelWithDebInfo" ]]; then
            ./node_modules/.bin/node-pre-gyp package --target="${TARGET}" $@
            ./node_modules/.bin/node-pre-gyp-github publish --release
        elif [[ "${BUILDTYPE}" == "Debug" ]]; then
            ./node_modules/.bin/node-pre-gyp package --target="${TARGET}" --debug $@
            ./node_modules/.bin/node-pre-gyp-github publish --release
        else
            echo "error: must provide either Debug or RelWithDebInfo for BUILDTYPE"
            exit 1
        fi
    done
fi
