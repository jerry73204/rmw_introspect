#include <gtest/gtest.h>

#include "rcutils/allocator.h"
#include "rmw/error_handling.h"
#include "rmw/event.h"
#include "rmw/names_and_types.h"
#include "rmw/rmw.h"
#include "rmw_introspect/identifier.hpp"
#include "std_msgs/msg/string.h"
#include "std_msgs/msg/detail/string__type_support.h"

#include <cstdlib>
#include <cstring>

class AdvancedIntermediateTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Set intermediate mode
    setenv("RMW_INTROSPECT_DELEGATE_TO", "rmw_fastrtps_cpp", 1);

    // Initialize allocator
    allocator_ = rcutils_get_default_allocator();

    // Initialize options
    rmw_ret_t ret = rmw_init_options_init(&init_options_, allocator_);
    ASSERT_EQ(RMW_RET_OK, ret);

    // Initialize context
    context_ = rmw_get_zero_initialized_context();
    ret = rmw_init(&init_options_, &context_);
    ASSERT_EQ(RMW_RET_OK, ret);

    // Create node
    node_ = rmw_create_node(&context_, "test_node", "/test");
    ASSERT_NE(nullptr, node_);
    ASSERT_STREQ(rmw_introspect_cpp_identifier,
                 node_->implementation_identifier);
  }

  void TearDown() override {
    if (node_) {
      rmw_ret_t ret = rmw_destroy_node(node_);
      EXPECT_EQ(RMW_RET_OK, ret);
    }

    if (context_.implementation_identifier != nullptr) {
      rmw_ret_t ret = rmw_shutdown(&context_);
      EXPECT_EQ(RMW_RET_OK, ret);

      ret = rmw_context_fini(&context_);
      EXPECT_EQ(RMW_RET_OK, ret);
    }

    rmw_ret_t ret = rmw_init_options_fini(&init_options_);
    EXPECT_EQ(RMW_RET_OK, ret);

    unsetenv("RMW_INTROSPECT_DELEGATE_TO");
  }

  rcutils_allocator_t allocator_;
  rmw_init_options_t init_options_;
  rmw_context_t context_;
  rmw_node_t *node_{nullptr};
};

TEST_F(AdvancedIntermediateTest, GuardConditionCreateDestroy) {
  // Create guard condition
  rmw_guard_condition_t *guard_condition = rmw_create_guard_condition(&context_);
  ASSERT_NE(nullptr, guard_condition);
  ASSERT_STREQ(rmw_introspect_cpp_identifier,
               guard_condition->implementation_identifier);
  ASSERT_NE(nullptr, guard_condition->data); // Should have wrapper

  // Trigger guard condition
  rmw_ret_t ret = rmw_trigger_guard_condition(guard_condition);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Destroy guard condition
  ret = rmw_destroy_guard_condition(guard_condition);
  EXPECT_EQ(RMW_RET_OK, ret);
}

TEST_F(AdvancedIntermediateTest, WaitSetCreateDestroy) {
  // Create wait set
  rmw_wait_set_t *wait_set = rmw_create_wait_set(&context_, 10);
  ASSERT_NE(nullptr, wait_set);
  ASSERT_STREQ(rmw_introspect_cpp_identifier,
               wait_set->implementation_identifier);
  ASSERT_NE(nullptr, wait_set->data); // Should have wrapper

  // Destroy wait set
  rmw_ret_t ret = rmw_destroy_wait_set(wait_set);
  EXPECT_EQ(RMW_RET_OK, ret);
}

TEST_F(AdvancedIntermediateTest, GraphNodeQueries) {
  // Get node names
  rcutils_string_array_t node_names = rcutils_get_zero_initialized_string_array();
  rcutils_string_array_t node_namespaces = rcutils_get_zero_initialized_string_array();
  rmw_ret_t ret = rmw_get_node_names(node_, &node_names, &node_namespaces);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Cleanup
  rcutils_string_array_fini(&node_names);
  rcutils_string_array_fini(&node_namespaces);

  // Get node names with enclaves
  node_names = rcutils_get_zero_initialized_string_array();
  node_namespaces = rcutils_get_zero_initialized_string_array();
  rcutils_string_array_t enclaves = rcutils_get_zero_initialized_string_array();
  ret = rmw_get_node_names_with_enclaves(node_, &node_names, &node_namespaces, &enclaves);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Cleanup
  rcutils_string_array_fini(&node_names);
  rcutils_string_array_fini(&node_namespaces);
  rcutils_string_array_fini(&enclaves);
}

TEST_F(AdvancedIntermediateTest, GraphTopicQueries) {
  // Get topic names and types
  rmw_names_and_types_t topic_names_and_types =
      rmw_get_zero_initialized_names_and_types();
  rmw_ret_t ret = rmw_get_topic_names_and_types(
      node_, &allocator_, false, &topic_names_and_types);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Cleanup
  rmw_names_and_types_fini(&topic_names_and_types);

  // Get service names and types
  rmw_names_and_types_t service_names_and_types =
      rmw_get_zero_initialized_names_and_types();
  ret = rmw_get_service_names_and_types(
      node_, &allocator_, &service_names_and_types);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Cleanup
  rmw_names_and_types_fini(&service_names_and_types);
}

TEST_F(AdvancedIntermediateTest, CountPublishersSubscribers) {
  // Create a publisher
  const rosidl_message_type_support_t *type_support =
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String);
  rmw_qos_profile_t qos_profile = rmw_qos_profile_default;
  rmw_publisher_options_t pub_options = rmw_get_default_publisher_options();
  rmw_publisher_t *publisher = rmw_create_publisher(
      node_, type_support, "/test_topic", &qos_profile, &pub_options);
  ASSERT_NE(nullptr, publisher);

  // Count publishers
  size_t count = 0;
  rmw_ret_t ret = rmw_count_publishers(node_, "/test_topic", &count);
  EXPECT_EQ(RMW_RET_OK, ret);
  // Note: count may be 0 or higher depending on timing and backend

  // Count subscribers
  count = 0;
  ret = rmw_count_subscribers(node_, "/test_topic", &count);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Cleanup
  ret = rmw_destroy_publisher(node_, publisher);
  EXPECT_EQ(RMW_RET_OK, ret);
}

TEST_F(AdvancedIntermediateTest, GIDOperations) {
  // Create a publisher
  const rosidl_message_type_support_t *type_support =
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String);
  rmw_qos_profile_t qos_profile = rmw_qos_profile_default;
  rmw_publisher_options_t pub_options = rmw_get_default_publisher_options();
  rmw_publisher_t *publisher = rmw_create_publisher(
      node_, type_support, "/test_topic", &qos_profile, &pub_options);
  ASSERT_NE(nullptr, publisher);

  // Get GID for publisher
  rmw_gid_t gid;
  rmw_ret_t ret = rmw_get_gid_for_publisher(publisher, &gid);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Compare GIDs
  rmw_gid_t gid2;
  ret = rmw_get_gid_for_publisher(publisher, &gid2);
  EXPECT_EQ(RMW_RET_OK, ret);

  bool are_equal = false;
  ret = rmw_compare_gids_equal(&gid, &gid2, &are_equal);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Cleanup
  ret = rmw_destroy_publisher(node_, publisher);
  EXPECT_EQ(RMW_RET_OK, ret);
}

TEST_F(AdvancedIntermediateTest, EventOperations) {
  // Create a publisher
  const rosidl_message_type_support_t *type_support =
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String);
  rmw_qos_profile_t qos_profile = rmw_qos_profile_default;
  rmw_publisher_options_t pub_options = rmw_get_default_publisher_options();
  rmw_publisher_t *publisher = rmw_create_publisher(
      node_, type_support, "/test_topic", &qos_profile, &pub_options);
  ASSERT_NE(nullptr, publisher);

  // Initialize publisher event
  rmw_event_t event = rmw_get_zero_initialized_event();
  rmw_ret_t ret = rmw_publisher_event_init(
      &event, publisher, RMW_EVENT_LIVELINESS_LOST);
  // Event init may succeed or return unsupported depending on backend
  EXPECT_TRUE(ret == RMW_RET_OK || ret == RMW_RET_UNSUPPORTED);

  if (ret == RMW_RET_OK) {
    // Finalize event
    ret = rmw_event_fini(&event);
    EXPECT_EQ(RMW_RET_OK, ret);
  }

  // Cleanup
  ret = rmw_destroy_publisher(node_, publisher);
  EXPECT_EQ(RMW_RET_OK, ret);
}

TEST_F(AdvancedIntermediateTest, GetNamesAndTypesByNode) {
  // Get publisher names and types by node
  rmw_names_and_types_t names_and_types =
      rmw_get_zero_initialized_names_and_types();
  rmw_ret_t ret = rmw_get_publisher_names_and_types_by_node(
      node_, &allocator_, "test_node", "/test", false, &names_and_types);
  EXPECT_EQ(RMW_RET_OK, ret);
  rmw_names_and_types_fini(&names_and_types);

  // Get subscriber names and types by node
  names_and_types = rmw_get_zero_initialized_names_and_types();
  ret = rmw_get_subscriber_names_and_types_by_node(
      node_, &allocator_, "test_node", "/test", false, &names_and_types);
  EXPECT_EQ(RMW_RET_OK, ret);
  rmw_names_and_types_fini(&names_and_types);

  // Get service names and types by node
  names_and_types = rmw_get_zero_initialized_names_and_types();
  ret = rmw_get_service_names_and_types_by_node(
      node_, &allocator_, "test_node", "/test", &names_and_types);
  EXPECT_EQ(RMW_RET_OK, ret);
  rmw_names_and_types_fini(&names_and_types);

  // Get client names and types by node
  names_and_types = rmw_get_zero_initialized_names_and_types();
  ret = rmw_get_client_names_and_types_by_node(
      node_, &allocator_, "test_node", "/test", &names_and_types);
  EXPECT_EQ(RMW_RET_OK, ret);
  rmw_names_and_types_fini(&names_and_types);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
