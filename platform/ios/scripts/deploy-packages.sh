#!/usr/bin/env bash

set -e
set -o pipefail
set -u

# dynamic environment variables:
#     VERSION_TAG={determined automatically}: Version tag in format ios-vX.X.X-pre.X
#     GITHUB_RELEASE=true: Upload to github

# environment variables and dependencies:
#     - You must run "mbx auth ..." before running
#     - Set GITHUB_TOKEN to a GitHub API access token in your environment to use GITHUB_RELEASE
#     - The "github-release" command is required to use GITHUB_RELEASE

function step { >&2 echo -e "\033[1m\033[36m* $@\033[0m"; }
function finish { >&2 echo -en "\033[0m"; }
trap finish EXIT

buildPackageStyle() {
    local package=$1 style=""
    if [[ ${#} -eq 2 ]]; then
        style="$2"
    fi
    step "Building: make ${package} ${style}"
    FORMAT=${style} make ${package} 
    step "Publishing ${package} with ${style}"
    local file_name=""
    if [ -z ${style} ]
    then
        ./platform/ios/scripts/publish.sh "${PUBLISH_VERSION}"
        file_name=mapbox-ios-sdk-${PUBLISH_VERSION}.zip
    else
        ./platform/ios/scripts/publish.sh "${PUBLISH_VERSION}" ${style}
        file_name=mapbox-ios-sdk-${PUBLISH_VERSION}-${style}.zip
    fi
    if [[ "${GITHUB_RELEASE}" == true ]]; then
        step "Uploading ${file_name} to GitHub"
        github-release upload \
            --tag "ios-v${PUBLISH_VERSION}" \
            --name ${file_name} \
            --file "${BINARY_DIRECTORY}/${file_name}" > /dev/null
    else
        step "CREATED ${file_name} - skipping upload to Github"
    fi
}

export GITHUB_USER=maplibre
export GITHUB_REPO=maplibre-gl-native
export BUILDTYPE=Release
export PACKAGE_FORMAT=xcframework

VERSION_TAG=${VERSION_TAG:-''}
PUBLISH_VERSION=
BINARY_DIRECTORY='build/ios'
GITHUB_RELEASE=${GITHUB_RELEASE:-false}
BUILD_FOR_COCOAPODS=${BUILD_FOR_COCOAPODS:-false}
PUBLISH_PRE_FLAG=''

if [[ -z `which github-release` ]]; then
    step "Installing github-release…"
    brew install github-release
    if [ -z `which github-release` ]; then
        echo "Unable to install github-release. See: https://github.com/aktau/github-release"
        exit 1
    fi
fi

if [[ ${GITHUB_RELEASE} = "true" ]]; then
    GITHUB_RELEASE=true # Assign bool, not just a string
fi

if [[ ${GITHUB_RELEASE} = "false" ]]; then
    GITHUB_RELEASE=false # Assign bool, not just a string
fi

if [[ ${BUILD_FOR_COCOAPODS} = "false" ]]; then
    BUILD_FOR_COCOAPODS=false # Assign bool, not just a string
fi

if [[ ${BUILD_FOR_COCOAPODS} = "true" ]]; then
    BUILD_FOR_COCOAPODS=true # Assign bool, not just a string
fi

if [[ -z ${VERSION_TAG} ]]; then
    step "Determining version number from most recent relevant git tag…"
    VERSION_TAG=$( git describe --tags --match=ios-v*.*.* --abbrev=0 )
    echo "Found tag: ${VERSION_TAG}"
fi

if [[ $( echo ${VERSION_TAG} | grep --invert-match ios-v ) ]]; then
    echo "Error: ${VERSION_TAG} is not a valid iOS version tag"
    echo "VERSION_TAG should be in format: ios-vX.X.X-pre.X"
    if [[ "${GITHUB_RELEASE}" == true ]]; then
        exit 1
    fi
fi

if github-release info --tag ${VERSION_TAG} | grep --quiet "draft: ✗"; then
    echo "Error: ${VERSION_TAG} has already been published on GitHub"
    echo "See: https://github.com/${GITHUB_USER}/${GITHUB_REPO}/releases/tag/${VERSION_TAG}"
    if [[ "${GITHUB_RELEASE}" == true ]]; then
        exit 1
    fi
fi

if [[ "${GITHUB_RELEASE}" == true ]]; then
    PUBLISH_VERSION=$( echo ${VERSION_TAG} | sed 's/^ios-v//' )
    git checkout ${VERSION_TAG}
    step "Deploying version ${PUBLISH_VERSION}…"
else
    PUBLISH_VERSION=${VERSION_TAG}
    step "Building packages for version ${PUBLISH_VERSION} (Not deploying to Github)"
fi

npm install --ignore-scripts
mkdir -p ${BINARY_DIRECTORY}

if [[ "${GITHUB_RELEASE}" == true ]]; then
    step "Create GitHub release…"
    if [[ $( echo ${PUBLISH_VERSION} | awk '/[0-9]-/' ) ]]; then
        PUBLISH_PRE_FLAG='--pre-release'
    fi
    RELEASE_NOTES=$( ./platform/ios/scripts/release-notes.js github )
    github-release release \
        --tag "ios-v${PUBLISH_VERSION}" \
        --name "ios-v${PUBLISH_VERSION}" \
        --draft ${PUBLISH_PRE_FLAG} \
        --description "${RELEASE_NOTES}"
fi

# Used for binary release on Github - includes events SDK
buildPackageStyle "${PACKAGE_FORMAT}" "dynamic"

echo "Binary artifact zip file: ${BINARY_ARTIFACT_ZIP_FILE}"
echo "Binary artifact url: ${BINARY_ARTIFACT_URL}"

if [[ !-z "${BINARY_ARTIFACT_URL}" ]]; then
    step "Creating Swift package…"

    rm -f Package.swift
    cp swift_package_template.swift Package.swift
    sed -i "s/__PACKAGE_URL__/${BINARY_ARTIFACT_URL}/g" Package.swift
    CHECKSUM=$(swift package compute-checksum ${BINARY_ARTIFACT_ZIP_FILE})
    echo "Checksum: ${CHECKSUM}"
    sed -i "s/__PACKAGE_CHECKSUM__/${CHECKSUM}/g" Package.swift
    cat Package.swift
fi


if [[ ${BUILD_FOR_COCOAPODS} == true ]]; then
    # Used for Cocoapods/Carthage
    buildPackageStyle "${PACKAGE_FORMAT}" "dynamic"
    buildPackageStyle "${PACKAGE_FORMAT} SYMBOLS=NO" "stripped-dynamic"
fi




step "Finished deploying ${PUBLISH_VERSION} in $(($SECONDS / 60)) minutes and $(($SECONDS % 60)) seconds"
