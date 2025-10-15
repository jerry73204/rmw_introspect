"""
Tests for RMW introspection wrapper.
"""

from ros2_introspect import introspect_node


class TestIntrospector:
    """Tests for node introspection."""

    def test_introspect_talker(self):
        """Test introspecting demo_nodes_cpp::talker."""
        result = introspect_node(
            "demo_nodes_cpp",
            "talker",
            timeout=3.0,
        )

        assert result.success, f"Introspection failed: {result.error}"
        assert len(result.nodes) == 1
        assert "talker" in result.nodes[0]

        # Should have at least the /chatter publisher
        assert len(result.publishers) >= 1
        chatter_pubs = [p for p in result.publishers if p.topic_name == "/chatter"]
        assert len(chatter_pubs) == 1

        chatter = chatter_pubs[0]
        assert chatter.message_type == "std_msgs/msg/String"
        assert chatter.node_name == "talker"
        assert chatter.qos.reliability in ["reliable", "best_effort"]
        assert chatter.qos.durability in ["volatile", "transient_local"]

    def test_introspect_listener(self):
        """Test introspecting demo_nodes_cpp::listener."""
        result = introspect_node(
            "demo_nodes_cpp",
            "listener",
            timeout=3.0,
        )

        assert result.success, f"Introspection failed: {result.error}"
        assert len(result.nodes) == 1
        assert "listener" in result.nodes[0]

        # Should have at least the /chatter subscription
        assert len(result.subscriptions) >= 1
        chatter_subs = [s for s in result.subscriptions if s.topic_name == "/chatter"]
        assert len(chatter_subs) == 1

        chatter = chatter_subs[0]
        assert chatter.message_type == "std_msgs/msg/String"
        assert chatter.node_name == "listener"

    def test_introspect_with_namespace(self):
        """Test introspecting node with custom namespace."""
        result = introspect_node(
            "demo_nodes_cpp",
            "talker",
            namespace="/test_ns",
            timeout=3.0,
        )

        assert result.success, f"Introspection failed: {result.error}"
        assert len(result.nodes) == 1
        # Node namespace should be reflected
        assert result.nodes[0].startswith("/test_ns")

    def test_introspect_with_remapping(self):
        """Test introspecting node with topic remapping."""
        result = introspect_node(
            "demo_nodes_cpp",
            "talker",
            remappings=[("/chatter", "/my_topic")],
            timeout=3.0,
        )

        assert result.success, f"Introspection failed: {result.error}"

        # Should see remapped topic name
        my_topic_pubs = [p for p in result.publishers if p.topic_name == "/my_topic"]
        assert len(my_topic_pubs) == 1

    def test_introspect_invalid_package(self):
        """Test introspecting non-existent package."""
        result = introspect_node(
            "nonexistent_package",
            "nonexistent_executable",
            timeout=1.0,
        )

        assert not result.success
        assert result.error is not None

    def test_result_format(self):
        """Test that result has correct format and fields."""
        result = introspect_node(
            "demo_nodes_cpp",
            "talker",
            timeout=3.0,
        )

        assert result.success
        assert result.format_version is not None
        assert result.timestamp is not None

        # Check QoS profile structure
        for pub in result.publishers:
            assert hasattr(pub, "qos")
            assert hasattr(pub.qos, "reliability")
            assert hasattr(pub.qos, "durability")
            assert hasattr(pub.qos, "history")
            assert hasattr(pub.qos, "depth")
            assert isinstance(pub.qos.depth, int)
