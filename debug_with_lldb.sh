#!/bin/bash

echo "Running with LLDB to capture crash stack trace..."

# Kill any existing process
pkill -9 -f mbgl-glfw 2>/dev/null
sleep 0.5

# Create LLDB commands file
cat > /tmp/lldb_commands.txt << 'EOF'
run
bt
quit
EOF

# Run with LLDB
echo "Starting LLDB session..."
lldb -s /tmp/lldb_commands.txt ./build/platform/glfw/mbgl-glfw 2>&1 | tee /tmp/lldb_output.log

# Extract meaningful information
echo ""
echo "=== Crash Analysis ==="
grep -A 20 "stop reason" /tmp/lldb_output.log | head -25

echo ""
echo "=== Top of Stack Trace ==="
grep -A 10 "frame #0" /tmp/lldb_output.log | head -15

echo ""
echo "Full output saved to: /tmp/lldb_output.log"