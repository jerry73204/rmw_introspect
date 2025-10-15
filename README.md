# RMW Introspect

Fast, lightweight ROS 2 node interface introspection without middleware communication.

## Overview

RMW Introspect provides tools to discover ROS 2 node interfaces (publishers, subscriptions, services, clients) quickly and accurately without full middleware initialization. This is 3-5x faster than spawning nodes with standard RMW implementations (~50-100ms vs ~500ms per node).

This repository contains two complementary components:

1. **rmw_introspect_cpp** - A custom ROS 2 RMW implementation that records interface metadata
2. **ros2_introspect** - A Python CLI and library for easy introspection

## Key Features

- **Fast**: 3-5x faster than real middleware initialization
- **Isolated**: No network communication, no DDS discovery, no domain conflicts
- **Accurate**: Captures exact message types, topic names, and QoS settings from node code
- **Simple**: Just set `RMW_IMPLEMENTATION=rmw_introspect_cpp` and run the node
- **Complete**: Records all interface types (publishers, subscriptions, services, clients)
- **Python Integration**: CLI tool and library for easy use in Python workflows

## Use Cases

- **Launch-to-Plan Conversion**: Automatically discover node interfaces when converting ROS 2 launch files to ROS-Plan format
- **Node Documentation**: Generate interface documentation for nodes
- **Interface Validation**: Verify that nodes declare expected interfaces
- **Development Tools**: Build tools that need node interface information without runtime overhead

## Repository Structure

```
rmw_introspect/
├── README.md                    # This file
├── LICENSE                      # MIT license
├── Makefile                     # Build and test automation
├── rmw_introspect_cpp/          # C++ RMW implementation (ROS 2 package)
│   ├── CMakeLists.txt
│   ├── package.xml
│   ├── README.md                # C++ component documentation
│   ├── include/
│   ├── src/
│   └── test/
└── ros2_introspect/             # Python wrapper (library + CLI)
    ├── pyproject.toml
    ├── README.md                # Python component documentation
    ├── src/
    └── tests/
```

## Quick Start

### Installation

**Using the Makefile (recommended)**:

```bash
# Source ROS 2 environment
source /opt/ros/humble/setup.bash

# Build and install all components
cd rmw_introspect
make install

# Source the workspace
source install/setup.bash
```

**Manual installation**:

```bash
# 1. Build C++ RMW implementation
source /opt/ros/humble/setup.bash
cd rmw_introspect
colcon build
source install/setup.bash

# 2. Install Python package (optional, for CLI/library usage)
cd ros2_introspect
uv sync
```

### Basic Usage

**Using the RMW implementation directly**:

```bash
# Set RMW implementation
export RMW_IMPLEMENTATION=rmw_introspect_cpp

# Run a node
ros2 run demo_nodes_cpp talker

# Check output
cat /tmp/rmw_introspect_*.json
```

**Using the Python CLI**:

```bash
# Source workspace
source install/setup.bash

# Introspect a node
cd ros2_introspect
uv run ros2-introspect demo_nodes_cpp talker

# With options
uv run ros2-introspect demo_nodes_cpp talker --namespace /robot1 --format json
```

**Using as a Python library**:

```python
from ros2_introspect import introspect_node

result = introspect_node("demo_nodes_cpp", "talker")

if result.success:
    for pub in result.publishers:
        print(f"Publisher: {pub.topic_name} ({pub.message_type})")
```

## Output Format

The introspection produces JSON metadata containing:

```json
{
  "format_version": "1.0",
  "timestamp": "2025-10-15T10:30:45Z",
  "rmw_implementation": "rmw_introspect_cpp",
  "nodes": [{"name": "talker", "namespace": "/"}],
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

## Components

### rmw_introspect_cpp

C++ RMW implementation that intercepts ROS 2 middleware calls to capture interface metadata. See [rmw_introspect_cpp/README.md](rmw_introspect_cpp/README.md) for details.

**Key features**:
- Complete RMW API implementation
- JSON/YAML output formats
- Configurable via environment variables
- Comprehensive test coverage

### ros2_introspect

Python wrapper providing a convenient CLI and library interface. See [ros2_introspect/README.md](ros2_introspect/README.md) for details.

**Key features**:
- Simple `introspect_node()` function
- CLI with ROS 2 argument support (namespaces, remappings, parameters)
- Structured result objects with type information
- Text and JSON output formats

## Configuration

Control behavior with environment variables:

#### Required
- `RMW_IMPLEMENTATION=rmw_introspect_cpp` - Activates the introspection RMW

#### Optional
- `RMW_INTROSPECT_OUTPUT` - Output file path (default: `/tmp/rmw_introspect_<pid>.json`)
- `RMW_INTROSPECT_FORMAT` - Output format: `json` or `yaml` (default: `json`)
- `RMW_INTROSPECT_VERBOSE` - Enable debug logging: `0` or `1` (default: `0`)
- `RMW_INTROSPECT_AUTO_EXPORT` - Auto-export on shutdown: `0` or `1` (default: `1`)

## Development

### Building and Testing

**Using the Makefile** (see `make help` for all targets):

```bash
# Build all components
make build

# Run all tests
make test

# Run only C++ tests
make test-cpp

# Run only Python tests
make test-python

# Format code
make format

# Run linters
make lint

# Clean build artifacts
make clean
```

**Manual commands**:

```bash
# Test C++ component
source /opt/ros/humble/setup.bash
colcon build --packages-select rmw_introspect_cpp
colcon test --packages-select rmw_introspect_cpp
colcon test-result --verbose

# Test Python component
source /opt/ros/humble/setup.bash
source install/setup.bash
cd ros2_introspect
uv run pytest
```

### Additional Tools

```bash
# Run performance benchmarks
make benchmark

# Run end-to-end integration tests
make e2e-test
```

## Known Limitations

1. **Dynamic Interface Creation**: Only interfaces created during node initialization are captured. Runtime-conditional interfaces won't be recorded.

2. **Parameter-Dependent Interfaces**: Nodes that create different interfaces based on parameters need introspection with different parameter configurations.

3. **Side Effects**: Nodes may attempt file I/O, hardware access, or network operations during initialization. rmw_introspect cannot prevent these side effects.

## Integration with ROS-Plan

This tool was developed as part of the [ROS-Plan](https://github.com/jerry73204/ros-plan) project for the launch-to-plan conversion workflow (launch2plan). It enables automatic discovery of node interfaces when converting ROS 2 launch files to declarative ROS-Plan format.

## Contributing

Contributions welcome! Please follow standard ROS 2 development practices.

## License

MIT License

Copyright (c) 2025 ROS-Plan Contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Maintainer

- aeon <jerry73204@gmail.com>

## Version

0.1.0 (Initial release)
