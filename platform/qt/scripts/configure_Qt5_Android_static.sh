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

if [[ -z ${4+x} ]]; then
  echo "Error: Pass Android ABI as the fourth argument" 1>&2
  exit 3
fi

cmake "$1" \
-DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake" \
-DANDROID_ABI="$4" \
-DANDROID_NATIVE_API_LEVEL=21 \
-G Ninja \
-DMBGL_WITH_QT=ON \
-DMBGL_QT_STATIC=ON \
-DMBGL_QT_LIBRARY_ONLY=ON \
-DCMAKE_CXX_FLAGS_RELEASE=-g0 \
-DCMAKE_INSTALL_PREFIX="$2" \
-DCMAKE_FIND_ROOT_PATH="$3/android" \
-DCMAKE_PREFIX_PATH="$3/android"
