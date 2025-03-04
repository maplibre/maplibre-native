#!/usr/bin/env bash

set -e
set -o pipefail
set -u

# dynamic environment variables:
#     VERSION_TAG={determined automatically}: Version tag in format ios-vX.X.X-pre.X

# environment variables and dependencies:
#     - You must run "mbx auth ..." before running
#     - Set GITHUB_TOKEN to a GitHub API access token in your environment
#     - The "github-release" command is required

function step { >&2 echo -e "\033[1m\033[36m* $@\033[0m"; }
function finish { >&2 echo -en "\033[0m"; }
trap finish EXIT

export GITHUB_USER=maplibre
export GITHUB_REPO=maplibre-native
export BUILDTYPE=Release
export DISTRIBUTION_GITHUB_REPO=https://api.github.com/repos/maplibre/maplibre-gl-native-distribution

VERSION_TAG=${VERSION_TAG:-''}
PUBLISH_VERSION=
BINARY_DIRECTORY='build/ios/pkg/dynamic'
PUBLISH_PRE_FLAG=''

uploadToGithub() {
    local file_path=$1
    local version_tag=$2
    file_name=${file_path##*/}
    github-release upload \
        --tag "${version_tag}" \
        --name ${file_name} \
        --file "${file_path}" > /dev/null

    EXT_TARGET_GITHUB_URL="https://github.com/${GITHUB_USER}/${GITHUB_REPO}/releases/download/${version_tag}/${file_name}"
}

rm -rf ${BINARY_DIRECTORY}

if [[ -z `which github-release` ]]; then
    step "Installing github-release…"
    brew install github-release
    if [ -z `which github-release` ]; then
        echo "Unable to install github-release. See: https://github.com/aktau/github-release"
        exit 1
    fi
fi

if [[ -z `which jq` ]]; then
    step "Installing jq …"
    brew install jq
    if [ -z `which jq` ]; then
        echo "Unable to install which jq."
        exit 1
    fi
fi

if [[ -z ${VERSION_TAG} ]]; then
    step "Determining version number from most recent relevant git tag…"
    VERSION_TAG=$( git describe --tags --match=ios-v*.*.* --abbrev=0 )
    echo "Found tag: ${VERSION_TAG}"
fi

if [[ $( echo ${VERSION_TAG} | grep --invert-match ios-v ) ]]; then
    echo "Error: ${VERSION_TAG} is not a valid iOS version tag"
    echo "VERSION_TAG should be in format: ios-vX.X.X-pre.X"
    exit 1
fi
if github-release info --tag ${VERSION_TAG} | grep --quiet "draft: ✗"; then
    echo "Error: ${VERSION_TAG} has already been published on GitHub"
    echo "See: https://github.com/${GITHUB_USER}/${GITHUB_REPO}/releases/tag/${VERSION_TAG}"
    exit 1
fi
PUBLISH_VERSION=$( echo ${VERSION_TAG} | sed 's/^ios-v//' )

git checkout ${VERSION_TAG}

npm install --ignore-scripts
mkdir -p ${BINARY_DIRECTORY}

step "Building XCFramework"
bazel build //platform/ios:MapLibre.dynamic

step "Copying Bazel output"
BUILT_XCFRAMEWORK="$(bazel info execution_root)/$(bazel cquery --output=files //platform/ios:MapLibre.dynamic)"
MAPLIBRE_ZIP_FILE="MapLibre-${PUBLISH_VERSION}.zip"
cp "$BUILT_XCFRAMEWORK" "$MAPLIBRE_ZIP_FILE"

step "Create GitHub release…"
RELEASE_NOTES=$( ./scripts/release-notes.js github )

if [[ -z "${RELEASE_NOTES}" ]]; then
    echo "Release notes cannot be empty."
    exit 1
fi

github-release release \
    --tag "ios-v${PUBLISH_VERSION}" \
    --name "ios-v${PUBLISH_VERSION}" \
    --description "${RELEASE_NOTES}"

step "Uploading ${BINARY_DIRECTORY}/${MAPLIBRE_ZIP_FILE} to github release [${VERSION_TAG}]"
uploadToGithub "${BINARY_DIRECTORY}/${MAPLIBRE_ZIP_FILE}" "${VERSION_TAG}"
MAPLIBRE_ZIP_FILE_URL=$EXT_TARGET_GITHUB_URL

step "Creating Swift package…"

rm -f Package.swift
cp scripts/swift_package_template.swift Package.swift

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

setTarget "MAPLIBRE" "${BINARY_DIRECTORY}/${MAPLIBRE_ZIP_FILE}" "${MAPLIBRE_ZIP_FILE_URL}"

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

step "Finished deploying ${PUBLISH_VERSION} in $(($SECONDS / 60)) minutes and $(($SECONDS % 60)) seconds"

pod trunk push MapLibre.podspec
