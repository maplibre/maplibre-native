#!/bin/bash

node -v
cat /etc/os-release
uname -m

#Build lib files
cd /usr/src/app/
npm ci --ignore-scripts
ccache --clear --set-config cache_dir=~/.ccache
cmake . -B build -G Ninja  -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_C_COMPILER=gcc-10 -DCMAKE_CXX_COMPILER=g++-10
cmake --build build -j $(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null)
cp -R ./lib/ /data/lib/

#Test
xvfb-run --auto-servernum ./build/mbgl-render-test-runner --manifestPath metrics/linux-gcc8-release-style.json
xvfb-run --auto-servernum npm test