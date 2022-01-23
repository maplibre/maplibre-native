#!/usr/bin/env bash

set -e
set -o pipefail

rm -rf ./build

# Install these dependencies, if they aren't there already.
# brew install cmake ccache glfw ninja pkgconfig qt chargepoint/xcparse/xcparse libuv
# brew cask install google-cloud-sdk
# pip install ansi2html scipy

cmake . -B build -G Ninja -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0 -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_BUILD_TYPE=Debug -DMBGL_WITH_COVERAGE=ON

sed -i -e 's/$(ARCHS_STANDARD)/x86_64/g' build/build.ninja
sed -i -e 's/-arch arm64e/-arch x86_64/g' build/build.ninja

cmake --build build -j $(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null)
