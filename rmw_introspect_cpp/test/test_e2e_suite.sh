#!/bin/bash
# Master E2E test suite for rmw_introspect_cpp
# Tests various ROS 2 nodes and validates introspection output

set -e

# Change to script directory
cd "$(dirname "$0")"

# Make helper scripts executable
chmod +x test_e2e_node.sh

echo "========================================="
echo "rmw_introspect_cpp E2E Test Suite"
echo "========================================="
echo ""

PASSED=0
FAILED=0
FAILED_TESTS=()

run_test() {
  local package=$1
  local executable=$2
  local expected_pubs=$3
  local expected_subs=$4

  if ./test_e2e_node.sh "$package" "$executable" "$expected_pubs" "$expected_subs"; then
    ((PASSED++))
  else
    ((FAILED++))
    FAILED_TESTS+=("${package}::${executable}")
  fi
  echo ""
}

# Test demo_nodes_cpp nodes
echo "Testing demo_nodes_cpp nodes..."
echo "-----------------------------------"
# Talker has: /chatter publisher + /rosout + /parameter_events publishers, /parameter_events subscription
run_test demo_nodes_cpp talker 3 1
# Listener has: /rosout + /parameter_events publishers, /chatter + /parameter_events subscriptions
run_test demo_nodes_cpp listener 2 2

# Test demo_nodes_py nodes (if available)
if ros2 pkg list | grep -q demo_nodes_py; then
  echo "Testing demo_nodes_py nodes..."
  echo "-----------------------------------"
  # Same expectations as C++ nodes
  run_test demo_nodes_py talker 3 1
  run_test demo_nodes_py listener 2 2
else
  echo "⚠️  Skipping demo_nodes_py tests (package not found)"
  echo ""
fi

# Summary
echo "========================================="
echo "Test Summary"
echo "========================================="
echo "Passed: $PASSED"
echo "Failed: $FAILED"

if [ $FAILED -gt 0 ]; then
  echo ""
  echo "Failed tests:"
  for test in "${FAILED_TESTS[@]}"; do
    echo "  - $test"
  done
  exit 1
else
  echo ""
  echo "✅ All tests passed!"
  exit 0
fi
