"""
CLI entry point for ros2-introspect tool.
"""

import argparse
import json
import sys

from .introspector import introspect_node


def main() -> int:
    """Main CLI entry point."""
    parser = argparse.ArgumentParser(
        prog="ros2-introspect",
        description="Introspect ROS 2 node interfaces using rmw_introspect_cpp",
    )
    parser.add_argument("package", help="ROS 2 package name")
    parser.add_argument("executable", help="Executable name within the package")
    parser.add_argument(
        "--namespace",
        "-n",
        help="Node namespace (e.g., /robot1)",
    )
    parser.add_argument(
        "--node-name",
        help="Override node name",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=3.0,
        help="Maximum time to wait for node initialization (seconds, default: 3.0)",
    )
    parser.add_argument(
        "--remap",
        "-r",
        action="append",
        help="Topic remapping in format 'from:=to' (can be specified multiple times)",
    )
    parser.add_argument(
        "--param",
        "-p",
        action="append",
        help="Parameter in format 'name:=value' (can be specified multiple times)",
    )
    parser.add_argument(
        "--format",
        "-f",
        choices=["text", "json"],
        default="text",
        help="Output format (default: text)",
    )

    args = parser.parse_args()

    # Parse remappings
    remappings = None
    if args.remap:
        remappings = []
        for remap in args.remap:
            if ":=" not in remap:
                print(
                    f"Error: Invalid remapping format '{remap}'. Use 'from:=to'",
                    file=sys.stderr,
                )
                return 1
            from_topic, to_topic = remap.split(":=", 1)
            remappings.append((from_topic, to_topic))

    # Parse parameters
    parameters = None
    if args.param:
        parameters = [{}]
        for param in args.param:
            if ":=" not in param:
                print(
                    f"Error: Invalid parameter format '{param}'. Use 'name:=value'",
                    file=sys.stderr,
                )
                return 1
            name, value = param.split(":=", 1)
            # Try to parse as int, float, bool, or keep as string
            try:
                if value.lower() in ("true", "false"):
                    parameters[0][name] = value.lower() == "true"
                elif "." in value:
                    parameters[0][name] = float(value)
                else:
                    parameters[0][name] = int(value)
            except ValueError:
                parameters[0][name] = value

    # Run introspection
    result = introspect_node(
        args.package,
        args.executable,
        parameters=parameters,
        remappings=remappings,
        namespace=args.namespace,
        node_name=args.node_name,
        timeout=args.timeout,
    )

    # Output results
    if args.format == "json":
        output = {
            "success": result.success,
            "error": result.error,
            "format_version": result.format_version,
            "timestamp": result.timestamp,
            "nodes": result.nodes,
            "publishers": [
                {
                    "topic_name": pub.topic_name,
                    "message_type": pub.message_type,
                    "node_name": pub.node_name,
                    "node_namespace": pub.node_namespace,
                    "qos": {
                        "reliability": pub.qos.reliability,
                        "durability": pub.qos.durability,
                        "history": pub.qos.history,
                        "depth": pub.qos.depth,
                    },
                }
                for pub in result.publishers
            ],
            "subscriptions": [
                {
                    "topic_name": sub.topic_name,
                    "message_type": sub.message_type,
                    "node_name": sub.node_name,
                    "node_namespace": sub.node_namespace,
                    "qos": {
                        "reliability": sub.qos.reliability,
                        "durability": sub.qos.durability,
                        "history": sub.qos.history,
                        "depth": sub.qos.depth,
                    },
                }
                for sub in result.subscriptions
            ],
            "services": [
                {
                    "service_name": srv.service_name,
                    "service_type": srv.service_type,
                    "node_name": srv.node_name,
                    "node_namespace": srv.node_namespace,
                }
                for srv in result.services
            ],
            "clients": [
                {
                    "service_name": client.service_name,
                    "service_type": client.service_type,
                    "node_name": client.node_name,
                    "node_namespace": client.node_namespace,
                }
                for client in result.clients
            ],
        }
        print(json.dumps(output, indent=2))
    else:
        # Text output
        if not result.success:
            print(f"Error: {result.error}", file=sys.stderr)
            return 1

        print("Introspection successful!")
        print(f"Format version: {result.format_version}")
        print(f"Timestamp: {result.timestamp}")
        print(f"\nNodes: {', '.join(result.nodes)}")

        if result.publishers:
            print(f"\nPublishers ({len(result.publishers)}):")
            for pub in result.publishers:
                print(f"  - {pub.topic_name}")
                print(f"    Type: {pub.message_type}")
                print(f"    Node: {pub.node_namespace}/{pub.node_name}")
                print(
                    f"    QoS: {pub.qos.reliability}, {pub.qos.durability}, depth={pub.qos.depth}"
                )

        if result.subscriptions:
            print(f"\nSubscriptions ({len(result.subscriptions)}):")
            for sub in result.subscriptions:
                print(f"  - {sub.topic_name}")
                print(f"    Type: {sub.message_type}")
                print(f"    Node: {sub.node_namespace}/{sub.node_name}")
                print(
                    f"    QoS: {sub.qos.reliability}, {sub.qos.durability}, depth={sub.qos.depth}"
                )

        if result.services:
            print(f"\nServices ({len(result.services)}):")
            for srv in result.services:
                print(f"  - {srv.service_name}")
                print(f"    Type: {srv.service_type}")
                print(f"    Node: {srv.node_namespace}/{srv.node_name}")

        if result.clients:
            print(f"\nClients ({len(result.clients)}):")
            for client in result.clients:
                print(f"  - {client.service_name}")
                print(f"    Type: {client.service_type}")
                print(f"    Node: {client.node_namespace}/{client.node_name}")

    return 0 if result.success else 1


if __name__ == "__main__":
    sys.exit(main())
