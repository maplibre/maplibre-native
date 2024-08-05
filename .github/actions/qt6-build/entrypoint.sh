#!/bin/bash -l

source /opt/rh/gcc-toolset-11/enable

set -e
set -x

export CCACHE_DIR="$GITHUB_WORKSPACE/.ccache"
export PATH="$QT_ROOT_DIR/bin:$PATH"

echo "Downloading and installing Rust..."
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --profile minimal
. "$HOME/.cargo/env"

mkdir build && cd build
qt-cmake ../source/ \
  -G Ninja \
  -DCMAKE_BUILD_TYPE="Release" \
  -DCMAKE_C_COMPILER_LAUNCHER="ccache" \
  -DCMAKE_CXX_COMPILER_LAUNCHER="ccache" \
  -DMLN_WITH_QT=ON \
  -DMLN_QT_IGNORE_ICU=OFF
ninja
