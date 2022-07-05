#!/bin/bash

if [[ -z ${1+x} ]]; then
  echo "Error: Pass the path to maplibre-gl-native as first argument" 1>&2
  exit 1
fi

if [[ -z ${2+x} ]]; then
  echo "Error: Pass the install prefix as the second argument" 1>&2
  exit 2
fi

qt-cmake "$1" \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=10.14 \
  -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
  -DMBGL_WITH_QT=ON \
  -DMBGL_QT_STATIC=ON \
  -DMBGL_QT_LIBRARY_ONLY=ON \
  -DCMAKE_INSTALL_PREFIX="$2"
