"""
RMW introspection wrapper for discovering node interfaces.

This module provides a Python wrapper around rmw_introspect_cpp for
discovering ROS 2 node interfaces (publishers, subscriptions, services, clients)
without running actual middleware communication.
"""

import json
import logging
import os
import subprocess
import tempfile
from typing import Any, Dict, List, Optional, Tuple

from .data import (
    ClientInfo,
    IntrospectionResult,
    PublisherInfo,
    QoSProfile,
    ServiceInfo,
    SubscriptionInfo,
)

logger = logging.getLogger(__name__)


def check_rmw_introspect_available() -> Tuple[bool, Optional[str]]:
    """
    Check if rmw_introspect_cpp is available in the environment.

    Returns:
        Tuple of (is_available, error_message)
        If available: (True, None)
        If not available: (False, helpful error message)
    """
    # Check if ROS 2 is sourced
    if "ROS_DISTRO" not in os.environ:
        return (
            False,
            "ROS 2 environment not sourced. Please run:\n"
            "  source /opt/ros/humble/setup.bash  # or your ROS 2 distro",
        )

    # Check if rmw_introspect_cpp is available
    # The RMW should be in CMAKE_PREFIX_PATH or AMENT_PREFIX_PATH
    ament_prefix_path = os.environ.get("AMENT_PREFIX_PATH", "")
    found_rmw = False

    for prefix in ament_prefix_path.split(":"):
        if not prefix:
            continue
        # Check for rmw_introspect_cpp shared library
        rmw_lib_path = os.path.join(prefix, "lib", "librmw_introspect_cpp.so")
        if os.path.exists(rmw_lib_path):
            found_rmw = True
            break

    if not found_rmw:
        return (
            False,
            "rmw_introspect_cpp not found in environment.\n"
            "This RMW implementation must be built and its workspace sourced:\n"
            "  cd /path/to/rmw_introspect\n"
            "  colcon build --packages-select rmw_introspect_cpp\n"
            "  source install/setup.bash",
        )

    return (True, None)


def introspect_node(
    package: str,
    executable: str,
    *,
    parameters: Optional[List[Dict[str, Any]]] = None,
    remappings: Optional[List[Tuple[str, str]]] = None,
    namespace: Optional[str] = None,
    node_name: Optional[str] = None,
    arguments: Optional[List[str]] = None,
    timeout: float = 3.0,
) -> IntrospectionResult:
    """
    Introspect a ROS 2 node to discover its interfaces.

    This function spawns the node with rmw_introspect_cpp as the RMW implementation,
    which captures interface metadata without actual middleware communication.

    IMPORTANT: This function requires the ROS 2 environment to be sourced before calling,
    including the workspace containing rmw_introspect_cpp:
        source /opt/ros/humble/setup.bash
        source /path/to/workspace/install/setup.bash

    Args:
        package: ROS 2 package name
        executable: Executable name within the package
        parameters: List of parameter dictionaries to pass to the node
        remappings: List of (from, to) topic remapping tuples
        namespace: Node namespace (e.g., "/robot1")
        node_name: Override node name
        arguments: Additional ROS arguments
        timeout: Maximum time to wait for node initialization (seconds)

    Returns:
        IntrospectionResult with discovered interfaces or error information

    Example:
        >>> result = introspect_node("demo_nodes_cpp", "talker")
        >>> if result.success:
        ...     print(f"Found {len(result.publishers)} publishers")
        ...     for pub in result.publishers:
        ...         print(f"  {pub.topic_name}: {pub.message_type}")
    """
    # Check if rmw_introspect_cpp is available
    is_available, error_msg = check_rmw_introspect_available()
    if not is_available:
        return IntrospectionResult(success=False, error=error_msg)

    # Create temporary file path for JSON output (don't open it yet)
    import uuid

    output_path = f"/tmp/rmw_introspect_{uuid.uuid4().hex[:8]}.json"

    try:
        # Build command
        cmd = _build_ros2_command(
            package,
            executable,
            parameters=parameters,
            remappings=remappings,
            namespace=namespace,
            node_name=node_name,
            arguments=arguments,
        )

        # Use current environment (which should have ROS 2 sourced)
        env = os.environ.copy()

        # Set introspection environment variables
        env["RMW_IMPLEMENTATION"] = "rmw_introspect_cpp"
        env["RMW_INTROSPECT_OUTPUT"] = output_path
        env["RMW_INTROSPECT_AUTO_EXPORT"] = "1"

        logger.debug(f"Running command: {' '.join(cmd)}")
        logger.debug(f"Output path: {output_path}")

        # Run node with timeout using Popen for better control
        import signal
        import time

        proc = subprocess.Popen(
            cmd,
            env=env,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            start_new_session=True,  # Start in new process group
        )

        try:
            # Wait for timeout
            time.sleep(timeout)

            # Node should still be running, send SIGTERM to entire process group
            # This ensures both ros2 run wrapper and the actual node receive the signal
            logger.debug(f"Sending SIGTERM to process group after {timeout}s")
            os.killpg(os.getpgid(proc.pid), signal.SIGTERM)

            # Wait for process to complete shutdown and export data (give it up to 5 seconds)
            # ROS 2 shutdown sequence includes: signal handler → rclcpp shutdown → rmw_shutdown → export
            try:
                returncode = proc.wait(timeout=5.0)
                logger.debug(f"Process exited with code {returncode}")
            except subprocess.TimeoutExpired:
                # If it doesn't exit cleanly, kill it
                logger.debug("Process didn't exit after SIGTERM, sending SIGKILL")
                proc.kill()
                proc.wait()

        except Exception as e:
            # Clean up process if something goes wrong
            try:
                proc.kill()
                proc.wait()
            except Exception:
                pass
            return IntrospectionResult(
                success=False, error=f"Process management failed: {e}"
            )

        # Read JSON output
        if not os.path.exists(output_path):
            return IntrospectionResult(
                success=False,
                error=(
                    f"Introspection output file not created: {output_path}\n"
                    "Please ensure ROS 2 environment is sourced before running:\n"
                    "  source /opt/ros/humble/setup.bash\n"
                    "  source /path/to/workspace/install/setup.bash  # workspace with rmw_introspect_cpp"
                ),
            )

        with open(output_path, "r") as f:
            data = json.load(f)

        # Parse introspection data
        return _parse_introspection_data(data)

    except json.JSONDecodeError as e:
        return IntrospectionResult(
            success=False, error=f"Failed to parse JSON output: {e}"
        )
    except Exception as e:
        return IntrospectionResult(success=False, error=f"Introspection failed: {e}")
    finally:
        # Cleanup temporary file
        if os.path.exists(output_path):
            try:
                os.unlink(output_path)
            except OSError:
                pass


def _build_ros2_command(
    package: str,
    executable: str,
    *,
    parameters: Optional[List[Dict[str, Any]]] = None,
    remappings: Optional[List[Tuple[str, str]]] = None,
    namespace: Optional[str] = None,
    node_name: Optional[str] = None,
    arguments: Optional[List[str]] = None,
) -> List[str]:
    """Build ros2 run command with arguments."""
    cmd = ["ros2", "run", package, executable]

    # Add ROS arguments
    ros_args = []

    if node_name:
        ros_args.extend(["-r", f"__node:={node_name}"])

    if namespace:
        ros_args.extend(["-r", f"__ns:={namespace}"])

    if remappings:
        for from_topic, to_topic in remappings:
            ros_args.extend(["-r", f"{from_topic}:={to_topic}"])

    if parameters:
        # Create temporary parameter file
        with tempfile.NamedTemporaryFile(
            mode="w", suffix=".yaml", delete=False
        ) as param_file:
            import yaml

            yaml.dump({"/**": {"ros__parameters": parameters[0]}}, param_file)
            ros_args.extend(["--params-file", param_file.name])

    if arguments:
        ros_args.extend(arguments)

    if ros_args:
        cmd.append("--ros-args")
        cmd.extend(ros_args)

    return cmd


def _parse_introspection_data(data: Dict[str, Any]) -> IntrospectionResult:
    """Parse JSON introspection data into structured result."""
    result = IntrospectionResult(success=True)

    result.format_version = data.get("format_version")
    result.timestamp = data.get("timestamp")
    result.nodes = data.get("nodes", [])

    # Parse publishers
    for pub_data in data.get("publishers", []):
        qos = QoSProfile(
            reliability=pub_data["qos"]["reliability"],
            durability=pub_data["qos"]["durability"],
            history=pub_data["qos"]["history"],
            depth=pub_data["qos"]["depth"],
        )
        pub = PublisherInfo(
            topic_name=pub_data["topic_name"],
            message_type=pub_data["message_type"],
            qos=qos,
            node_name=pub_data["node_name"],
            node_namespace=pub_data["node_namespace"],
        )
        result.publishers.append(pub)

    # Parse subscriptions
    for sub_data in data.get("subscriptions", []):
        qos = QoSProfile(
            reliability=sub_data["qos"]["reliability"],
            durability=sub_data["qos"]["durability"],
            history=sub_data["qos"]["history"],
            depth=sub_data["qos"]["depth"],
        )
        sub = SubscriptionInfo(
            topic_name=sub_data["topic_name"],
            message_type=sub_data["message_type"],
            qos=qos,
            node_name=sub_data["node_name"],
            node_namespace=sub_data["node_namespace"],
        )
        result.subscriptions.append(sub)

    # Parse services
    for srv_data in data.get("services", []):
        srv = ServiceInfo(
            service_name=srv_data["service_name"],
            service_type=srv_data["service_type"],
            node_name=srv_data["node_name"],
            node_namespace=srv_data["node_namespace"],
        )
        result.services.append(srv)

    # Parse clients
    for client_data in data.get("clients", []):
        client = ClientInfo(
            service_name=client_data["service_name"],
            service_type=client_data["service_type"],
            node_name=client_data["node_name"],
            node_namespace=client_data["node_namespace"],
        )
        result.clients.append(client)

    return result
