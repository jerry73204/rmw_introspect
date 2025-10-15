#!/usr/bin/env python3
"""
JSON schema validator for rmw_introspect_cpp output
Validates that introspection output matches expected schema
"""

import json
import sys
import re
from pathlib import Path

# Expected schema for rmw_introspect output
SCHEMA = {
    "type": "object",
    "required": ["format_version", "timestamp", "rmw_implementation", "nodes"],
    "properties": {
        "format_version": {"type": "string"},
        "timestamp": {"type": ["string", "number"]},
        "rmw_implementation": {"const": "rmw_introspect_cpp"},
        "nodes": {"type": "array"},
        "publishers": {"type": "array"},
        "subscriptions": {"type": "array"},
        "services": {"type": "array"},
        "clients": {"type": "array"}
    }
}

def validate_message_type_format(msg_type: str) -> bool:
    """Validate message type format: package/msg/Type or package/srv/Type"""
    pattern = r'^[a-z_][a-z0-9_]*/(msg|srv)/[A-Z][a-zA-Z0-9]*$'
    return bool(re.match(pattern, msg_type))

def validate_qos_profile(qos: dict) -> list:
    """Validate QoS profile structure"""
    errors = []
    required_fields = ["reliability", "durability", "history", "depth"]

    for field in required_fields:
        if field not in qos:
            errors.append(f"Missing QoS field: {field}")

    if "reliability" in qos and qos["reliability"] not in ["reliable", "best_effort"]:
        errors.append(f"Invalid reliability: {qos['reliability']}")

    if "durability" in qos and qos["durability"] not in ["transient_local", "volatile"]:
        errors.append(f"Invalid durability: {qos['durability']}")

    if "history" in qos and qos["history"] not in ["keep_last", "keep_all"]:
        errors.append(f"Invalid history: {qos['history']}")

    if "depth" in qos and not isinstance(qos["depth"], int):
        errors.append(f"Invalid depth type: {type(qos['depth'])}")

    return errors

def validate_output(json_file: str) -> tuple[bool, list]:
    """
    Validate rmw_introspect JSON output

    Returns:
        (is_valid, errors) tuple
    """
    errors = []

    # Load JSON
    try:
        with open(json_file) as f:
            data = json.load(f)
    except FileNotFoundError:
        return False, [f"File not found: {json_file}"]
    except json.JSONDecodeError as e:
        return False, [f"Invalid JSON: {e}"]

    # Check required top-level fields
    for field in SCHEMA["required"]:
        if field not in data:
            errors.append(f"Missing required field: {field}")

    # Validate rmw_implementation
    if data.get("rmw_implementation") != "rmw_introspect_cpp":
        errors.append(f"Invalid rmw_implementation: {data.get('rmw_implementation')}")

    # Validate format_version
    if not isinstance(data.get("format_version"), str):
        errors.append("format_version must be a string")

    # Validate timestamp
    timestamp = data.get("timestamp")
    if not isinstance(timestamp, (str, int, float)):
        errors.append(f"Invalid timestamp type: {type(timestamp)}")

    # Validate nodes array
    if not isinstance(data.get("nodes"), list):
        errors.append("nodes must be an array")
    else:
        for i, node in enumerate(data["nodes"]):
            if not isinstance(node, str):
                errors.append(f"Node {i}: must be a string")
            elif not node.startswith("/"):
                errors.append(f"Node {i}: must start with / (got: {node})")

    # Validate publishers
    if "publishers" in data:
        if not isinstance(data["publishers"], list):
            errors.append("publishers must be an array")
        else:
            for i, pub in enumerate(data["publishers"]):
                if not pub.get("topic_name"):
                    errors.append(f"Publisher {i}: missing topic_name")

                msg_type = pub.get("message_type")
                if not msg_type:
                    errors.append(f"Publisher {i}: missing message_type")
                elif not validate_message_type_format(msg_type):
                    errors.append(f"Publisher {i}: invalid message_type format: {msg_type}")

                if "qos" in pub:
                    qos_errors = validate_qos_profile(pub["qos"])
                    for err in qos_errors:
                        errors.append(f"Publisher {i}: {err}")

    # Validate subscriptions
    if "subscriptions" in data:
        if not isinstance(data["subscriptions"], list):
            errors.append("subscriptions must be an array")
        else:
            for i, sub in enumerate(data["subscriptions"]):
                if not sub.get("topic_name"):
                    errors.append(f"Subscription {i}: missing topic_name")

                msg_type = sub.get("message_type")
                if not msg_type:
                    errors.append(f"Subscription {i}: missing message_type")
                elif not validate_message_type_format(msg_type):
                    errors.append(f"Subscription {i}: invalid message_type format: {msg_type}")

                if "qos" in sub:
                    qos_errors = validate_qos_profile(sub["qos"])
                    for err in qos_errors:
                        errors.append(f"Subscription {i}: {err}")

    # Validate services
    if "services" in data:
        if not isinstance(data["services"], list):
            errors.append("services must be an array")
        else:
            for i, srv in enumerate(data["services"]):
                if not srv.get("service_name"):
                    errors.append(f"Service {i}: missing service_name")

                srv_type = srv.get("service_type")
                if not srv_type:
                    errors.append(f"Service {i}: missing service_type")
                elif not validate_message_type_format(srv_type):
                    errors.append(f"Service {i}: invalid service_type format: {srv_type}")

    # Validate clients
    if "clients" in data:
        if not isinstance(data["clients"], list):
            errors.append("clients must be an array")
        else:
            for i, client in enumerate(data["clients"]):
                if not client.get("service_name"):
                    errors.append(f"Client {i}: missing service_name")

                srv_type = client.get("service_type")
                if not srv_type:
                    errors.append(f"Client {i}: missing service_type")
                elif not validate_message_type_format(srv_type):
                    errors.append(f"Client {i}: invalid service_type format: {srv_type}")

    return (len(errors) == 0, errors)

def main():
    if len(sys.argv) != 2:
        print("Usage: validate_json.py <json_file>")
        sys.exit(1)

    json_file = sys.argv[1]
    is_valid, errors = validate_output(json_file)

    if is_valid:
        print(f"✓ {json_file} is valid")
        sys.exit(0)
    else:
        print(f"✗ {json_file} validation failed:")
        for error in errors:
            print(f"  - {error}")
        sys.exit(1)

if __name__ == "__main__":
    main()
