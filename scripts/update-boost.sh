#!/usr/bin/env bash

set -eu

# Extract boost modules that we need
rm -rf vendor/boost/include
mkdir -p vendor/boost/include
bcp --boost="$BOOST" --scan $(find {src,include,test,platform,bin} -name "*.cpp" -o -name "*.hpp") vendor/boost/include

pushd vendor/boost
VERSION=$(sed -n 's/^#define BOOST_LIB_VERSION "\([^"]*\)"$/\1/p' include/boost/version.hpp)
echo "libboost ${VERSION/_/.} for MapLibre Native" > README.md
git add README.md include
popd
