#include <gtest/gtest.h>

#include "rcutils/allocator.h"
#include "rcutils/strdup.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw_introspect/identifier.hpp"
#include "std_msgs/msg/string.h"
#include "std_msgs/msg/detail/string__type_support.h"

#include <cstdlib>
#include <cstring>

class PublisherSubscriberIntermediateTest : public ::testing::Test {
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

    if (rmw_context_is_valid(&context_)) {
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

TEST_F(PublisherSubscriberIntermediateTest, CreateDestroyPublisher) {
  // Get type support
  const rosidl_message_type_support_t *type_support =
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String);
  ASSERT_NE(nullptr, type_support);

  // Create publisher
  rmw_qos_profile_t qos_profile = rmw_qos_profile_default;
  rmw_publisher_options_t pub_options = rmw_get_default_publisher_options();

  rmw_publisher_t *publisher = rmw_create_publisher(
      node_, type_support, "/test_topic", &qos_profile, &pub_options);
  ASSERT_NE(nullptr, publisher);
  ASSERT_STREQ(rmw_introspect_cpp_identifier,
               publisher->implementation_identifier);
  ASSERT_NE(nullptr, publisher->data); // Should have wrapper

  // Destroy publisher
  rmw_ret_t ret = rmw_destroy_publisher(node_, publisher);
  EXPECT_EQ(RMW_RET_OK, ret);
}

TEST_F(PublisherSubscriberIntermediateTest, CreateDestroySubscription) {
  // Get type support
  const rosidl_message_type_support_t *type_support =
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String);
  ASSERT_NE(nullptr, type_support);

  // Create subscription
  rmw_qos_profile_t qos_profile = rmw_qos_profile_default;
  rmw_subscription_options_t sub_options = rmw_get_default_subscription_options();

  rmw_subscription_t *subscription = rmw_create_subscription(
      node_, type_support, "/test_topic", &qos_profile, &sub_options);
  ASSERT_NE(nullptr, subscription);
  ASSERT_STREQ(rmw_introspect_cpp_identifier,
               subscription->implementation_identifier);
  ASSERT_NE(nullptr, subscription->data); // Should have wrapper

  // Destroy subscription
  rmw_ret_t ret = rmw_destroy_subscription(node_, subscription);
  EXPECT_EQ(RMW_RET_OK, ret);
}

TEST_F(PublisherSubscriberIntermediateTest, PublishAndTake) {
  // Get type support
  const rosidl_message_type_support_t *type_support =
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String);
  ASSERT_NE(nullptr, type_support);

  // Create publisher
  rmw_qos_profile_t qos_profile = rmw_qos_profile_default;
  rmw_publisher_options_t pub_options = rmw_get_default_publisher_options();
  rmw_publisher_t *publisher = rmw_create_publisher(
      node_, type_support, "/test_topic", &qos_profile, &pub_options);
  ASSERT_NE(nullptr, publisher);

  // Create subscription
  rmw_subscription_options_t sub_options = rmw_get_default_subscription_options();
  rmw_subscription_t *subscription = rmw_create_subscription(
      node_, type_support, "/test_topic", &qos_profile, &sub_options);
  ASSERT_NE(nullptr, subscription);

  // Publish a message
  std_msgs__msg__String msg;
  std_msgs__msg__String__init(&msg);
  msg.data.data = rcutils_strdup("Hello, World!", allocator_);
  msg.data.size = strlen(msg.data.data);
  msg.data.capacity = msg.data.size + 1;

  rmw_ret_t ret = rmw_publish(publisher, &msg, nullptr);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Try to take a message (may or may not succeed depending on timing)
  std_msgs__msg__String received_msg;
  std_msgs__msg__String__init(&received_msg);
  bool taken = false;
  ret = rmw_take(subscription, &received_msg, &taken, nullptr);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Cleanup
  std_msgs__msg__String__fini(&msg);
  std_msgs__msg__String__fini(&received_msg);

  ret = rmw_destroy_subscription(node_, subscription);
  EXPECT_EQ(RMW_RET_OK, ret);

  ret = rmw_destroy_publisher(node_, publisher);
  EXPECT_EQ(RMW_RET_OK, ret);
}

TEST_F(PublisherSubscriberIntermediateTest, GetActualQoS) {
  // Get type support
  const rosidl_message_type_support_t *type_support =
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String);
  ASSERT_NE(nullptr, type_support);

  // Create publisher
  rmw_qos_profile_t qos_profile = rmw_qos_profile_default;
  rmw_publisher_options_t pub_options = rmw_get_default_publisher_options();
  rmw_publisher_t *publisher = rmw_create_publisher(
      node_, type_support, "/test_topic", &qos_profile, &pub_options);
  ASSERT_NE(nullptr, publisher);

  // Get actual QoS
  rmw_qos_profile_t actual_qos;
  rmw_ret_t ret = rmw_publisher_get_actual_qos(publisher, &actual_qos);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Create subscription
  rmw_subscription_options_t sub_options = rmw_get_default_subscription_options();
  rmw_subscription_t *subscription = rmw_create_subscription(
      node_, type_support, "/test_topic", &qos_profile, &sub_options);
  ASSERT_NE(nullptr, subscription);

  // Get actual QoS
  ret = rmw_subscription_get_actual_qos(subscription, &actual_qos);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Cleanup
  ret = rmw_destroy_subscription(node_, subscription);
  EXPECT_EQ(RMW_RET_OK, ret);

  ret = rmw_destroy_publisher(node_, publisher);
  EXPECT_EQ(RMW_RET_OK, ret);
}

TEST_F(PublisherSubscriberIntermediateTest, CountMatched) {
  // Get type support
  const rosidl_message_type_support_t *type_support =
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String);
  ASSERT_NE(nullptr, type_support);

  // Create publisher
  rmw_qos_profile_t qos_profile = rmw_qos_profile_default;
  rmw_publisher_options_t pub_options = rmw_get_default_publisher_options();
  rmw_publisher_t *publisher = rmw_create_publisher(
      node_, type_support, "/test_topic", &qos_profile, &pub_options);
  ASSERT_NE(nullptr, publisher);

  // Create subscription
  rmw_subscription_options_t sub_options = rmw_get_default_subscription_options();
  rmw_subscription_t *subscription = rmw_create_subscription(
      node_, type_support, "/test_topic", &qos_profile, &sub_options);
  ASSERT_NE(nullptr, subscription);

  // Count matched subscriptions (should forward to real RMW)
  size_t sub_count = 0;
  rmw_ret_t ret = rmw_publisher_count_matched_subscriptions(publisher, &sub_count);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Count matched publishers (should forward to real RMW)
  size_t pub_count = 0;
  ret = rmw_subscription_count_matched_publishers(subscription, &pub_count);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Cleanup
  ret = rmw_destroy_subscription(node_, subscription);
  EXPECT_EQ(RMW_RET_OK, ret);

  ret = rmw_destroy_publisher(node_, publisher);
  EXPECT_EQ(RMW_RET_OK, ret);
}

TEST_F(PublisherSubscriberIntermediateTest, PublisherLiveliness) {
  // Get type support
  const rosidl_message_type_support_t *type_support =
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String);
  ASSERT_NE(nullptr, type_support);

  // Create publisher
  rmw_qos_profile_t qos_profile = rmw_qos_profile_default;
  rmw_publisher_options_t pub_options = rmw_get_default_publisher_options();
  rmw_publisher_t *publisher = rmw_create_publisher(
      node_, type_support, "/test_topic", &qos_profile, &pub_options);
  ASSERT_NE(nullptr, publisher);

  // Assert liveliness (should forward to real RMW)
  rmw_ret_t ret = rmw_publisher_assert_liveliness(publisher);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Cleanup
  ret = rmw_destroy_publisher(node_, publisher);
  EXPECT_EQ(RMW_RET_OK, ret);
}

TEST_F(PublisherSubscriberIntermediateTest, SerializedMessage) {
  // Get type support
  const rosidl_message_type_support_t *type_support =
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String);
  ASSERT_NE(nullptr, type_support);

  // Create publisher
  rmw_qos_profile_t qos_profile = rmw_qos_profile_default;
  rmw_publisher_options_t pub_options = rmw_get_default_publisher_options();
  rmw_publisher_t *publisher = rmw_create_publisher(
      node_, type_support, "/test_topic", &qos_profile, &pub_options);
  ASSERT_NE(nullptr, publisher);

  // Create subscription
  rmw_subscription_options_t sub_options = rmw_get_default_subscription_options();
  rmw_subscription_t *subscription = rmw_create_subscription(
      node_, type_support, "/test_topic", &qos_profile, &sub_options);
  ASSERT_NE(nullptr, subscription);

  // Create a serialized message
  rmw_serialized_message_t serialized_msg = rmw_get_zero_initialized_serialized_message();
  rmw_ret_t ret = rmw_serialized_message_init(&serialized_msg, 100, &allocator_);
  ASSERT_EQ(RMW_RET_OK, ret);

  // Publish serialized message
  ret = rmw_publish_serialized_message(publisher, &serialized_msg, nullptr);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Try to take serialized message
  rmw_serialized_message_t received_msg = rmw_get_zero_initialized_serialized_message();
  ret = rmw_serialized_message_init(&received_msg, 100, &allocator_);
  ASSERT_EQ(RMW_RET_OK, ret);

  bool taken = false;
  ret = rmw_take_serialized_message(subscription, &received_msg, &taken, nullptr);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Cleanup
  ret = rmw_serialized_message_fini(&serialized_msg);
  EXPECT_EQ(RMW_RET_OK, ret);
  ret = rmw_serialized_message_fini(&received_msg);
  EXPECT_EQ(RMW_RET_OK, ret);

  ret = rmw_destroy_subscription(node_, subscription);
  EXPECT_EQ(RMW_RET_OK, ret);

  ret = rmw_destroy_publisher(node_, publisher);
  EXPECT_EQ(RMW_RET_OK, ret);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
