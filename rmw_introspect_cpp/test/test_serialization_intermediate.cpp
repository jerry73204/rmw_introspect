#include <gtest/gtest.h>

#include <cstdlib>
#include <cstring>

#include "rcutils/allocator.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw_introspect/identifier.hpp"
#include "test_msgs/msg/basic_types.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"

class SerializationIntermediateTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Enable intermediate mode for these tests
    setenv("RMW_INTROSPECT_DELEGATE_TO", "rmw_fastrtps_cpp", 1);

    // Initialize context
    rmw_init_options_t init_options = rmw_get_zero_initialized_init_options();
    rmw_ret_t ret = rmw_init_options_init(&init_options, rcutils_get_default_allocator());
    ASSERT_EQ(RMW_RET_OK, ret);

    context = rmw_get_zero_initialized_context();
    ret = rmw_init(&init_options, &context);
    ASSERT_EQ(RMW_RET_OK, ret);

    ret = rmw_init_options_fini(&init_options);
    ASSERT_EQ(RMW_RET_OK, ret);

    // Create node
    node = rmw_create_node(&context, "test_node", "/test_namespace");
    ASSERT_NE(nullptr, node);
    ASSERT_STREQ(rmw_introspect_cpp_identifier,
                 node->implementation_identifier);

    // Get type support
    type_support = ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, BasicTypes);
    ASSERT_NE(nullptr, type_support);
  }

  void TearDown() override {
    if (node) {
      rmw_ret_t ret = rmw_destroy_node(node);
      EXPECT_EQ(RMW_RET_OK, ret);
    }

    rmw_ret_t ret = rmw_shutdown(&context);
    EXPECT_EQ(RMW_RET_OK, ret);

    ret = rmw_context_fini(&context);
    EXPECT_EQ(RMW_RET_OK, ret);

    unsetenv("RMW_INTROSPECT_DELEGATE_TO");
  }

  rmw_context_t context;
  rmw_node_t *node = nullptr;
  const rosidl_message_type_support_t *type_support = nullptr;
};

// Test serialization operations
TEST_F(SerializationIntermediateTest, TestSerializeDeserialize) {
  // Create a test message
  test_msgs__msg__BasicTypes input_msg;
  test_msgs__msg__BasicTypes__init(&input_msg);
  input_msg.int32_value = 42;
  input_msg.float32_value = 3.14f;

  // Get serialized message size
  size_t size = 0;
  rmw_ret_t ret =
      rmw_get_serialized_message_size(type_support, nullptr, &size);
  ASSERT_EQ(RMW_RET_OK, ret);
  EXPECT_GT(size, 0u);

  // Allocate serialized message
  rmw_serialized_message_t serialized_msg =
      rmw_get_zero_initialized_serialized_message();
  ret = rmw_serialized_message_init(&serialized_msg, size,
                                    &rcutils_get_default_allocator());
  ASSERT_EQ(RMW_RET_OK, ret);

  // Serialize the message
  ret = rmw_serialize(&input_msg, type_support, &serialized_msg);
  ASSERT_EQ(RMW_RET_OK, ret);
  EXPECT_GT(serialized_msg.buffer_length, 0u);

  // Deserialize the message
  test_msgs__msg__BasicTypes output_msg;
  test_msgs__msg__BasicTypes__init(&output_msg);
  ret = rmw_deserialize(&serialized_msg, type_support, &output_msg);
  ASSERT_EQ(RMW_RET_OK, ret);

  // Verify the deserialized values match
  EXPECT_EQ(input_msg.int32_value, output_msg.int32_value);
  EXPECT_FLOAT_EQ(input_msg.float32_value, output_msg.float32_value);

  // Cleanup
  test_msgs__msg__BasicTypes__fini(&input_msg);
  test_msgs__msg__BasicTypes__fini(&output_msg);
  ret = rmw_serialized_message_fini(&serialized_msg);
  EXPECT_EQ(RMW_RET_OK, ret);
}

// Test loaned message operations with publisher
TEST_F(SerializationIntermediateTest, TestPublisherLoanedMessage) {
  // Create a publisher
  rmw_publisher_options_t pub_options = rmw_get_default_publisher_options();
  rmw_publisher_t *publisher =
      rmw_create_publisher(node, type_support, "test_topic", &rmw_qos_profile_default,
                          &pub_options);
  ASSERT_NE(nullptr, publisher);

  // Try to borrow a loaned message
  // Note: This may return UNSUPPORTED if the underlying RMW doesn't support it
  void *loaned_msg = nullptr;
  rmw_ret_t ret =
      rmw_borrow_loaned_message(publisher, type_support, &loaned_msg);

  if (ret == RMW_RET_OK) {
    // If loaned messages are supported, we should have a valid pointer
    ASSERT_NE(nullptr, loaned_msg);

    // Return the loaned message
    ret = rmw_return_loaned_message_from_publisher(publisher, loaned_msg);
    EXPECT_EQ(RMW_RET_OK, ret);
  } else {
    // If not supported, that's also acceptable
    EXPECT_EQ(RMW_RET_UNSUPPORTED, ret);
  }

  // Cleanup
  ret = rmw_destroy_publisher(publisher);
  EXPECT_EQ(RMW_RET_OK, ret);
}

// Test loaned message operations with subscription
TEST_F(SerializationIntermediateTest, TestSubscriptionLoanedMessage) {
  // Create a subscription
  rmw_subscription_options_t sub_options =
      rmw_get_default_subscription_options();
  rmw_subscription_t *subscription = rmw_create_subscription(
      node, type_support, "test_topic", &rmw_qos_profile_default, &sub_options);
  ASSERT_NE(nullptr, subscription);

  // Try to take a loaned message
  // Note: This may return UNSUPPORTED if the underlying RMW doesn't support it
  void *loaned_msg = nullptr;
  bool taken = false;
  rmw_ret_t ret =
      rmw_take_loaned_message(subscription, &loaned_msg, &taken, nullptr);

  if (ret == RMW_RET_OK) {
    // If loaned messages are supported and a message was taken
    if (taken) {
      ASSERT_NE(nullptr, loaned_msg);

      // Return the loaned message
      ret = rmw_return_loaned_message_from_subscription(subscription,
                                                        loaned_msg);
      EXPECT_EQ(RMW_RET_OK, ret);
    } else {
      // No message available, which is fine
      EXPECT_FALSE(taken);
    }
  } else {
    // If not supported, that's also acceptable
    EXPECT_EQ(RMW_RET_UNSUPPORTED, ret);
    EXPECT_FALSE(taken);
  }

  // Cleanup
  ret = rmw_destroy_subscription(subscription);
  EXPECT_EQ(RMW_RET_OK, ret);
}

// Test loaned message with info
TEST_F(SerializationIntermediateTest, TestSubscriptionLoanedMessageWithInfo) {
  // Create a subscription
  rmw_subscription_options_t sub_options =
      rmw_get_default_subscription_options();
  rmw_subscription_t *subscription = rmw_create_subscription(
      node, type_support, "test_topic", &rmw_qos_profile_default, &sub_options);
  ASSERT_NE(nullptr, subscription);

  // Try to take a loaned message with info
  void *loaned_msg = nullptr;
  bool taken = false;
  rmw_message_info_t message_info = rmw_get_zero_initialized_message_info();
  rmw_ret_t ret = rmw_take_loaned_message_with_info(
      subscription, &loaned_msg, &taken, &message_info, nullptr);

  if (ret == RMW_RET_OK) {
    // If loaned messages are supported and a message was taken
    if (taken) {
      ASSERT_NE(nullptr, loaned_msg);

      // Return the loaned message
      ret = rmw_return_loaned_message_from_subscription(subscription,
                                                        loaned_msg);
      EXPECT_EQ(RMW_RET_OK, ret);
    } else {
      // No message available, which is fine
      EXPECT_FALSE(taken);
    }
  } else {
    // If not supported, that's also acceptable
    EXPECT_EQ(RMW_RET_UNSUPPORTED, ret);
    EXPECT_FALSE(taken);
  }

  // Cleanup
  ret = rmw_destroy_subscription(subscription);
  EXPECT_EQ(RMW_RET_OK, ret);
}

// Test serialization in recording-only mode
TEST_F(SerializationIntermediateTest, TestSerializationRecordingMode) {
  // Temporarily disable intermediate mode
  unsetenv("RMW_INTROSPECT_DELEGATE_TO");

  // Create a test message
  test_msgs__msg__BasicTypes input_msg;
  test_msgs__msg__BasicTypes__init(&input_msg);
  input_msg.int32_value = 42;

  // Get serialized message size (should return 0 in recording-only mode)
  size_t size = 0;
  rmw_ret_t ret =
      rmw_get_serialized_message_size(type_support, nullptr, &size);
  ASSERT_EQ(RMW_RET_OK, ret);
  EXPECT_EQ(0u, size);

  // Allocate serialized message
  rmw_serialized_message_t serialized_msg =
      rmw_get_zero_initialized_serialized_message();
  ret = rmw_serialized_message_init(&serialized_msg, 1024,
                                    &rcutils_get_default_allocator());
  ASSERT_EQ(RMW_RET_OK, ret);

  // Serialize (should be no-op in recording-only mode)
  ret = rmw_serialize(&input_msg, type_support, &serialized_msg);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Deserialize (should be no-op in recording-only mode)
  test_msgs__msg__BasicTypes output_msg;
  test_msgs__msg__BasicTypes__init(&output_msg);
  ret = rmw_deserialize(&serialized_msg, type_support, &output_msg);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Cleanup
  test_msgs__msg__BasicTypes__fini(&input_msg);
  test_msgs__msg__BasicTypes__fini(&output_msg);
  ret = rmw_serialized_message_fini(&serialized_msg);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Re-enable intermediate mode for cleanup
  setenv("RMW_INTROSPECT_DELEGATE_TO", "rmw_fastrtps_cpp", 1);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
