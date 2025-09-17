#!/bin/bash

set -e

if [ $# -lt 1 ] || [ $# -gt 2 ]; then
  echo "Usage: $0 <tag> [<commit-sha>]"
  echo "  <tag>        Git tag name to create or verify"
  echo "  <commit-sha> Optional commit SHA to use (defaults to HEAD)"
  echo ""
  echo "This script will:"
  echo "  - Check if the tag exists and matches the specified commit"
  echo "  - Create and push the tag if it doesn't exist"
  echo "  - Exit with error if tag exists but points to different commit"
  exit 1
fi

tag=$1
commit_sha=${2:-$(git rev-parse HEAD)}

if [ -z "$(git config user.name)" ]; then
  git config user.name "MapLibre Team"
fi

if [ -z "$(git config user.email)" ]; then
  git config user.email "team@maplibre.org"
fi

if git rev-parse "$tag" >/dev/null 2>&1; then
  tag_sha=$(git rev-parse "$tag^{commit}")
  if [ "$tag_sha" = "$commit_sha" ]; then
    echo "✅ Tag $tag exists and matches specified commit SHA."
    exit 0
  else
    echo "::error::❌ Tag $tag exists but points to a different commit."
    echo "  Expected: $commit_sha"
    echo "  Actual:   $tag_sha"
    exit 1
  fi
else
  git tag -a "$tag" -m "Publish $tag" "$commit_sha"
  git push origin "$tag"
  echo "✅ Created tag $tag for commit $commit_sha."
fi
