"""
Data structures for ROS 2 node introspection results.
"""

from dataclasses import dataclass, field
from typing import List, Optional


@dataclass
class QoSProfile:
    """QoS profile for a ROS 2 interface."""

    reliability: str
    durability: str
    history: str
    depth: int


@dataclass
class PublisherInfo:
    """Publisher interface metadata."""

    topic_name: str
    message_type: str
    qos: QoSProfile
    node_name: str
    node_namespace: str


@dataclass
class SubscriptionInfo:
    """Subscription interface metadata."""

    topic_name: str
    message_type: str
    qos: QoSProfile
    node_name: str
    node_namespace: str


@dataclass
class ServiceInfo:
    """Service interface metadata."""

    service_name: str
    service_type: str
    node_name: str
    node_namespace: str


@dataclass
class ClientInfo:
    """Client interface metadata."""

    service_name: str
    service_type: str
    node_name: str
    node_namespace: str


@dataclass
class IntrospectionResult:
    """Result of introspecting a ROS 2 node."""

    success: bool
    nodes: List[str] = field(default_factory=list)
    publishers: List[PublisherInfo] = field(default_factory=list)
    subscriptions: List[SubscriptionInfo] = field(default_factory=list)
    services: List[ServiceInfo] = field(default_factory=list)
    clients: List[ClientInfo] = field(default_factory=list)
    error: Optional[str] = None
    format_version: Optional[str] = None
    timestamp: Optional[str] = None
