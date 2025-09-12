#!/bin/bash

# Script to reliably start the GLFW WebGPU example
# Handles intermittent startup crashes and verifies the process is actually running

MAX_ATTEMPTS=10
ATTEMPT=1
SUCCESS=0
PROCESS_CHECK_DELAY=2

echo "Starting GLFW WebGPU example with reliability checks..."

# Function to check if the process is truly running
check_process_running() {
    sleep $PROCESS_CHECK_DELAY
    if pgrep -f "mbgl-glfw" > /dev/null; then
        # Check if process is still alive after another second
        sleep 1
        if pgrep -f "mbgl-glfw" > /dev/null; then
            return 0
        fi
    fi
    return 1
}

# Function to clean up any existing processes
cleanup() {
    echo "Cleaning up any existing processes..."
    pkill -9 -f mbgl-glfw 2>/dev/null || true
    sleep 0.5
}

# Initial cleanup
cleanup

while [ $ATTEMPT -le $MAX_ATTEMPTS ] && [ $SUCCESS -eq 0 ]; do
    echo ""
    echo "========================================="
    echo "Attempt $ATTEMPT of $MAX_ATTEMPTS..."
    echo "========================================="
    
    # Start the application in background
    echo "Starting mbgl-glfw in background..."
    ./build/platform/glfw/mbgl-glfw > /tmp/mbgl-glfw.log 2>&1 &
    GLFW_PID=$!
    
    echo "Started with PID: $GLFW_PID"
    
    # Check if process is actually running after a delay
    if check_process_running; then
        echo "✓ Process is running and stable!"
        SUCCESS=1
        
        # Get the actual PID (in case it changed)
        ACTUAL_PID=$(pgrep -f "mbgl-glfw" | head -1)
        echo "✓ WebGPU GLFW app is running with PID: $ACTUAL_PID"
        echo ""
        echo "Monitor the process with: tail -f /tmp/mbgl-glfw.log"
        echo "Stop the process with: kill $ACTUAL_PID"
        echo ""
        
        # Keep the script running and monitor the process
        echo "Press Ctrl+C to stop the application..."
        
        # Monitor the process
        while kill -0 $ACTUAL_PID 2>/dev/null; do
            sleep 1
        done
        
        echo "Process has stopped."
    else
        echo "✗ Process failed to start or crashed immediately"
        
        # Show last few lines of log for debugging
        if [ -f /tmp/mbgl-glfw.log ]; then
            echo "Last log entries:"
            tail -5 /tmp/mbgl-glfw.log
        fi
        
        # Kill any remaining process
        kill -9 $GLFW_PID 2>/dev/null || true
        cleanup
        
        if [ $ATTEMPT -lt $MAX_ATTEMPTS ]; then
            echo "Waiting before retry..."
            sleep 1
        fi
    fi
    
    ATTEMPT=$((ATTEMPT + 1))
done

if [ $SUCCESS -eq 0 ]; then
    echo ""
    echo "✗ Failed to start the app after $MAX_ATTEMPTS attempts"
    echo "Check the full log at: /tmp/mbgl-glfw.log"
    echo ""
    echo "Possible issues:"
    echo "  - WebGPU/Dawn initialization race condition"
    echo "  - Metal backend resource contention"
    echo "  - Previous instance not fully cleaned up"
    echo ""
    echo "Try:"
    echo "  1. Reboot or reset Metal/GPU state"
    echo "  2. Build with: cmake --build build --target mbgl-glfw --clean-first"
    echo "  3. Check system logs: log show --last 1m | grep -i metal"
    exit 1
fi