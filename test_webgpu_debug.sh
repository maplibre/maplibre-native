#!/bin/bash

echo "Testing WebGPU renderer with debug output..."

# Run for 2 seconds and capture all output with MapLibre demo tiles
timeout 2 ./build/platform/glfw/mbgl-glfw --style https://demotiles.maplibre.org/style.json --benchmark --zoom 2 2>&1 | tee /tmp/webgpu_output.log

echo ""
echo "=== Key Logs ==="
echo "Depth buffer:"
grep -i "depth" /tmp/webgpu_output.log | head -5

echo ""
echo "Vertex buffers:"
grep -i "vertex" /tmp/webgpu_output.log | head -5

echo ""
echo "Draw calls:"
grep -i "drawing" /tmp/webgpu_output.log | head -5

echo ""
echo "Errors:"
grep -i "error" /tmp/webgpu_output.log | head -5

echo ""
echo "Total lines of output: $(wc -l /tmp/webgpu_output.log | cut -d' ' -f1)"