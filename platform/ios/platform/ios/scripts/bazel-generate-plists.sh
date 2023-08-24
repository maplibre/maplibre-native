#!/bin/bash

set -eo pipefail

echo "Generating plist files..."

sem_version="0.0.0"
hash=""
teamid="0000000000" # Provisioning profile team ID, required for targeting physical devices
bundleidprefix="org.maplibre.app" # bundle id prefix

while [[ $# -gt 0 ]]; do
   case $1 in
   --sem-ver)
      shift
      sem_version="$1"
      shift
      ;;
   --hash)
      shift
      hash="$1"
      shift
      ;;
   --token)
      shift
      token="$1"
      shift
      ;;
   --team-id)
      shift
      teamid="$1"
      shift
      ;;
   --bundleidprefix)
      shift
      bundleidprefix="$1"
      shift
      ;;
   -*|--*)
      echo "Unknown option $1"
      exit 1
      ;;
   esac
done

# Move and configure the info plists with the semantic version and hash.
temp_info_static_plist="platform/ios/build/Info-static.plist"
temp_info_plist="platform/ios/build/Info.plist"
# Move and configure the app plist
temp_app_info_plist="platform/ios/build/Info-app.plist"
# Move and configure the BenchmarkApp plist
temp_benchmark_info_plist="platform/ios/build/Info-benchmark.plist"

if [ ! -d platform/ios/build ]; then
   mkdir platform/ios/build
fi

cp platform/ios/platform/ios/framework/Info-static.plist "$temp_info_static_plist"
cp platform/ios/platform/ios/framework/Info.plist "$temp_info_plist"
cp platform/ios/platform/ios/app/Info.plist "$temp_app_info_plist"
cp platform/ios/platform/ios/benchmark/Info.plist "$temp_benchmark_info_plist"

# Replace semver and hash
plutil -replace MLNSemanticVersionString -string "$sem_version" "$temp_info_static_plist"
plutil -replace MLNCommitHash -string "$hash" "$temp_info_static_plist"
plutil -replace MLNSemanticVersionString -string "$sem_version" "$temp_info_plist"
plutil -replace MLNCommitHash -string "$hash" "$temp_info_plist"

# Insert API Key
echo "Inserting MapLibre API key..."
if [ "$token" ]; then
    plutil -replace MLNApiKey -string $token "$temp_app_info_plist"
    plutil -replace MLNApiKey -string $token "$temp_benchmark_info_plist"
    echo "API key insertion successful"
else
    echo "Warning: Missing API key."
fi

# Generate the team ID for Xcode device provisioning
echo "Updating bazel/config.bzl..."
cat > platform/ios/bazel/config.bzl <<EOF
APPLE_MOBILE_PROVISIONING_PROFILE_TEAM_ID = "$teamid"
APPLE_MOBILE_PROVISIONING_PROFILE_NAME = "iOS Team Provisioning Profile: *"
BUNDLE_ID_PREFIX = "$bundleidprefix"
EOF
