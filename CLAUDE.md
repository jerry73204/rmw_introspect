# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

RMW Introspect provides fast, lightweight ROS 2 node interface introspection without middleware communication. It consists of two complementary components:

1. **rmw_introspect_cpp** - A custom ROS 2 RMW (ROS Middleware) implementation that intercepts middleware calls to record interface metadata
2. **ros2_introspect** - A Python CLI and library that wraps the RMW implementation for convenient introspection

This is 3-5x faster than spawning nodes with standard RMW implementations (~50-100ms vs ~500ms per node) because it captures interface metadata without network communication, DDS discovery, or domain conflicts.

## Build and Test Commands

### Prerequisites
Always source the ROS 2 environment before building or testing:
```bash
source /opt/ros/humble/setup.bash
```

### Building

**Recommended (using Makefile):**
```bash
make build              # Build all components
make build-cpp          # Build only C++ RMW
make build-python       # Install Python dependencies only
make install            # Full install (build + install)
```

**Manual build:**
```bash
colcon build --packages-select rmw_introspect_cpp
source install/setup.bash
cd ros2_introspect && uv sync
```

### Testing

**Run all tests:**
```bash
make test               # Run both C++ and Python tests
```

**C++ tests only:**
```bash
make test-cpp
# OR manually:
colcon test --packages-select rmw_introspect_cpp
colcon test-result --verbose
```

**Python tests only (requires workspace to be sourced):**
```bash
source install/setup.bash
make test-python
# OR manually:
cd ros2_introspect && PYTEST_DISABLE_PLUGIN_AUTOLOAD=1 uv run pytest
```

**Individual C++ test targets:**
- `test_init` - Tests RMW initialization functions
- `test_data` - Tests data structures and singleton
- `test_phase1` - Tests publishers and subscriptions
- `test_phase2` - Tests services and clients
- `test_phase3` - Tests wait sets and events
- `test_phase4_stubs` - Tests stub implementations

### Development Tools

```bash
make format             # Format C++ (clang-format) and Python (ruff)
make lint               # Check formatting and run linters
make clean              # Remove build artifacts
```

## Architecture

### rmw_introspect_cpp (C++ RMW Implementation)

This implements a complete RMW layer that intercepts ROS 2 middleware API calls. The core architecture:

**Core Data Flow:**
1. Node code calls standard ROS 2 RMW APIs (rmw_create_publisher, rmw_create_subscription, etc.)
2. rmw_introspect_cpp's implementations intercept these calls
3. Metadata is extracted and stored in a singleton `IntrospectionData` object
4. On shutdown, data is automatically exported to JSON/YAML

**Key Components:**

- **Singleton Data Store** (`data.hpp/cpp`): Thread-safe singleton that records all interface metadata
  - Stores: nodes, publishers, subscriptions, services, clients
  - Thread-safe using `std::mutex`
  - Exports to JSON or YAML format

- **Type Support** (`type_support.hpp/cpp`): Extracts message/service type names from ROS typesupport introspection
  - Uses `rosidl_typesupport_introspection_cpp` to get type information
  - Converts from introspection format to package/type strings (e.g., "std_msgs/msg/String")

- **RMW API Implementation**: Each RMW function is implemented in separate files:
  - `rmw_init.cpp` - Initialization and shutdown, exports data on shutdown
  - `rmw_node.cpp` - Node creation/destruction
  - `rmw_publisher.cpp` - Publisher creation/destruction
  - `rmw_subscription.cpp` - Subscription creation/destruction
  - `rmw_service.cpp` - Service creation/destruction
  - `rmw_client.cpp` - Client creation/destruction
  - `rmw_wait.cpp`, `rmw_graph.cpp`, `rmw_event.cpp` - Minimal stubs (return success/empty)
  - Phase 4 stubs (`rmw_gid.cpp`, `rmw_qos_compat.cpp`, etc.) - Additional stub implementations

**Data Structures:**
- `QoSProfile` - Stores reliability, durability, history, depth
- `PublisherInfo` - node_name, node_namespace, topic_name, message_type, qos, timestamp
- `SubscriptionInfo` - Same structure as PublisherInfo
- `ServiceInfo` - node_name, node_namespace, service_name, service_type, qos, timestamp
- `ClientInfo` - Same structure as ServiceInfo

**Environment Variables:**
- `RMW_IMPLEMENTATION=rmw_introspect_cpp` (Required) - Activates this RMW
- `RMW_INTROSPECT_OUTPUT` - Output file path (default: `/tmp/rmw_introspect_<pid>.json`)
- `RMW_INTROSPECT_FORMAT` - Output format: `json` or `yaml` (default: `json`)
- `RMW_INTROSPECT_VERBOSE` - Enable debug logging: `0` or `1` (default: `0`)
- `RMW_INTROSPECT_AUTO_EXPORT` - Auto-export on shutdown: `0` or `1` (default: `1`)

### ros2_introspect (Python Wrapper)

Provides a convenient Python interface over the C++ RMW implementation.

**Key Components:**

- **introspector.py**: Main introspection logic
  - `introspect_node()` - Spawns node with rmw_introspect_cpp and parses output
  - `check_rmw_introspect_available()` - Validates environment setup
  - Uses `ros2 run` command with RMW_IMPLEMENTATION override
  - Parses JSON output into structured Python objects

- **data.py**: Python dataclasses mirroring C++ structures
  - `QoSProfile`, `PublisherInfo`, `SubscriptionInfo`, `ServiceInfo`, `ClientInfo`
  - `IntrospectionResult` - Top-level result object

- **__main__.py**: CLI implementation
  - Provides `ros2-introspect` command
  - Supports ROS 2 arguments: `--namespace`, `--node-name`, remappings, parameters
  - Output formats: text (default) or JSON (`--format json`)

**Usage Pattern:**
```python
from ros2_introspect import introspect_node

result = introspect_node("demo_nodes_cpp", "talker",
                         namespace="/robot1",
                         parameters=[{"use_sim_time": True}])
if result.success:
    for pub in result.publishers:
        print(f"{pub.topic_name} ({pub.message_type})")
```

## Key Technical Details

### RMW Implementation Requirements
- Must implement the entire RMW API defined in `rmw/rmw.h`
- Uses C linkage (`extern "C"`) for all RMW functions
- Exports `rmw_introspect_cpp_identifier` as the implementation identifier
- Must validate all incoming parameters with RMW-specific checks

### Type Name Extraction
The type support system uses ROS introspection to extract fully-qualified type names:
- Requires both `rosidl_typesupport_introspection_cpp` and `rosidl_typesupport_cpp`
- Extracts package name, message/service namespace, and type name
- Handles both message types (for pub/sub) and service types (for service/client)

### Thread Safety
- All data recording operations are protected by `std::mutex` in the singleton
- Multiple nodes/threads can safely record interfaces concurrently

### Python Environment
- Requires Python 3.10 exactly (specified in pyproject.toml)
- Uses `uv` for dependency management
- Pytest configured to disable ROS-specific plugins that interfere with testing

## Common Development Workflows

### Adding a New RMW Function
1. Add function signature to appropriate `.cpp` file (or create new one)
2. Add to `CMakeLists.txt` sources if new file
3. Implement with proper error handling and identifier checks
4. Add test case to appropriate `test_phase*.cpp`

### Extending Metadata Collection
1. Add field to relevant struct in `types.hpp`
2. Update recording logic in `data.cpp`
3. Update export logic in `data.cpp` (JSON/YAML serialization)
4. Update Python dataclass in `data.py`
5. Update Python parsing in `introspector.py`

### Testing a Specific ROS 2 Node
```bash
source install/setup.bash
cd ros2_introspect
uv run ros2-introspect <package> <executable> [--namespace /ns] [--format json]
```

## Integration Context

This project was developed for the [ROS-Plan](https://github.com/jerry73204/ros-plan) launch-to-plan conversion workflow (launch2plan), enabling automatic discovery of node interfaces when converting ROS 2 launch files to declarative ROS-Plan format.

## Known Limitations

1. **Dynamic Interface Creation**: Only captures interfaces created during node initialization, not runtime-conditional interfaces
2. **Parameter-Dependent Interfaces**: Nodes creating different interfaces based on parameters require separate introspection runs with different configs
3. **Side Effects**: Cannot prevent nodes from attempting file I/O, hardware access, or network operations during initialization
