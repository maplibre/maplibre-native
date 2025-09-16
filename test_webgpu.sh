#!/bin/bash

# Run WebGPU for 3 seconds and capture output
timeout 3 ./build/bin/mbgl-glfw --style test/fixtures/api/water.json --benchmark --zoom 2 2>&1 | grep -E "(FILL Drawing|successfully created drawable|tiles rendered|drawables created|shader assigned)" | tail -20