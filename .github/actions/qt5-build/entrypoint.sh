#!/bin/bash -l

source scl_source enable devtoolset-11 rh-git218

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
  -DMLN_WITH_QT=ON \
  -DMLN_QT_IGNORE_ICU=OFF
ninja
