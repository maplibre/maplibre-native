#!/usr/bin/env bash
set -euo pipefail

STYLE_URL="https://demotiles.maplibre.org/style.json"

# Build (same as you do)
cmake --build ./build -- -j8

# Run under gdb with env + args
WGPU_BACKEND_VALIDATION=1 \
exec "$(brew --prefix)/opt/gdb/bin/gdb" \
  -q --command=glfw_gdb.cmd --args \
  ./build/platform/glfw/mbgl-glfw \
    --style "$STYLE_URL" \
    --zoom 2