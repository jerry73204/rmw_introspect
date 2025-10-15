#include "rmw/rmw.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rcutils/macros.h"
#include "rmw_introspect/identifier.hpp"
#include "rmw_introspect/visibility_control.h"
#include "rmw_introspect/data.hpp"
#include "rmw_introspect/types.hpp"
#include "rmw_introspect/type_support.hpp"
#include <new>
#include <chrono>

extern "C"
{

// Create subscription
RMW_INTROSPECT_PUBLIC
rmw_subscription_t * rmw_create_subscription(
  const rmw_node_t * node,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name,
  const rmw_qos_profile_t * qos_profile,
  const rmw_subscription_options_t * subscription_options)
{
  (void)subscription_options;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node, nullptr);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(type_support, nullptr);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(topic_name, nullptr);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(qos_profile, nullptr);

  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node,
    node->implementation_identifier,
    rmw_introspect_cpp_identifier,
    return nullptr);

  // Allocate subscription structure
  rmw_subscription_t * subscription = new (std::nothrow) rmw_subscription_t;
  if (!subscription) {
    RMW_SET_ERROR_MSG("failed to allocate subscription");
    return nullptr;
  }

  subscription->implementation_identifier = rmw_introspect_cpp_identifier;
  subscription->data = nullptr;
  subscription->topic_name = topic_name;
  subscription->options = *subscription_options;
  subscription->can_loan_messages = false;
  subscription->is_cft_enabled = false;

  // Extract message type
  std::string message_type = rmw_introspect::extract_message_type(type_support);

  // Record subscription info
  rmw_introspect::SubscriptionInfo info;
  info.node_name = node->name;
  info.node_namespace = node->namespace_;
  info.topic_name = topic_name;
  info.message_type = message_type;
  info.qos = rmw_introspect::QoSProfile::from_rmw(*qos_profile);
  info.timestamp = std::chrono::duration<double>(
    std::chrono::system_clock::now().time_since_epoch()).count();

  rmw_introspect::IntrospectionData::instance().record_subscription(info);

  return subscription;
}

// Destroy subscription
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_destroy_subscription(rmw_node_t * node, rmw_subscription_t * subscription)
{
  (void)node;
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    subscription,
    subscription->implementation_identifier,
    rmw_introspect_cpp_identifier,
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  delete subscription;
  return RMW_RET_OK;
}

// Take message (no-op)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_take(
  const rmw_subscription_t * subscription,
  void * ros_message,
  bool * taken,
  rmw_subscription_allocation_t * allocation)
{
  (void)subscription;
  (void)ros_message;
  (void)allocation;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(taken, RMW_RET_INVALID_ARGUMENT);
  *taken = false;
  return RMW_RET_OK;
}

// Take message with info (no-op)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_take_with_info(
  const rmw_subscription_t * subscription,
  void * ros_message,
  bool * taken,
  rmw_message_info_t * message_info,
  rmw_subscription_allocation_t * allocation)
{
  (void)subscription;
  (void)ros_message;
  (void)message_info;
  (void)allocation;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(taken, RMW_RET_INVALID_ARGUMENT);
  *taken = false;
  return RMW_RET_OK;
}

// Take serialized message (no-op)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_take_serialized_message(
  const rmw_subscription_t * subscription,
  rmw_serialized_message_t * serialized_message,
  bool * taken,
  rmw_subscription_allocation_t * allocation)
{
  (void)subscription;
  (void)serialized_message;
  (void)allocation;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(taken, RMW_RET_INVALID_ARGUMENT);
  *taken = false;
  return RMW_RET_OK;
}

// Take serialized message with info (no-op)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_take_serialized_message_with_info(
  const rmw_subscription_t * subscription,
  rmw_serialized_message_t * serialized_message,
  bool * taken,
  rmw_message_info_t * message_info,
  rmw_subscription_allocation_t * allocation)
{
  (void)subscription;
  (void)serialized_message;
  (void)message_info;
  (void)allocation;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(taken, RMW_RET_INVALID_ARGUMENT);
  *taken = false;
  return RMW_RET_OK;
}

// Take loaned message (unsupported)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_take_loaned_message(
  const rmw_subscription_t * subscription,
  void ** loaned_message,
  bool * taken,
  rmw_subscription_allocation_t * allocation)
{
  (void)subscription;
  (void)loaned_message;
  (void)taken;
  (void)allocation;
  return RMW_RET_UNSUPPORTED;
}

// Take loaned message with info (unsupported)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_take_loaned_message_with_info(
  const rmw_subscription_t * subscription,
  void ** loaned_message,
  bool * taken,
  rmw_message_info_t * message_info,
  rmw_subscription_allocation_t * allocation)
{
  (void)subscription;
  (void)loaned_message;
  (void)taken;
  (void)message_info;
  (void)allocation;
  return RMW_RET_UNSUPPORTED;
}

// Return loaned message (unsupported)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_return_loaned_message_from_subscription(
  const rmw_subscription_t * subscription,
  void * loaned_message)
{
  (void)subscription;
  (void)loaned_message;
  return RMW_RET_UNSUPPORTED;
}

// Get subscription actual QoS (stub)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_subscription_get_actual_qos(
  const rmw_subscription_t * subscription,
  rmw_qos_profile_t * qos)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(qos, RMW_RET_INVALID_ARGUMENT);

  *qos = rmw_qos_profile_default;
  return RMW_RET_OK;
}

// Subscription count matched publishers (stub)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_subscription_count_matched_publishers(
  const rmw_subscription_t * subscription,
  size_t * publisher_count)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(publisher_count, RMW_RET_INVALID_ARGUMENT);

  *publisher_count = 0;
  return RMW_RET_OK;
}

// Set content filter (unsupported)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_subscription_set_content_filter(
  rmw_subscription_t * subscription,
  const rmw_subscription_content_filter_options_t * options)
{
  (void)subscription;
  (void)options;
  return RMW_RET_UNSUPPORTED;
}

// Get content filter (unsupported)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_subscription_get_content_filter(
  const rmw_subscription_t * subscription,
  rcutils_allocator_t * allocator,
  rmw_subscription_content_filter_options_t * options)
{
  (void)subscription;
  (void)allocator;
  (void)options;
  return RMW_RET_UNSUPPORTED;
}

}  // extern "C"
