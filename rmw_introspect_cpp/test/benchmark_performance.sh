#!/bin/bash
# Performance benchmarking for rmw_introspect_cpp
# Measures introspection time for various nodes

set -e

# Source ROS 2 and workspace
source /opt/ros/humble/setup.bash
# Detect workspace root (assume script is in rmw_introspect_cpp/test/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
if [ -f "$WORKSPACE_ROOT/install/setup.bash" ]; then
  source "$WORKSPACE_ROOT/install/setup.bash"
fi

echo "========================================="
echo "rmw_introspect_cpp Performance Benchmark"
echo "========================================="
echo ""

measure_node() {
  local package=$1
  local executable=$2
  local iterations=${3:-3}

  echo "Benchmarking ${package}::${executable} (${iterations} iterations)..."

  local total_time=0
  local output_file="/tmp/rmw_introspect_benchmark_$$.json"

  for i in $(seq 1 $iterations); do
    export RMW_IMPLEMENTATION=rmw_introspect_cpp
    export RMW_INTROSPECT_OUTPUT="$output_file"
    export RMW_INTROSPECT_AUTO_EXPORT=1

    start_time=$(date +%s%N)
    timeout 3 ros2 run "$package" "$executable" >/dev/null 2>&1 || true
    end_time=$(date +%s%N)

    duration_ns=$((end_time - start_time))
    total_time=$((total_time + duration_ns))

    rm -f "$output_file"
  done

  avg_time_ms=$((total_time / iterations / 1000000))
  echo "  Average: ${avg_time_ms}ms"

  # Check against targets
  if [ "$avg_time_ms" -lt 200 ]; then
    echo "  ✅ PASS: < 200ms target"
  elif [ "$avg_time_ms" -lt 500 ]; then
    echo "  ⚠️  WARN: > 200ms but < 500ms"
  else
    echo "  ❌ FAIL: > 500ms"
  fi

  echo ""
}

# Benchmark demo_nodes_cpp nodes
measure_node demo_nodes_cpp talker 3
measure_node demo_nodes_cpp listener 3

# Benchmark Python nodes if available
if ros2 pkg list | grep -q demo_nodes_py; then
  measure_node demo_nodes_py talker 3
  measure_node demo_nodes_py listener 3
fi

echo "========================================="
echo "Benchmark Complete"
echo "========================================="
