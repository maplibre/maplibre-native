#!/bin/bash

# Run WebGPU for 3 seconds and capture output
timeout 3 ./build/platform/glfw/mbgl-glfw --style https://demotiles.maplibre.org/style.json --benchmark --zoom 2 2>&1 | tee /tmp/webgpu_output.log