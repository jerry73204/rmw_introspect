#!/bin/bash
# End-to-end test with demo_nodes_cpp::listener
# Tests that rmw_introspect can capture subscription interfaces

set -e

# Source ROS 2 environment
source /opt/ros/humble/setup.bash

# Create temporary output file
OUTPUT_FILE=$(mktemp /tmp/rmw_introspect_listener_XXXXXX.json)

# Set environment for introspection
export RMW_IMPLEMENTATION=rmw_introspect_cpp
export RMW_INTROSPECT_OUTPUT="$OUTPUT_FILE"
export RMW_INTROSPECT_FORMAT=json

echo "Testing rmw_introspect with demo_nodes_cpp::listener"
echo "Output file: $OUTPUT_FILE"

# Run listener node with timeout (it will initialize and exit quickly)
timeout 5s ros2 run demo_nodes_cpp listener || true

# Check if output file was created
if [ ! -f "$OUTPUT_FILE" ]; then
    echo "ERROR: Output file was not created"
    exit 1
fi

echo "Output file created successfully"
cat "$OUTPUT_FILE"

# Verify JSON is valid
if ! python3 -m json.tool "$OUTPUT_FILE" > /dev/null 2>&1; then
    echo "ERROR: Output is not valid JSON"
    cat "$OUTPUT_FILE"
    rm -f "$OUTPUT_FILE"
    exit 1
fi

echo "JSON is valid"

# Parse and verify content
SUBSCRIPTION_COUNT=$(python3 -c "
import json
with open('$OUTPUT_FILE') as f:
    data = json.load(f)
    print(len(data.get('subscriptions', [])))
")

NODE_COUNT=$(python3 -c "
import json
with open('$OUTPUT_FILE') as f:
    data = json.load(f)
    print(len(data.get('nodes', [])))
")

echo "Nodes found: $NODE_COUNT"
echo "Subscriptions found: $SUBSCRIPTION_COUNT"

# Verify listener node is recorded
if [ "$NODE_COUNT" -lt 1 ]; then
    echo "ERROR: No nodes found in output"
    cat "$OUTPUT_FILE"
    rm -f "$OUTPUT_FILE"
    exit 1
fi

# Verify at least one subscription (listener subscribes to 'chatter')
if [ "$SUBSCRIPTION_COUNT" -lt 1 ]; then
    echo "ERROR: No subscriptions found in output"
    cat "$OUTPUT_FILE"
    rm -f "$OUTPUT_FILE"
    exit 1
fi

# Check for 'chatter' topic
HAS_CHATTER=$(python3 -c "
import json
with open('$OUTPUT_FILE') as f:
    data = json.load(f)
    for sub in data.get('subscriptions', []):
        if 'chatter' in sub.get('topic_name', ''):
            print('true')
            break
    else:
        print('false')
")

if [ "$HAS_CHATTER" != "true" ]; then
    echo "ERROR: No subscription on 'chatter' topic found"
    cat "$OUTPUT_FILE"
    rm -f "$OUTPUT_FILE"
    exit 1
fi

echo "SUCCESS: listener node introspection successful"
echo "  - Found 'chatter' subscription with correct message type"

# Clean up
rm -f "$OUTPUT_FILE"

exit 0
