#!/bin/bash

set -eo pipefail

pushd ../../../../../

echo "------ Determining Maplibre version and hash ------"
sem_version=0.0.0
hash=$(git log | head -1 | awk '{ print $2 }' | cut -c 1-10) && true

echo "------ Configuring project ------"

# Build parameters
token_file=~/.maplibre
token_file2=~/maplibre
token="$(cat $token_file 2>/dev/null || cat $token_file2 2>/dev/null || echo $MLN_API_KEY)"
flavor="legacy" # Renderer build flavor: legacy, drawable, split

args=("--sem-ver" "$sem_version" "--hash" "$hash")

while [[ $# -gt 0 ]]; do
   case $1 in
   --flavor)
      shift
      flavor="$1"
      shift
      ;;
   --teamid)
      shift
      args+=("--team-id" "$1")
      shift
      ;;
   --bundleidprefix)
      shift
      args+=("--bundleidprefix" "$1")
      shift
      ;;
   --apikey)
      shift
      args+=("--token" "$1")
      shift
      ;;
   -*|--*)
      echo "Unknown option $1"
      exit 1
      ;;
   esac
done

bash "platform/ios/platform/ios/scripts/bazel-generate-plists.sh" "${args[@]}"

echo "------ Building Maplibre version: $sem_version hash: $hash ------"

# Generate the Xcode project
# Example invocation: ./bazel-xcodeproj.sh flavor split teamid 1234567890
# Find your team ID inside a .mobileprovision file or in your keychain (Apple development: your@email -> Get Info -> Organizational Unit)
bazel run //platform/ios:xcodeproj --@rules_xcodeproj//xcodeproj:extra_common_flags="--//:renderer=$flavor --//:maplibre_platform=ios"

popd
