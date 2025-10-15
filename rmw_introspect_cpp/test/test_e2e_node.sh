#!/bin/bash
# Helper script to test a single node with rmw_introspect_cpp
# Usage: ./test_e2e_node.sh <package> <executable> [expected_publishers] [expected_subscriptions]

set -e

PACKAGE=$1
EXECUTABLE=$2
EXPECTED_PUBS=${3:-0}
EXPECTED_SUBS=${4:-0}

if [ -z "$PACKAGE" ] || [ -z "$EXECUTABLE" ]; then
  echo "Usage: $0 <package> <executable> [expected_publishers] [expected_subscriptions]"
  exit 1
fi

# Source ROS 2 and workspace
source /opt/ros/humble/setup.bash
# Detect workspace root (assume script is in rmw_introspect_cpp/test/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
if [ -f "$WORKSPACE_ROOT/install/setup.bash" ]; then
  source "$WORKSPACE_ROOT/install/setup.bash"
fi

# Setup test output file
OUTPUT_FILE="/tmp/rmw_introspect_test_${PACKAGE}_${EXECUTABLE}_$$.json"

# Set environment
export RMW_IMPLEMENTATION=rmw_introspect_cpp
export RMW_INTROSPECT_OUTPUT="$OUTPUT_FILE"
export RMW_INTROSPECT_AUTO_EXPORT=1

echo "Testing ${PACKAGE}::${EXECUTABLE}..."

# Run node with timeout (kill after 3 seconds)
timeout 3 ros2 run "$PACKAGE" "$EXECUTABLE" >/dev/null 2>&1 || true

# Check if output file was created
if [ ! -f "$OUTPUT_FILE" ]; then
  echo "❌ FAILED: No output file created"
  exit 1
fi

# Validate JSON
if ! python3 -c "import json; json.load(open('$OUTPUT_FILE'))" 2>/dev/null; then
  echo "❌ FAILED: Invalid JSON output"
  rm -f "$OUTPUT_FILE"
  exit 1
fi

# Count publishers and subscriptions
ACTUAL_PUBS=$(python3 -c "import json; data = json.load(open('$OUTPUT_FILE')); print(len(data.get('publishers', [])))")
ACTUAL_SUBS=$(python3 -c "import json; data = json.load(open('$OUTPUT_FILE')); print(len(data.get('subscriptions', [])))")

echo "  Publishers: $ACTUAL_PUBS (expected: $EXPECTED_PUBS)"
echo "  Subscriptions: $ACTUAL_SUBS (expected: $EXPECTED_SUBS)"

# Verify counts
if [ "$ACTUAL_PUBS" -ne "$EXPECTED_PUBS" ]; then
  echo "❌ FAILED: Publisher count mismatch"
  rm -f "$OUTPUT_FILE"
  exit 1
fi

if [ "$ACTUAL_SUBS" -ne "$EXPECTED_SUBS" ]; then
  echo "❌ FAILED: Subscription count mismatch"
  rm -f "$OUTPUT_FILE"
  exit 1
fi

echo "✅ PASSED: ${PACKAGE}::${EXECUTABLE}"
rm -f "$OUTPUT_FILE"
exit 0
