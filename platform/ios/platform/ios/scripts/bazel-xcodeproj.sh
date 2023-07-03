#!/bin/bash

set -eo pipefail

pushd ../../../../../

echo "------ Determining Maplibre version and hash ------"
sem_version=0.0.0
hash=$(git log | head -1 | awk '{ print $2 }' | cut -c 1-10) && true

echo "------ Configuring project ------"
# Move and configure the info plists with the semantic version and hash.
temp_info_static_plist="platform/ios/build/Info-static.plist"
temp_info_plist="platform/ios/build/Info.plist"
# Move and configure the app plist
temp_app_info_plist="platform/ios/build/Info-app.plist"

if [ ! -d platform/ios/build ]; then
   mkdir platform/ios/build
fi

cp platform/ios/platform/ios/framework/Info-static.plist "$temp_info_static_plist"
cp platform/ios/platform/ios/framework/Info.plist "$temp_info_plist"
cp platform/ios/platform/ios/app/Info.plist "$temp_app_info_plist"

# Replace semver and hash
plutil -replace MLNSemanticVersionString -string "$sem_version" "$temp_info_static_plist"
plutil -replace MLNCommitHash -string "$hash" "$temp_info_static_plist"
plutil -replace MLNSemanticVersionString -string "$sem_version" "$temp_info_plist"
plutil -replace MLNCommitHash -string "$hash" "$temp_info_plist"

# Build parameters
token_file=~/.maplibre
token_file2=~/maplibre
token="$(cat $token_file 2>/dev/null || cat $token_file2 2>/dev/null || echo $MLN_API_KEY)"
flavor="legacy" # Renderer build flavor: legacy, drawable, split
teamid="0000000000" # Provisioning profile team ID, required for targeting physical devices

while [[ $# -gt 0 ]]; do
   case $1 in
   --flavor)
      shift
      flavor="$1"
      shift
      ;;
   --teamid)
      shift
      teamid="$1"
      shift
      ;;
   --apikey)
      shift
      token="$1"
      shift
      ;;
   -*|--*)
      echo "Unknown option $1"
      exit 1
      ;;
   esac
done

# Insert API Key
echo "Inserting MapLibre API key..."
if [ "$token" ]; then
    plutil -replace MLNApiKey -string $token "$temp_app_info_plist"
    echo "API key insertion successful"
else
    echo "Warning: Missing API key."
fi

# Generate the team ID for Xcode device provisioning
cat > platform/ios/bazel/config.bzl <<EOF
APPLE_MOBILE_PROVISIONING_PROFILE_TEAM_ID = "$teamid"
EOF

echo "------ Building Maplibre version: $sem_version hash: $hash ------"

# Generate the Xcode project
# Example invocation: ./bazel-xcodeproj.sh flavor split teamid 1234567890
# Find your team ID inside a .mobileprovision file or in your keychain (Apple development: your@email -> Get Info -> Organizational Unit)
bazel run //platform/ios:xcodeproj --@rules_xcodeproj//xcodeproj:extra_common_flags="--//:renderer=$flavor --//:maplibre_platform=ios"

popd
