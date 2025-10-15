#include <gtest/gtest.h>
#include "rmw/rmw.h"
#include "rmw/init.h"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/detail/string__rosidl_typesupport_introspection_cpp.hpp"
#include "rmw_introspect/data.hpp"

// Test node creation
TEST(TestPhase1, NodeCreation) {
  rmw_init_options_t options = rmw_get_zero_initialized_init_options();
  rmw_context_t context = rmw_get_zero_initialized_context();
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  rmw_ret_t ret = rmw_init_options_init(&options, allocator);
  ASSERT_EQ(ret, RMW_RET_OK);

  ret = rmw_init(&options, &context);
  ASSERT_EQ(ret, RMW_RET_OK);

  // Create node
  rmw_node_t * node = rmw_create_node(&context, "test_node", "/test_ns");
  ASSERT_NE(node, nullptr);
  EXPECT_STREQ(node->name, "test_node");
  EXPECT_STREQ(node->namespace_, "/test_ns");

  // Verify node recorded
  auto & data = rmw_introspect::IntrospectionData::instance();
  const auto & nodes = data.get_nodes();
  bool found = false;
  for (const auto & n : nodes) {
    if (n == "/test_ns/test_node") {
      found = true;
      break;
    }
  }
  EXPECT_TRUE(found);

  // Clean up
  ret = rmw_destroy_node(node);
  EXPECT_EQ(ret, RMW_RET_OK);

  rmw_shutdown(&context);
  rmw_context_fini(&context);
  rmw_init_options_fini(&options);
}

// Test publisher creation
TEST(TestPhase1, PublisherCreation) {
  rmw_init_options_t options = rmw_get_zero_initialized_init_options();
  rmw_context_t context = rmw_get_zero_initialized_context();
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  rmw_init_options_init(&options, allocator);
  rmw_init(&options, &context);

  rmw_node_t * node = rmw_create_node(&context, "pub_node", "/");
  ASSERT_NE(node, nullptr);

  // Get introspection type support for std_msgs/msg/String
  const rosidl_message_type_support_t * type_support =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
      rosidl_typesupport_introspection_cpp, std_msgs, msg, String)();

  rmw_qos_profile_t qos = rmw_qos_profile_default;
  rmw_publisher_options_t pub_options = rmw_get_default_publisher_options();

  // Create publisher
  rmw_publisher_t * publisher = rmw_create_publisher(
    node, type_support, "test_topic", &qos, &pub_options);
  ASSERT_NE(publisher, nullptr);
  EXPECT_STREQ(publisher->topic_name, "test_topic");

  // Verify publisher recorded
  auto & data = rmw_introspect::IntrospectionData::instance();
  const auto & pubs = data.get_publishers();
  bool found = false;
  for (const auto & p : pubs) {
    if (p.topic_name == "test_topic" && p.message_type == "std_msgs/msg/String") {
      found = true;
      EXPECT_EQ(p.node_name, "pub_node");
      break;
    }
  }
  EXPECT_TRUE(found);

  // Test publish (no-op)
  std_msgs::msg::String msg;
  rmw_ret_t ret = rmw_publish(publisher, &msg, nullptr);
  EXPECT_EQ(ret, RMW_RET_OK);

  // Clean up
  rmw_destroy_publisher(node, publisher);
  rmw_destroy_node(node);
  rmw_shutdown(&context);
  rmw_context_fini(&context);
  rmw_init_options_fini(&options);
}

// Test subscription creation
TEST(TestPhase1, SubscriptionCreation) {
  rmw_init_options_t options = rmw_get_zero_initialized_init_options();
  rmw_context_t context = rmw_get_zero_initialized_context();
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  rmw_init_options_init(&options, allocator);
  rmw_init(&options, &context);

  rmw_node_t * node = rmw_create_node(&context, "sub_node", "/");
  ASSERT_NE(node, nullptr);

  const rosidl_message_type_support_t * type_support =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
      rosidl_typesupport_introspection_cpp, std_msgs, msg, String)();

  rmw_qos_profile_t qos = rmw_qos_profile_default;
  rmw_subscription_options_t sub_options = rmw_get_default_subscription_options();

  // Create subscription
  rmw_subscription_t * subscription = rmw_create_subscription(
    node, type_support, "test_sub_topic", &qos, &sub_options);
  ASSERT_NE(subscription, nullptr);
  EXPECT_STREQ(subscription->topic_name, "test_sub_topic");

  // Verify subscription recorded
  auto & data = rmw_introspect::IntrospectionData::instance();
  const auto & subs = data.get_subscriptions();
  bool found = false;
  for (const auto & s : subs) {
    if (s.topic_name == "test_sub_topic" && s.message_type == "std_msgs/msg/String") {
      found = true;
      EXPECT_EQ(s.node_name, "sub_node");
      break;
    }
  }
  EXPECT_TRUE(found);

  // Test take (no-op)
  std_msgs::msg::String msg;
  bool taken = true;
  rmw_ret_t ret = rmw_take(subscription, &msg, &taken, nullptr);
  EXPECT_EQ(ret, RMW_RET_OK);
  EXPECT_FALSE(taken);

  // Clean up
  rmw_destroy_subscription(node, subscription);
  rmw_destroy_node(node);
  rmw_shutdown(&context);
  rmw_context_fini(&context);
  rmw_init_options_fini(&options);
}

int main(int argc, char ** argv) {
  // Clear introspection data before tests
  rmw_introspect::IntrospectionData::instance().clear();

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
