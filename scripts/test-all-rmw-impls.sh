#!/bin/bash
# Test rmw_introspect with all available RMW implementations
# Runs tests in both recording-only and intermediate modes

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "==================================================================="
echo "RMW Introspect Multi-Implementation Test Runner"
echo "==================================================================="
echo ""

# Source ROS 2
if [ ! -f /opt/ros/humble/setup.bash ]; then
    echo "ERROR: ROS 2 Humble not found at /opt/ros/humble/setup.bash"
    exit 1
fi

source /opt/ros/humble/setup.bash

# Source workspace
if [ -f "$WORKSPACE_DIR/install/setup.bash" ]; then
    source "$WORKSPACE_DIR/install/setup.bash"
    echo "✓ Workspace sourced from $WORKSPACE_DIR/install/setup.bash"
else
    echo "ERROR: Workspace not built. Please run 'make build' first."
    exit 1
fi

echo ""

# Detect available RMW implementations
echo "Detecting available RMW implementations..."
echo ""

declare -a AVAILABLE_RMWS=()

# Check for each RMW implementation library
check_rmw() {
    local rmw_name=$1
    local lib_name="lib${rmw_name}.so"

    # Search in ROS 2 Humble library path
    if [ -f "/opt/ros/humble/lib/$lib_name" ]; then
        AVAILABLE_RMWS+=("$rmw_name")
        echo "  ✓ $rmw_name"
        return 0
    else
        echo "  ✗ $rmw_name (not installed)"
        return 1
    fi
}

check_rmw "rmw_fastrtps_cpp"
check_rmw "rmw_fastrtps_dynamic_cpp"
check_rmw "rmw_cyclonedds_cpp"
check_rmw "rmw_connextdds"
check_rmw "rmw_zenoh_cpp"

echo ""

if [ ${#AVAILABLE_RMWS[@]} -eq 0 ]; then
    echo "ERROR: No RMW implementations found."
    echo "Please run: $SCRIPT_DIR/install-rmw-deps.sh"
    exit 1
fi

echo "Found ${#AVAILABLE_RMWS[@]} RMW implementation(s)"
echo ""

# Test results tracking
declare -A TEST_RESULTS
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Function to run tests with a specific configuration
run_test_config() {
    local mode=$1
    local rmw_impl=$2
    local delegate_to=$3

    TOTAL_TESTS=$((TOTAL_TESTS + 1))

    echo ""
    echo "-------------------------------------------------------------------"
    if [ "$mode" = "recording" ]; then
        echo "Test $TOTAL_TESTS: Recording-only mode"
        echo "  RMW_IMPLEMENTATION: $rmw_impl"
    else
        echo "Test $TOTAL_TESTS: Intermediate mode"
        echo "  RMW_IMPLEMENTATION: $rmw_impl"
        echo "  RMW_INTROSPECT_DELEGATE_TO: $delegate_to"
    fi
    echo "-------------------------------------------------------------------"

    # Set environment
    export RMW_IMPLEMENTATION="$rmw_impl"

    if [ "$mode" = "intermediate" ]; then
        export RMW_INTROSPECT_DELEGATE_TO="$delegate_to"
    else
        unset RMW_INTROSPECT_DELEGATE_TO
    fi

    # Run tests
    cd "$WORKSPACE_DIR"

    if colcon test --packages-select rmw_introspect_cpp 2>&1 | tee /tmp/rmw_test_output.log; then
        echo "✓ Tests PASSED"
        TEST_RESULTS["${mode}_${rmw_impl}_${delegate_to}"]="PASS"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        echo "✗ Tests FAILED"
        TEST_RESULTS["${mode}_${rmw_impl}_${delegate_to}"]="FAIL"
        FAILED_TESTS=$((FAILED_TESTS + 1))

        # Show test results
        echo ""
        echo "Test failures:"
        colcon test-result --verbose 2>/dev/null | grep -A 5 "failures" || true
    fi

    # Clean up for next test
    unset RMW_IMPLEMENTATION
    unset RMW_INTROSPECT_DELEGATE_TO
}

# =================================================================
# Test 1: Recording-only mode
# =================================================================

echo ""
echo "==================================================================="
echo "PHASE 1: Testing Recording-Only Mode"
echo "==================================================================="

# Recording mode should work with rmw_introspect_cpp directly
run_test_config "recording" "rmw_introspect_cpp" "none"

# =================================================================
# Test 2: Intermediate mode with each available RMW
# =================================================================

echo ""
echo "==================================================================="
echo "PHASE 2: Testing Intermediate Mode"
echo "==================================================================="

for rmw_impl in "${AVAILABLE_RMWS[@]}"; do
    # Skip if this is rmw_introspect_cpp itself
    if [ "$rmw_impl" = "rmw_introspect_cpp" ]; then
        continue
    fi

    run_test_config "intermediate" "rmw_introspect_cpp" "$rmw_impl"
done

# =================================================================
# Summary
# =================================================================

echo ""
echo "==================================================================="
echo "TEST SUMMARY"
echo "==================================================================="
echo ""
echo "Total test configurations: $TOTAL_TESTS"
echo "  Passed: $PASSED_TESTS"
echo "  Failed: $FAILED_TESTS"
echo ""

if [ $FAILED_TESTS -eq 0 ]; then
    echo "✓ ALL TESTS PASSED!"
    echo ""
    exit 0
else
    echo "✗ SOME TESTS FAILED"
    echo ""
    echo "Failed configurations:"
    for key in "${!TEST_RESULTS[@]}"; do
        if [ "${TEST_RESULTS[$key]}" = "FAIL" ]; then
            echo "  - $key"
        fi
    done
    echo ""
    exit 1
fi
