#!/bin/bash -l

set -e
set -x

export CCACHE_DIR="$GITHUB_WORKSPACE/.ccache"
export PATH="$Qt6_DIR/bin:$PATH"

mkdir build && cd build
qt-cmake ../source/ \
  -G Ninja \
  -DCMAKE_BUILD_TYPE="Release" \
  -DCMAKE_C_COMPILER_LAUNCHER="ccache" \
  -DCMAKE_CXX_COMPILER_LAUNCHER="ccache" \
  -DCMAKE_INSTALL_PREFIX=../install \
  -DMBGL_WITH_QT=ON \
  -DMBGL_QT_DEPLOYMENT=ON \
  -DMBGL_QT_LIBRARY_ONLY=ON
ninja
ninja install
cd ../install
ln -s lib64 lib
