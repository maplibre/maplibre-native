#!/bin/bash

validate_version() {
  local version="$1"
  if [[ ! "$version" =~ ^[0-9]+\.[0-9]+\.[0-9]+(-pre.*)?$ ]]; then
    echo "::error::Invalid version format in VERSION file. Expected: X.Y.Z or X.Y.Z-pre*"
    return 1
  fi
  return 0
}

version=$(cat VERSION 2>/dev/null)
if [ -z "$version" ]; then
  echo "::error::VERSION file not found or empty"
  exit 1
fi

if validate_version "$version"; then
  echo "Version validation successful: $version"
  if [[ -n "${GITHUB_ENV:-}" ]]; then
    echo "version=$version" >> "$GITHUB_ENV"
  fi
fi
