#!/usr/bin/env bash
# Download the resources (e.g. images) used by the DocC documentation from S3.
# Requires aws-cli. See docs/mdbook/src/platforms/ios/ios-documentation.md.
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

aws s3 sync --no-sign-request "s3://maplibre-native/ios-documentation-resources" "$script_dir/../MapLibre.docc/Resources"
