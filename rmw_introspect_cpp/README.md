# rmw_introspect_cpp

A lightweight ROS 2 middleware (RMW) implementation for node interface introspection without actual data transport.

## Overview

**rmw_introspect_cpp** is a recording RMW implementation that intercepts ROS 2 middleware calls to capture node interface metadata (publishers, subscribers, services, clients) without performing actual DDS communication. This enables fast, isolated discovery of node interfaces for tools like launch2plan.

### Key Features

- **Fast**: 3-5x faster than spawning nodes with real middleware (~50-100ms vs ~500ms per node)
- **Isolated**: No network communication, no DDS discovery, no domain conflicts
- **Accurate**: Captures exact message types, topic names, and QoS settings from node code
- **Simple**: Just set `RMW_IMPLEMENTATION=rmw_introspect_cpp` and run the node
- **Complete**: Records all interface types (publishers, subscribers, services, clients)

## Installation

### Build from Source

Build with colcon:

```bash
# Source ROS 2 environment
source /opt/ros/humble/setup.bash

# Build the package
cd /path/to/rmw_introspect
colcon build

# Source the workspace
source install/setup.bash
```

## Usage

### Basic Usage

1. **Set the RMW implementation environment variable**:
   ```bash
   export RMW_IMPLEMENTATION=rmw_introspect_cpp
   ```

2. **Run your node**:
   ```bash
   ros2 run demo_nodes_cpp talker
   ```

3. **Check the output file**:
   ```bash
   cat /tmp/rmw_introspect_*.json
   ```

The node will initialize its interfaces and exit quickly, leaving a JSON file with the captured metadata.

### Configuration

Control rmw_introspect behavior with environment variables:

#### Required
- `RMW_IMPLEMENTATION=rmw_introspect_cpp` - Activates the introspection RMW

#### Optional
- `RMW_INTROSPECT_OUTPUT` - Output file path (default: `/tmp/rmw_introspect_<pid>.json`)
- `RMW_INTROSPECT_FORMAT` - Output format: `json` or `yaml` (default: `json`)
- `RMW_INTROSPECT_VERBOSE` - Enable debug logging: `0` or `1` (default: `0`)
- `RMW_INTROSPECT_AUTO_EXPORT` - Auto-export on shutdown: `0` or `1` (default: `1`)

### Example: Custom Output Location

```bash
export RMW_IMPLEMENTATION=rmw_introspect_cpp
export RMW_INTROSPECT_OUTPUT=/tmp/my_node_interfaces.json
export RMW_INTROSPECT_FORMAT=json

ros2 run my_package my_node

cat /tmp/my_node_interfaces.json
```

## Output Format

### JSON Structure

The output JSON contains complete interface metadata:

```json
{
  "format_version": "1.0",
  "timestamp": "2025-10-13T10:30:45Z",
  "rmw_implementation": "rmw_introspect_cpp",
  "nodes": [
    {
      "name": "talker",
      "namespace": "/"
    }
  ],
  "publishers": [
    {
      "node_name": "talker",
      "node_namespace": "/",
      "topic_name": "chatter",
      "message_type": "std_msgs/msg/String",
      "qos": {
        "reliability": "reliable",
        "durability": "volatile",
        "history": "keep_last",
        "depth": 10
      }
    }
  ],
  "subscriptions": [],
  "services": [],
  "clients": []
}
```

### Fields

- **nodes**: List of node names and namespaces
- **publishers**: Publisher interfaces with topic names, message types, and QoS
- **subscriptions**: Subscription interfaces with the same metadata
- **services**: Service server interfaces with service names and types
- **clients**: Service client interfaces

Each interface entry includes:
- Node identification (name, namespace)
- Topic/service name
- Message/service type (e.g., "std_msgs/msg/String", "std_srvs/srv/SetBool")
- QoS profile (reliability, durability, history, depth)
- Creation timestamp

## Use Cases

### 1. Launch File Conversion (launch2plan)

Automatically discover node interfaces when converting ROS 2 launch files to ROS-Plan format:

```python
# In launch2plan converter
data = introspect_node('image_tools', 'cam2image')

# Generate plan node with sockets
node_yaml = f"""
node:
  {data['nodes'][0]['name']}:
    pkg: image_tools
    exec: cam2image
    socket:
"""

# Add publishers as !pub sockets
for pub in data['publishers']:
    socket_name = pub['topic_name'].split('/')[-1]
    node_yaml += f"""
      {socket_name}: !pub
        type: {pub['message_type']}
"""

# Add subscriptions as !sub sockets
for sub in data['subscriptions']:
    socket_name = sub['topic_name'].split('/')[-1]
    node_yaml += f"""
      {socket_name}: !sub
        type: {sub['message_type']}
"""
```

### 2. Node Documentation

Generate documentation for node interfaces:

```bash
export RMW_IMPLEMENTATION=rmw_introspect_cpp
export RMW_INTROSPECT_OUTPUT=camera_driver_interfaces.json

ros2 run camera_pkg camera_driver

# Use output for documentation generation
python generate_node_docs.py camera_driver_interfaces.json
```

### 3. Interface Validation

Verify that nodes declare expected interfaces:

```python
data = introspect_node('my_package', 'my_node')

# Check expected interfaces
expected_pubs = {'sensor_data', 'diagnostics'}
actual_pubs = {pub['topic_name'] for pub in data['publishers']}

missing = expected_pubs - actual_pubs
if missing:
    print(f"Missing publishers: {missing}")
```

## Testing

The package includes comprehensive unit tests covering all phases:

```bash
# Build and test
source /opt/ros/humble/setup.bash
colcon build --packages-select rmw_introspect_cpp
colcon test --packages-select rmw_introspect_cpp

# View test results
colcon test-result --verbose
```

### Known Limitations

1. **Dynamic Interface Creation**: Only interfaces created during node initialization are captured. Nodes that create interfaces conditionally at runtime won't have those interfaces recorded.

2. **Parameter-Dependent Interfaces**: Nodes that create different interfaces based on parameter values need to be introspected with different parameter combinations.

3. **Side Effects**: Nodes may attempt file I/O, hardware access, or network operations during initialization. rmw_introspect cannot prevent these side effects.


## Contributing

Contributions welcome! See the repository root README for details.

## License

MIT License - See LICENSE file in repository root.
