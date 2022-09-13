#!/bin/bash -l

source scl_source enable devtoolset-8 rh-git218

set -e
set -x

export CCACHE_DIR="$GITHUB_WORKSPACE/.ccache"
export PATH="$Qt5_Dir/bin:$PATH"

mkdir build && cd build
cmake ../source/ \
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
