#!/bin/bash

set -e
set -o pipefail

PACKAGE_JSON_VERSION=`node -e "console.log(require('./package.json').version)"`
RELEASE_TAG="node-v${PACKAGE_JSON_VERSION}"

if [[ "${CIRCLE_TAG}" == "${RELEASE_TAG}" ]] || [[ "${PUBLISH:-}" == true ]]; then
    # Changes to the version targets here should happen in tandem with updates to the
    # EXCLUDE_NODE_ABIS property in cmake/node.cmake and the "node" engines property in
    # package.json.
    for TARGET in 22.0.0 24.0.0 26.0.0; do
        rm -rf build/stage

        if [[ "${BUILDTYPE}" == "RelWithDebInfo" ]]; then
            ./node_modules/.bin/node-pre-gyp package --target="${TARGET}" $@
        elif [[ "${BUILDTYPE}" == "Debug" ]]; then
            ./node_modules/.bin/node-pre-gyp package --target="${TARGET}" --debug $@
        else
            echo "error: must provide either Debug or RelWithDebInfo for BUILDTYPE"
            exit 1
        fi

        gh release upload "${RELEASE_TAG}" build/stage/${RELEASE_TAG}/*.tar.gz --clobber
    done
fi
