"""
ROS 2 node introspection tool using rmw_introspect_cpp.

This package provides a Python wrapper around rmw_introspect_cpp for
discovering ROS 2 node interfaces (publishers, subscriptions, services, clients)
without running actual middleware communication.
"""

from .data import (
    ClientInfo,
    IntrospectionResult,
    PublisherInfo,
    QoSProfile,
    ServiceInfo,
    SubscriptionInfo,
)
from .introspector import check_rmw_introspect_available, introspect_node

__version__ = "0.1.0"

__all__ = [
    "ClientInfo",
    "IntrospectionResult",
    "PublisherInfo",
    "QoSProfile",
    "ServiceInfo",
    "SubscriptionInfo",
    "check_rmw_introspect_available",
    "introspect_node",
]
