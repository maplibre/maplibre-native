#!/bin/bash

# Stable startup with initialization delay
echo "Starting MapLibre WebGPU (stable mode)..."

# Kill any existing processes
pkill -9 -f mbgl-glfw 2>/dev/null

# Give the system time to clean up resources
echo "Waiting for clean state..."
sleep 2

# Set environment variables that might help with stability
export METAL_DEVICE_WRAPPER_TYPE=1
export METAL_DEBUG_ERROR_MODE=0

# Try to start
attempt=0
while [ $attempt -lt 10 ]; do
    attempt=$((attempt + 1))
    
    # Add small random delay to avoid timing issues
    sleep 0.$((RANDOM % 10))
    
    ./build/platform/glfw/mbgl-glfw &
    PID=$!
    
    # Check if it's still running after 3 seconds
    sleep 3
    
    if kill -0 $PID 2>/dev/null; then
        echo "✓ Started successfully (PID: $PID)"
        echo "Press Ctrl+C to stop"
        
        # Keep the script running
        wait $PID
        exit 0
    else
        echo "Attempt $attempt failed, retrying..."
    fi
done

echo "Could not start after 10 attempts"
exit 1