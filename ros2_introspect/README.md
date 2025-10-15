# ros2-introspect

ROS 2 node introspection tool using `rmw_introspect_cpp`.

This package provides a Python wrapper around the `rmw_introspect_cpp` RMW implementation to discover ROS 2 node interfaces (publishers, subscriptions, services, clients) without running actual middleware communication.

## Features

- **Fast introspection**: Discover node interfaces without full middleware initialization
- **Complete interface metadata**: Publishers, subscriptions, services, clients with QoS profiles
- **ROS 2 argument support**: Namespaces, remappings, parameters, custom node names
- **Flexible output**: Text and JSON formats
- **Library and CLI**: Use as a Python library or standalone command-line tool

## Requirements

- ROS 2 Humble (or compatible distribution)
- Python 3.10
- `rmw_introspect_cpp` built and installed in your ROS 2 workspace

## Installation

Install using UV:

```bash
cd ros2_introspect
uv sync
```

## Command-Line Usage

**IMPORTANT**: Before running the CLI, source your ROS 2 environment and workspace containing rmw_introspect_cpp:

```bash
source /opt/ros/humble/setup.bash
source /path/to/workspace/install/setup.bash
```

Basic usage:

```bash
ros2-introspect <package> <executable>
```

Examples:

```bash
# Introspect demo talker node
ros2-introspect demo_nodes_cpp talker

# With custom namespace
ros2-introspect demo_nodes_cpp talker --namespace /robot1

# With topic remapping
ros2-introspect demo_nodes_cpp talker --remap /chatter:=/my_topic

# With parameters
ros2-introspect my_package my_node --param rate:=10.0 --param debug:=true

# JSON output
ros2-introspect demo_nodes_cpp talker --format json
```

### CLI Options

- `package`: ROS 2 package name (required)
- `executable`: Executable name within the package (required)
- `--namespace`, `-n`: Node namespace (e.g., `/robot1`)
- `--node-name`: Override node name
- `--timeout`: Maximum time to wait for node initialization (default: 3.0 seconds)
- `--remap`, `-r`: Topic remapping in format `from:=to` (can be specified multiple times)
- `--param`, `-p`: Parameter in format `name:=value` (can be specified multiple times)
- `--format`, `-f`: Output format: `text` or `json` (default: `text`)

## Library Usage

**IMPORTANT**: The ROS 2 environment must be sourced before importing and using this library:

```bash
source /opt/ros/humble/setup.bash
source /path/to/workspace/install/setup.bash
```

Import and use in your Python code:

```python
from ros2_introspect import introspect_node

# Basic introspection
result = introspect_node("demo_nodes_cpp", "talker")

if result.success:
    print(f"Nodes: {result.nodes}")

    for pub in result.publishers:
        print(f"Publisher: {pub.topic_name}")
        print(f"  Type: {pub.message_type}")
        print(f"  QoS: {pub.qos.reliability}, {pub.qos.durability}")

    for sub in result.subscriptions:
        print(f"Subscription: {sub.topic_name}")
        print(f"  Type: {sub.message_type}")
else:
    print(f"Error: {result.error}")
```

### Advanced Usage

```python
# With full configuration
result = introspect_node(
    "my_package",
    "my_node",
    parameters=[{"rate": 10.0, "debug": True}],
    remappings=[("/input", "/camera/image")],
    namespace="/robot1",
    node_name="custom_name",
    timeout=5.0,
)

# Access specific interfaces
for service in result.services:
    print(f"Service: {service.service_name} ({service.service_type})")

for client in result.clients:
    print(f"Client: {client.service_name} ({client.service_type})")
```

### Data Structures

The `IntrospectionResult` contains:

- `success`: Boolean indicating if introspection succeeded
- `error`: Error message if failed (optional)
- `nodes`: List of node names discovered
- `publishers`: List of `PublisherInfo` objects
- `subscriptions`: List of `SubscriptionInfo` objects
- `services`: List of `ServiceInfo` objects
- `clients`: List of `ClientInfo` objects
- `format_version`: Introspection format version
- `timestamp`: Introspection timestamp

Each interface info object contains:

- Topic/service name
- Message/service type
- Node name and namespace
- QoS profile (for publishers/subscriptions)

## How It Works

1. Spawns the ROS 2 node with `RMW_IMPLEMENTATION=rmw_introspect_cpp`
2. The custom RMW implementation records interface creation without middleware communication
3. Exports interface metadata to a JSON file on node termination
4. Parses the JSON output into structured Python objects

This approach is much faster than runtime introspection via the ROS 2 graph API, since it doesn't require full middleware initialization or node communication.

## Testing

Run tests using pytest:

```bash
cd ros2_introspect
source /opt/ros/humble/setup.bash  # Source ROS 2 environment
uv run pytest
```

Tests require:
- ROS 2 environment sourced
- `demo_nodes_cpp` package available
- `rmw_introspect_cpp` built and installed

## Integration with launch2plan

This package is designed to be used by `launch2plan` for the launch-to-plan conversion workflow. The introspection capability allows launch2plan to discover actual node interfaces for more accurate plan generation.

## License

MIT License - See LICENSE file in repository root.
