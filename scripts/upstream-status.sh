#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

if ! git remote get-url upstream &>/dev/null; then
  echo "Adding upstream remote..."
  git remote add upstream https://github.com/maplibre/maplibre-native.git
fi

echo "Fetching upstream..."
git fetch upstream --quiet

echo ""
echo "=== Upstream (official MapLibre Native) ==="
git log upstream/main -1 --format='  %h  %s  (%cr)'

echo ""
echo "=== This repo (main) ==="
git log main -1 --format='  %h  %s  (%cr)'

VERSION="$(cat platform/android/VERSION 2>/dev/null || echo unknown)"
LATEST_ANDROID_TAG="$(git tag -l 'android-v*' --sort=-v:refname | head -1)"

echo ""
echo "=== Versions ==="
echo "  This fork Android SDK:  $VERSION"
echo "  Latest upstream tag:    ${LATEST_ANDROID_TAG:-unknown}"

echo ""
echo "=== Custom 3D files (diff vs upstream/main) ==="
for f in \
  src/mbgl/util/gltf_loader.cpp \
  src/mbgl/map/transform_state.cpp \
  platform/android/MapLibreAndroid/src/cpp/style/layers/gltf_model_layer_host.cpp \
  platform/android/MapLibreAndroid/src/main/java/org/maplibre/android/style/layers/GltfModelLayer.java \
  shaders/background.fragment.glsl \
  shaders/fill_extrusion.fragment.glsl; do
  if git diff --quiet upstream/main -- "$f" 2>/dev/null; then
    echo "  (unchanged) $f"
  else
    lines="$(git diff upstream/main --stat -- "$f" | tail -1)"
    echo "  (modified)  $f  —  $lines"
  fi
done

echo ""
echo "Tip: git diff upstream/main -- <file>   to inspect a specific change"
echo "Docs: UPSTREAM.md"
