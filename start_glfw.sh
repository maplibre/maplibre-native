#!/bin/bash

# Simple persistent starter for GLFW WebGPU
# Keeps trying until it successfully starts

echo "Starting GLFW WebGPU (will retry until successful)..."

# Kill any existing instances
pkill -9 -f mbgl-glfw 2>/dev/null

while true; do
    # Try to start the app
    ./build/platform/glfw/mbgl-glfw &
    PID=$!
    
    # Wait a bit to see if it stays running
    sleep 2
    
    # Check if still running
    if kill -0 $PID 2>/dev/null; then
        echo "✓ Successfully started with PID: $PID"
        echo "Application is running. Press Ctrl+C to stop."
        
        # Wait for the process to end
        wait $PID
        echo "Application stopped."
        break
    else
        # Failed, try again
        echo "Failed to start, retrying..."
        pkill -9 -f mbgl-glfw 2>/dev/null
        sleep 0.5
    fi
done