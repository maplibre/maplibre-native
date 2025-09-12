#!/bin/bash

# Script to debug the intermittent crash issue
echo "Debugging GLFW WebGPU crash issue..."

# Clean up
pkill -9 -f mbgl-glfw 2>/dev/null
rm -f /tmp/mbgl-glfw-*.log /tmp/crash-*.log

# Try to start multiple times and capture crash info
for i in {1..5}; do
    echo ""
    echo "========================================="
    echo "Attempt $i - Capturing crash information"
    echo "========================================="
    
    # Start with crash reporting
    LOGFILE="/tmp/mbgl-glfw-$i.log"
    CRASHLOG="/tmp/crash-$i.log"
    
    # Run with debugging enabled
    RUST_BACKTRACE=1 \
    LIBGL_DEBUG=verbose \
    MESA_DEBUG=1 \
    ./build/platform/glfw/mbgl-glfw > $LOGFILE 2>&1 &
    
    PID=$!
    echo "Started PID: $PID"
    
    # Monitor for 3 seconds
    RUNNING=1
    for j in {1..30}; do
        if ! kill -0 $PID 2>/dev/null; then
            RUNNING=0
            wait $PID
            EXIT_CODE=$?
            echo "Process crashed after ~$((j/10)).${j: -1} seconds with exit code: $EXIT_CODE"
            
            # Capture crash info
            echo "Exit code: $EXIT_CODE" > $CRASHLOG
            echo "Duration: ~$((j/10)).${j: -1} seconds" >> $CRASHLOG
            
            # Get last lines of log
            echo -e "\nLast 20 lines of output:" >> $CRASHLOG
            tail -20 $LOGFILE >> $CRASHLOG
            
            # Look for specific error patterns
            echo -e "\nError patterns found:" >> $CRASHLOG
            grep -i "error\|fault\|abort\|crash\|assert\|failed" $LOGFILE | tail -10 >> $CRASHLOG
            
            # Check for Metal/GPU errors
            echo -e "\nMetal/GPU related:" >> $CRASHLOG
            grep -i "metal\|gpu\|dawn\|webgpu\|wgpu" $LOGFILE | grep -i "error\|fail" | tail -10 >> $CRASHLOG
            
            break
        fi
        sleep 0.1
    done
    
    if [ $RUNNING -eq 1 ]; then
        echo "✓ Process is still running after 3 seconds - SUCCESS!"
        echo "Keeping it running for 2 more seconds to verify stability..."
        sleep 2
        
        if kill -0 $PID 2>/dev/null; then
            echo "✓ Process stable - stopping it"
            kill $PID 2>/dev/null
            wait $PID 2>/dev/null
            break
        else
            echo "✗ Process crashed during stability check"
        fi
    fi
    
    # Clean up before next attempt
    pkill -9 -f mbgl-glfw 2>/dev/null
    sleep 0.5
done

echo ""
echo "========================================="
echo "Crash Analysis Summary"
echo "========================================="

# Analyze patterns across crashes
echo "Checking for consistent crash patterns..."

# Count crash occurrences
CRASHES=$(ls /tmp/crash-*.log 2>/dev/null | wc -l)
echo "Total crashes captured: $CRASHES"

if [ $CRASHES -gt 0 ]; then
    echo ""
    echo "Exit codes:"
    grep "Exit code:" /tmp/crash-*.log 2>/dev/null | sort | uniq -c
    
    echo ""
    echo "Common error messages:"
    grep -h -i "error\|fault\|abort\|crash\|assert\|failed" /tmp/crash-*.log 2>/dev/null | \
        grep -v "Exit code\|Duration\|Last.*lines\|Error patterns" | \
        sort | uniq -c | sort -rn | head -5
    
    echo ""
    echo "Timing of crashes:"
    grep "Duration:" /tmp/crash-*.log 2>/dev/null
    
    echo ""
    echo "Full crash logs saved to: /tmp/crash-*.log"
    echo "Full output logs saved to: /tmp/mbgl-glfw-*.log"
fi

# Check system logs for Metal/GPU issues
echo ""
echo "Checking system logs for Metal/GPU issues (last 30 seconds)..."
log show --last 30s --predicate 'subsystem CONTAINS "Metal" OR message CONTAINS "GPU"' 2>/dev/null | \
    grep -i "error\|fault\|crash" | tail -5

echo ""
echo "Debug complete. Check /tmp/crash-*.log for details."