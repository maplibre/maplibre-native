#!/bin/bash

if [[ -z ${1+x} ]]; then
  echo "Error: Pass the path to maplibre-gl-native as first argument" 1>&2
  exit 1
fi

if [[ -z ${2+x} ]]; then
  echo "Error: Pass the install prefix as the second argument" 1>&2
  exit 2
fi

if [[ -z ${3+x} ]]; then
  echo "Error: Pass Qt installation path as the third argument" 1>&2
  exit 3
fi

cmake "$1" \
  -G"Ninja Multi-Config" \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_CONFIGURATION_TYPES="Release;Debug" \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0 \
  -DMBGL_WITH_QT=ON \
  -DMBGL_QT_STATIC=ON \
  -DMBGL_QT_LIBRARY_ONLY=ON \
  -DCMAKE_INSTALL_PREFIX="$2" \
  -DCMAKE_FIND_ROOT_PATH="$3/ios" \
  -DCMAKE_PREFIX_PATH="$3/ios"
