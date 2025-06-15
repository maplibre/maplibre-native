#!/bin/bash

validate_version() {
  local version="$1"
  if [[ ! "$version" =~ ^[0-9]+\.[0-9]+\.[0-9]+(-pre.*)?$ ]]; then
    echo "::error::Invalid version '$version' in $(realpath "$version_file"). Expected: X.Y.Z or X.Y.Z-pre*"
    return 1
  fi
  return 0
}

if [[ $# -ne 1 ]]; then
  echo "Usage: $0 <version-file>"
  echo ""
  echo "This script will:"
  echo "  - Validate version format in specified file"
  echo "  - Export version to GITHUB_ENV if in GitHub Actions"
  exit 1
fi

version_file="$1"

if [ ! -f "$version_file" ]; then
  echo "::error::Version file not found: $(realpath "$version_file")"
  exit 1
fi

version=$(cat "$version_file" 2>/dev/null)
if [ -z "$version" ]; then
  echo "::error::Version file is empty: $(realpath "$version_file")"
  exit 1
fi

if validate_version "$version"; then
  echo "✅ Version validation successful: $version"
  if [[ -n "${GITHUB_ENV:-}" ]]; then
    echo "version=$version" >> "$GITHUB_ENV"
  fi
  exit 0
else
  exit 1
fi
