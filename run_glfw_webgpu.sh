#!/bin/bash

# Script to run the GLFW WebGPU example with retries
# Handles the intermittent startup crashes

MAX_ATTEMPTS=5
ATTEMPT=1
SUCCESS=0

echo "Running GLFW WebGPU example with retry logic..."

while [ $ATTEMPT -le $MAX_ATTEMPTS ] && [ $SUCCESS -eq 0 ]; do
    echo "Attempt $ATTEMPT of $MAX_ATTEMPTS..."
    
    # Kill any existing process
    pkill -f mbgl-glfw 2>/dev/null || true
    sleep 0.5
    
    # Run the application with a timeout
    timeout 3 ./build/platform/glfw/mbgl-glfw 2>&1 | head -100
    EXIT_CODE=$?
    
    # Check if it ran successfully (timeout exit code is 124)
    if [ $EXIT_CODE -eq 124 ]; then
        echo "App appears to be running successfully (timed out after 3 seconds)"
        SUCCESS=1
    elif [ $EXIT_CODE -eq 0 ]; then
        echo "App exited normally"
        SUCCESS=1
    else
        echo "App crashed with exit code $EXIT_CODE"
        if [ $ATTEMPT -lt $MAX_ATTEMPTS ]; then
            echo "Retrying in 1 second..."
            sleep 1
        fi
    fi
    
    ATTEMPT=$((ATTEMPT + 1))
done

if [ $SUCCESS -eq 1 ]; then
    echo "✓ Successfully started the WebGPU GLFW example"
    echo "The app is now running. Press Ctrl+C to stop it."
    
    # Run the app normally without timeout
    ./build/platform/glfw/mbgl-glfw
else
    echo "✗ Failed to start the app after $MAX_ATTEMPTS attempts"
    echo "This might indicate a deeper issue with the WebGPU initialization"
    exit 1
fi