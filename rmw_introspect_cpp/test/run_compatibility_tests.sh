#!/bin/bash
# Compatibility testing script for rmw_introspect
# Tests the intermediate layer with multiple RMW backends

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test results
RESULTS_FILE="compatibility_test_results.txt"
echo "RMW Introspect Compatibility Test Results" > "$RESULTS_FILE"
echo "=========================================" >> "$RESULTS_FILE"
echo "Date: $(date)" >> "$RESULTS_FILE"
echo "" >> "$RESULTS_FILE"

# RMW implementations to test
RMW_IMPLEMENTATIONS=(
    "rmw_fastrtps_cpp"
    "rmw_cyclonedds_cpp"
    "rmw_connextdds"
)

# Function to test with a specific RMW backend
test_rmw_backend() {
    local rmw_impl=$1
    echo -e "${YELLOW}Testing with $rmw_impl...${NC}"
    echo "Testing with $rmw_impl" >> "$RESULTS_FILE"
    echo "-----------------------------------" >> "$RESULTS_FILE"

    # Check if RMW implementation is available
    if ! ros2 pkg list | grep -q "^$rmw_impl\$"; then
        echo -e "${RED}$rmw_impl not found, skipping...${NC}"
        echo "Status: NOT AVAILABLE" >> "$RESULTS_FILE"
        echo "" >> "$RESULTS_FILE"
        return 1
    fi

    # Set environment variable for intermediate mode
    export RMW_INTROSPECT_DELEGATE_TO="$rmw_impl"

    # Run colcon test with this RMW implementation
    echo "Running tests with RMW_INTROSPECT_DELEGATE_TO=$rmw_impl"

    if colcon test --packages-select rmw_introspect_cpp --event-handlers console_direct+ 2>&1 | tee -a "$RESULTS_FILE"; then
        echo -e "${GREEN}✓ Tests passed with $rmw_impl${NC}"
        echo "Status: PASSED" >> "$RESULTS_FILE"
    else
        echo -e "${RED}✗ Tests failed with $rmw_impl${NC}"
        echo "Status: FAILED" >> "$RESULTS_FILE"
    fi

    echo "" >> "$RESULTS_FILE"
    unset RMW_INTROSPECT_DELEGATE_TO
}

# Function to test recording-only mode
test_recording_mode() {
    echo -e "${YELLOW}Testing recording-only mode (no delegation)...${NC}"
    echo "Testing recording-only mode" >> "$RESULTS_FILE"
    echo "-----------------------------------" >> "$RESULTS_FILE"

    # Ensure RMW_INTROSPECT_DELEGATE_TO is not set
    unset RMW_INTROSPECT_DELEGATE_TO

    # Run subset of tests that work in recording-only mode
    if colcon test --packages-select rmw_introspect_cpp --ctest-args -R "test_init|test_data|test_phase" --event-handlers console_direct+ 2>&1 | tee -a "$RESULTS_FILE"; then
        echo -e "${GREEN}✓ Recording-only mode tests passed${NC}"
        echo "Status: PASSED" >> "$RESULTS_FILE"
    else
        echo -e "${RED}✗ Recording-only mode tests failed${NC}"
        echo "Status: FAILED" >> "$RESULTS_FILE"
    fi

    echo "" >> "$RESULTS_FILE"
}

# Main test execution
echo "================================================"
echo "RMW Introspect Compatibility Testing"
echo "================================================"
echo ""

# Test recording-only mode first
test_recording_mode

# Test each RMW implementation
for rmw_impl in "${RMW_IMPLEMENTATIONS[@]}"; do
    test_rmw_backend "$rmw_impl"
done

# Summary
echo ""
echo "================================================"
echo "Test Summary"
echo "================================================"
echo ""
echo "Full results saved to: $RESULTS_FILE"
echo ""

# Display summary from results file
if grep -q "FAILED" "$RESULTS_FILE"; then
    echo -e "${RED}Some tests failed. Please review the results.${NC}"
    exit 1
else
    echo -e "${GREEN}All available tests passed!${NC}"
fi
