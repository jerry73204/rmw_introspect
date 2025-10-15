#include <gtest/gtest.h>
#include "rmw/rmw.h"
#include "rmw/init.h"
#include "rmw/names_and_types.h"
#include "rmw/get_node_info_and_types.h"
#include "rmw/get_topic_names_and_types.h"
#include "rmw/get_service_names_and_types.h"
#include "rmw/get_network_flow_endpoints.h"
#include "rmw/network_flow_endpoint_array.h"
#include "rmw/validate_full_topic_name.h"
#include "rmw/validate_namespace.h"
#include "rmw/validate_node_name.h"
#include "rcutils/types/string_array.h"

// Test graph query functions
TEST(TestPhase3, CountPublishers) {
  rmw_init_options_t options = rmw_get_zero_initialized_init_options();
  rmw_context_t context = rmw_get_zero_initialized_context();
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  rmw_init_options_init(&options, allocator);
  rmw_init(&options, &context);

  rmw_node_t * node = rmw_create_node(&context, "test_node", "/");
  ASSERT_NE(node, nullptr);

  size_t count = 999;
  rmw_ret_t ret = rmw_count_publishers(node, "test_topic", &count);
  EXPECT_EQ(ret, RMW_RET_OK);
  EXPECT_EQ(count, 0);  // Should return 0

  // Clean up
  rmw_destroy_node(node);
  rmw_shutdown(&context);
  rmw_context_fini(&context);
  rmw_init_options_fini(&options);
}

TEST(TestPhase3, CountSubscribers) {
  rmw_init_options_t options = rmw_get_zero_initialized_init_options();
  rmw_context_t context = rmw_get_zero_initialized_context();
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  rmw_init_options_init(&options, allocator);
  rmw_init(&options, &context);

  rmw_node_t * node = rmw_create_node(&context, "test_node", "/");
  ASSERT_NE(node, nullptr);

  size_t count = 999;
  rmw_ret_t ret = rmw_count_subscribers(node, "test_topic", &count);
  EXPECT_EQ(ret, RMW_RET_OK);
  EXPECT_EQ(count, 0);  // Should return 0

  // Clean up
  rmw_destroy_node(node);
  rmw_shutdown(&context);
  rmw_context_fini(&context);
  rmw_init_options_fini(&options);
}

TEST(TestPhase3, GetNodeNames) {
  rmw_init_options_t options = rmw_get_zero_initialized_init_options();
  rmw_context_t context = rmw_get_zero_initialized_context();
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  rmw_init_options_init(&options, allocator);
  rmw_init(&options, &context);

  rmw_node_t * node = rmw_create_node(&context, "test_node", "/");
  ASSERT_NE(node, nullptr);

  rcutils_string_array_t node_names = rcutils_get_zero_initialized_string_array();
  rcutils_string_array_t node_namespaces = rcutils_get_zero_initialized_string_array();

  rmw_ret_t ret = rmw_get_node_names(node, &node_names, &node_namespaces);
  EXPECT_EQ(ret, RMW_RET_OK);
  EXPECT_EQ(node_names.size, 0u);  // Should return empty
  EXPECT_EQ(node_namespaces.size, 0u);

  // Clean up
  rcutils_string_array_fini(&node_names);
  rcutils_string_array_fini(&node_namespaces);
  rmw_destroy_node(node);
  rmw_shutdown(&context);
  rmw_context_fini(&context);
  rmw_init_options_fini(&options);
}

TEST(TestPhase3, GetTopicNamesAndTypes) {
  rmw_init_options_t options = rmw_get_zero_initialized_init_options();
  rmw_context_t context = rmw_get_zero_initialized_context();
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  rmw_init_options_init(&options, allocator);
  rmw_init(&options, &context);

  rmw_node_t * node = rmw_create_node(&context, "test_node", "/");
  ASSERT_NE(node, nullptr);

  rmw_names_and_types_t topic_names_and_types = rmw_get_zero_initialized_names_and_types();

  rmw_ret_t ret = rmw_get_topic_names_and_types(
    node, &allocator, false, &topic_names_and_types);
  EXPECT_EQ(ret, RMW_RET_OK);
  EXPECT_EQ(topic_names_and_types.names.size, 0u);  // Should return empty

  // Clean up
  rmw_names_and_types_fini(&topic_names_and_types);
  rmw_destroy_node(node);
  rmw_shutdown(&context);
  rmw_context_fini(&context);
  rmw_init_options_fini(&options);
}

// Test serialization functions (no-ops)
TEST(TestPhase3, Serialization) {
  // Test serialize - should succeed as no-op
  rmw_serialized_message_t serialized = rmw_get_zero_initialized_serialized_message();
  rmw_ret_t ret = rmw_serialize(nullptr, nullptr, &serialized);
  EXPECT_EQ(ret, RMW_RET_OK);

  // Test deserialize - should succeed as no-op
  ret = rmw_deserialize(&serialized, nullptr, nullptr);
  EXPECT_EQ(ret, RMW_RET_OK);

  // Test get size - should return 0
  size_t size = 999;
  ret = rmw_get_serialized_message_size(nullptr, nullptr, &size);
  EXPECT_EQ(ret, RMW_RET_OK);
  EXPECT_EQ(size, 0u);
}

// Test network flow endpoints (should return empty)
TEST(TestPhase3, NetworkFlowEndpoints) {
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  // Create a minimal publisher structure for testing
  rmw_publisher_t pub;
  pub.implementation_identifier = "rmw_introspect_cpp";
  pub.data = nullptr;
  pub.topic_name = "test_topic";
  pub.can_loan_messages = false;

  rmw_network_flow_endpoint_array_t endpoints =
    rmw_get_zero_initialized_network_flow_endpoint_array();

  // Test that the function returns empty array
  rmw_ret_t ret = rmw_publisher_get_network_flow_endpoints(&pub, &allocator, &endpoints);
  EXPECT_EQ(ret, RMW_RET_OK);
  EXPECT_EQ(endpoints.size, 0u);  // Should return empty

  // Clean up
  rmw_network_flow_endpoint_array_fini(&endpoints);
}

// Test validation functions
TEST(TestPhase3, ValidateTopicName) {
  int validation_result = 0;
  size_t invalid_index = 0;

  // Valid topic name
  rmw_ret_t ret = rmw_validate_full_topic_name("/test_topic", &validation_result, &invalid_index);
  EXPECT_EQ(ret, RMW_RET_OK);
  EXPECT_EQ(validation_result, RMW_TOPIC_VALID);

  // Invalid topic name (empty)
  ret = rmw_validate_full_topic_name("", &validation_result, &invalid_index);
  EXPECT_EQ(ret, RMW_RET_OK);
  EXPECT_NE(validation_result, RMW_TOPIC_VALID);
}

TEST(TestPhase3, ValidateNodeName) {
  int validation_result = 0;
  size_t invalid_index = 0;

  // Valid node name
  rmw_ret_t ret = rmw_validate_node_name("test_node", &validation_result, &invalid_index);
  EXPECT_EQ(ret, RMW_RET_OK);
  EXPECT_EQ(validation_result, RMW_NODE_NAME_VALID);

  // Invalid node name (empty)
  ret = rmw_validate_node_name("", &validation_result, &invalid_index);
  EXPECT_EQ(ret, RMW_RET_OK);
  EXPECT_NE(validation_result, RMW_NODE_NAME_VALID);
}

TEST(TestPhase3, ValidateNamespace) {
  int validation_result = 0;
  size_t invalid_index = 0;

  // Valid namespace
  rmw_ret_t ret = rmw_validate_namespace("/test", &validation_result, &invalid_index);
  EXPECT_EQ(ret, RMW_RET_OK);
  EXPECT_EQ(validation_result, RMW_NAMESPACE_VALID);

  // Invalid namespace (empty)
  ret = rmw_validate_namespace("", &validation_result, &invalid_index);
  EXPECT_EQ(ret, RMW_RET_OK);
  EXPECT_NE(validation_result, RMW_NAMESPACE_VALID);
}

int main(int argc, char ** argv) {
  // Clear introspection data before tests
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
