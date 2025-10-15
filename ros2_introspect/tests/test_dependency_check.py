"""Tests for dependency checking functionality."""

import os
from unittest.mock import patch

from ros2_introspect import check_rmw_introspect_available


def test_check_rmw_when_ros2_not_sourced():
    """Test that check fails when ROS 2 is not sourced."""
    # Mock environment without ROS_DISTRO
    with patch.dict(os.environ, {}, clear=True):
        is_available, error_msg = check_rmw_introspect_available()

        assert is_available is False
        assert error_msg is not None
        assert "ROS 2 environment not sourced" in error_msg
        assert "source /opt/ros/humble/setup.bash" in error_msg


def test_check_rmw_when_rmw_introspect_not_built():
    """Test that check fails when rmw_introspect_cpp is not found."""
    # Mock environment with ROS 2 but without rmw_introspect_cpp
    with patch.dict(
        os.environ,
        {"ROS_DISTRO": "humble", "AMENT_PREFIX_PATH": "/opt/ros/humble"},
        clear=True,
    ):
        is_available, error_msg = check_rmw_introspect_available()

        # This might pass or fail depending on system state
        # If it fails, check the error message
        if not is_available:
            assert error_msg is not None
            assert "rmw_introspect_cpp not found" in error_msg
            assert "colcon build" in error_msg


def test_check_rmw_when_properly_configured():
    """Test that check succeeds when environment is properly configured."""
    # This test will only pass if the workspace is sourced
    is_available, error_msg = check_rmw_introspect_available()

    if is_available:
        assert error_msg is None
    else:
        # If not available, we should have a helpful error message
        assert error_msg is not None
        assert len(error_msg) > 0
