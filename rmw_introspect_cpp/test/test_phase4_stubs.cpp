#include <gtest/gtest.h>
#include <cstring>

#include "rmw/rmw.h"
#include "rmw/error_handling.h"
#include "rmw/features.h"
#include "rmw/get_topic_endpoint_info.h"
#include "rmw/topic_endpoint_info_array.h"
#include "rmw/names_and_types.h"
#include "rmw_introspect/identifier.hpp"

#include "std_msgs/msg/string.hpp"
#include "std_srvs/srv/empty.hpp"
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_cpp/service_type_support.hpp"

// Test fixture for Phase 4 stub functions
class Phase4StubTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    context_ = rmw_get_zero_initialized_context();

    rmw_init_options_t init_options = rmw_get_zero_initialized_init_options();
    rmw_ret_t ret = rmw_init_options_init(&init_options, rcutils_get_default_allocator());
    ASSERT_EQ(RMW_RET_OK, ret);

    ret = rmw_init(&init_options, &context_);
    ASSERT_EQ(RMW_RET_OK, ret);

    ret = rmw_init_options_fini(&init_options);
    ASSERT_EQ(RMW_RET_OK, ret);

    node_ = rmw_create_node(&context_, "test_node", "/test_namespace");
    ASSERT_NE(nullptr, node_);
  }

  void TearDown() override
  {
    if (node_) {
      rmw_ret_t ret = rmw_destroy_node(node_);
      EXPECT_EQ(RMW_RET_OK, ret);
    }

    if (context_.impl) {
      rmw_ret_t ret = rmw_shutdown(&context_);
      EXPECT_EQ(RMW_RET_OK, ret);
      ret = rmw_context_fini(&context_);
      EXPECT_EQ(RMW_RET_OK, ret);
    }
  }

  rmw_context_t context_;
  rmw_node_t * node_{nullptr};
};

// ============================================================================
// GID Functions Tests (rmw_gid.cpp) - NEW
// ============================================================================

TEST_F(Phase4StubTest, GetGidForPublisher)
{
  // Create publisher
  auto type_support = rosidl_typesupport_cpp::get_message_type_support_handle<std_msgs::msg::String>();
  rmw_publisher_options_t pub_options = rmw_get_default_publisher_options();
  auto publisher = rmw_create_publisher(
    node_,
    type_support,
    "test_topic",
    &rmw_qos_profile_default,
    &pub_options);
  ASSERT_NE(nullptr, publisher);

  // Get GID
  rmw_gid_t gid;
  rmw_ret_t ret = rmw_get_gid_for_publisher(publisher, &gid);
  EXPECT_EQ(RMW_RET_OK, ret);
  EXPECT_EQ(rmw_introspect_cpp_identifier, gid.implementation_identifier);

  // Verify GID is all zeros
  bool all_zeros = true;
  for (size_t i = 0; i < RMW_GID_STORAGE_SIZE; ++i) {
    if (gid.data[i] != 0) {
      all_zeros = false;
      break;
    }
  }
  EXPECT_TRUE(all_zeros);

  // Cleanup
  ret = rmw_destroy_publisher(node_, publisher);
  EXPECT_EQ(RMW_RET_OK, ret);
}

TEST_F(Phase4StubTest, CompareGidsEqual)
{
  rmw_gid_t gid1, gid2;
  gid1.implementation_identifier = rmw_introspect_cpp_identifier;
  gid2.implementation_identifier = rmw_introspect_cpp_identifier;
  memset(gid1.data, 0, RMW_GID_STORAGE_SIZE);
  memset(gid2.data, 0, RMW_GID_STORAGE_SIZE);

  bool are_equal = false;
  rmw_ret_t ret = rmw_compare_gids_equal(&gid1, &gid2, &are_equal);
  EXPECT_EQ(RMW_RET_OK, ret);
  EXPECT_TRUE(are_equal);  // All stub GIDs are equal
}

// ============================================================================
// QoS Compatibility Functions Tests (rmw_qos_compat.cpp) - NEW
// ============================================================================

TEST_F(Phase4StubTest, QosProfileCheckCompatible)
{
  rmw_qos_profile_t pub_profile = rmw_qos_profile_default;
  rmw_qos_profile_t sub_profile = rmw_qos_profile_sensor_data;

  rmw_qos_compatibility_type_t compatibility;
  rmw_ret_t ret = rmw_qos_profile_check_compatible(
    pub_profile, sub_profile, &compatibility, nullptr, 0);

  EXPECT_EQ(RMW_RET_OK, ret);
  EXPECT_EQ(RMW_QOS_COMPATIBILITY_OK, compatibility);
}

// ============================================================================
// Info Functions Tests (rmw_info.cpp) - NEW
// ============================================================================

TEST_F(Phase4StubTest, GetPublishersInfoByTopic)
{
  rmw_topic_endpoint_info_array_t publishers_info =
    rmw_get_zero_initialized_topic_endpoint_info_array();

  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rmw_ret_t ret = rmw_get_publishers_info_by_topic(
    node_, &allocator, "test_topic", false, &publishers_info);

  EXPECT_EQ(RMW_RET_OK, ret);
  EXPECT_EQ(0u, publishers_info.size);  // Returns empty array in introspection

  ret = rmw_topic_endpoint_info_array_fini(&publishers_info, &allocator);
  EXPECT_EQ(RMW_RET_OK, ret);
}

TEST_F(Phase4StubTest, GetSubscriptionsInfoByTopic)
{
  rmw_topic_endpoint_info_array_t subscriptions_info =
    rmw_get_zero_initialized_topic_endpoint_info_array();

  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rmw_ret_t ret = rmw_get_subscriptions_info_by_topic(
    node_, &allocator, "test_topic", false, &subscriptions_info);

  EXPECT_EQ(RMW_RET_OK, ret);
  EXPECT_EQ(0u, subscriptions_info.size);

  ret = rmw_topic_endpoint_info_array_fini(&subscriptions_info, &allocator);
  EXPECT_EQ(RMW_RET_OK, ret);
}

// ============================================================================
// Allocation Functions Tests (rmw_allocation.cpp) - NEW
// ============================================================================

TEST_F(Phase4StubTest, InitPublisherAllocation)
{
  rmw_publisher_allocation_t allocation;
  rmw_ret_t ret = rmw_init_publisher_allocation(nullptr, nullptr, &allocation);
  EXPECT_EQ(RMW_RET_UNSUPPORTED, ret);
}

TEST_F(Phase4StubTest, FiniPublisherAllocation)
{
  rmw_publisher_allocation_t allocation;
  rmw_ret_t ret = rmw_fini_publisher_allocation(&allocation);
  EXPECT_EQ(RMW_RET_UNSUPPORTED, ret);
}

TEST_F(Phase4StubTest, InitSubscriptionAllocation)
{
  rmw_subscription_allocation_t allocation;
  rmw_ret_t ret = rmw_init_subscription_allocation(nullptr, nullptr, &allocation);
  EXPECT_EQ(RMW_RET_UNSUPPORTED, ret);
}

TEST_F(Phase4StubTest, FiniSubscriptionAllocation)
{
  rmw_subscription_allocation_t allocation;
  rmw_ret_t ret = rmw_fini_subscription_allocation(&allocation);
  EXPECT_EQ(RMW_RET_UNSUPPORTED, ret);
}

TEST_F(Phase4StubTest, PublishLoanedMessage)
{
  rmw_ret_t ret = rmw_publish_loaned_message(nullptr, nullptr, nullptr);
  EXPECT_EQ(RMW_RET_UNSUPPORTED, ret);
}

// ============================================================================
// Callback Functions Tests (rmw_callbacks.cpp) - NEW
// ============================================================================

TEST_F(Phase4StubTest, SubscriptionSetOnNewMessageCallback)
{
  // Create subscription
  auto type_support = rosidl_typesupport_cpp::get_message_type_support_handle<std_msgs::msg::String>();
  rmw_subscription_options_t sub_options = rmw_get_default_subscription_options();
  auto subscription = rmw_create_subscription(
    node_,
    type_support,
    "test_topic",
    &rmw_qos_profile_default,
    &sub_options);
  ASSERT_NE(nullptr, subscription);

  // Set callback (should accept without error)
  auto callback = [](const void *, size_t) {};
  rmw_ret_t ret = rmw_subscription_set_on_new_message_callback(
    subscription, callback, nullptr);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Cleanup
  ret = rmw_destroy_subscription(node_, subscription);
  EXPECT_EQ(RMW_RET_OK, ret);
}

TEST_F(Phase4StubTest, ServiceSetOnNewRequestCallback)
{
  // Create service
  auto type_support = rosidl_typesupport_cpp::get_service_type_support_handle<std_srvs::srv::Empty>();
  auto service = rmw_create_service(
    node_,
    type_support,
    "test_service",
    &rmw_qos_profile_default);
  ASSERT_NE(nullptr, service);

  // Set callback
  auto callback = [](const void *, size_t) {};
  rmw_ret_t ret = rmw_service_set_on_new_request_callback(
    service, callback, nullptr);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Cleanup
  ret = rmw_destroy_service(node_, service);
  EXPECT_EQ(RMW_RET_OK, ret);
}

TEST_F(Phase4StubTest, ClientSetOnNewResponseCallback)
{
  // Create client
  auto type_support = rosidl_typesupport_cpp::get_service_type_support_handle<std_srvs::srv::Empty>();
  auto client = rmw_create_client(
    node_,
    type_support,
    "test_service",
    &rmw_qos_profile_default);
  ASSERT_NE(nullptr, client);

  // Set callback
  auto callback = [](const void *, size_t) {};
  rmw_ret_t ret = rmw_client_set_on_new_response_callback(
    client, callback, nullptr);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Cleanup
  ret = rmw_destroy_client(node_, client);
  EXPECT_EQ(RMW_RET_OK, ret);
}

// ============================================================================
// Feature Functions Tests (rmw_features.cpp) - NEW
// ============================================================================

TEST_F(Phase4StubTest, FeatureSupported)
{
  bool is_supported = rmw_feature_supported(RMW_FEATURE_MESSAGE_INFO_PUBLICATION_SEQUENCE_NUMBER);
  EXPECT_FALSE(is_supported);  // All features unsupported in introspection
}
