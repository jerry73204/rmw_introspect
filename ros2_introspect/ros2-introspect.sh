#!/bin/bash
# Helper script to run ros2-introspect with proper ROS 2 environment

# Check if ROS 2 is sourced
if [ -z "$ROS_DISTRO" ]; then
    echo "Error: ROS 2 environment not sourced"
    echo "Please run:"
    echo "  source /opt/ros/humble/setup.bash"
    echo "  source /path/to/workspace/install/setup.bash"
    exit 1
fi

# Check if rmw_introspect_cpp is available
if ! ros2 pkg list | grep -q rmw_introspect_cpp; then
    echo "Error: rmw_introspect_cpp package not found"
    echo "Please source the workspace containing rmw_introspect_cpp:"
    echo "  source /path/to/workspace/install/setup.bash"
    exit 1
fi

# Run ros2-introspect
uv run ros2-introspect "$@"
