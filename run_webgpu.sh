#!/bin/bash

# Enhanced startup script for WebGPU GLFW app
# Handles intermittent Metal initialization issues

echo "MapLibre Native WebGPU Demo"
echo "============================"
echo ""

STYLE_URL="./demotiles.json"

# Build the project first
echo "Building WebGPU implementation..."
if cmake --build ./build --target mbgl-glfw -- -j8 > /dev/null 2>&1; then
    echo "✓ Build successful"
else
    echo "✗ Build failed. Running with verbose output:"
    cmake --build ./build --target mbgl-glfw -- -j8
    exit 1
fi
echo ""

# Function to ensure clean state
cleanup_processes() {
    if pgrep -f "mbgl-glfw" > /dev/null; then
        echo "Cleaning up existing processes..."
        pkill -9 -f mbgl-glfw 2>/dev/null
        sleep 1  # Give Metal time to release resources
    fi
}

# Function to try starting the app
try_start() {
    local attempt=$1
    echo "Starting attempt $attempt..."
    
    # Small delay between attempts to let Metal reset
    if [ $attempt -gt 1 ]; then
        sleep 0.5
    fi
    
    # Select platform-specific environment overrides
    local cmd=("./build/platform/glfw/mbgl-glfw" "--style" "$STYLE_URL" "--zoom" "5" "--backend" "webgpu")
    if [[ "$(uname -s)" == "Darwin" ]]; then
        cmd=("env" "MTL_DEBUG_LAYER=1" "MTL_SHADER_VALIDATION=1" "${cmd[@]}")
    fi

    # Start in background and check if it stays alive
    "${cmd[@]}" &
    local pid=$!
    
    # Wait a bit to see if it crashes immediately
    sleep 2
    
    if kill -0 $pid 2>/dev/null; then
        echo "✓ Successfully started (PID: $pid)"
        echo ""
        echo "MapLibre WebGPU is running!"
        echo "Controls:"
        echo "  - Mouse: Pan and zoom the map"
        echo "  - Shift+drag: Tilt the map"
        echo "  - Ctrl+drag: Rotate the map"
        echo "  - S: Cycle through styles"
        echo "  - Tab: Toggle debug info"
        echo "  - Esc: Quit"
        echo ""
        echo "Press Ctrl+C to stop"
        
        # Wait for the process
        wait $pid
        return 0
    else
        echo "  Attempt $attempt failed"
        return 1
    fi
}

# Main execution
cleanup_processes

# Single attempt (adjust attempts here if needed)
if try_start 1; then
    exit 0
fi

echo ""
echo "Failed to start the WebGPU demo."
echo "This can happen if Dawn cannot initialize the adapter."
echo ""
echo "Troubleshooting:"
echo "1. Wait a few seconds and try again"
echo "2. Check Activity Monitor for stuck mbgl-glfw processes"
echo "3. Restart your terminal"
echo "4. If persistent, reboot your Mac to reset Metal state"

exit 1
