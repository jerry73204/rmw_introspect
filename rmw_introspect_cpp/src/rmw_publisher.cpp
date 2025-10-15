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

// Create publisher
RMW_INTROSPECT_PUBLIC
rmw_publisher_t * rmw_create_publisher(
  const rmw_node_t * node,
  const rosidl_message_type_support_t * type_support,
  const char * topic_name,
  const rmw_qos_profile_t * qos_profile,
  const rmw_publisher_options_t * publisher_options)
{
  (void)publisher_options;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node, nullptr);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(type_support, nullptr);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(topic_name, nullptr);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(qos_profile, nullptr);

  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node,
    node->implementation_identifier,
    rmw_introspect_cpp_identifier,
    return nullptr);

  // Allocate publisher structure
  rmw_publisher_t * publisher = new (std::nothrow) rmw_publisher_t;
  if (!publisher) {
    RMW_SET_ERROR_MSG("failed to allocate publisher");
    return nullptr;
  }

  publisher->implementation_identifier = rmw_introspect_cpp_identifier;
  publisher->data = nullptr;
  publisher->topic_name = topic_name;
  publisher->options = *publisher_options;
  publisher->can_loan_messages = false;

  // Extract message type
  std::string message_type = rmw_introspect::extract_message_type(type_support);

  // Record publisher info
  rmw_introspect::PublisherInfo info;
  info.node_name = node->name;
  info.node_namespace = node->namespace_;
  info.topic_name = topic_name;
  info.message_type = message_type;
  info.qos = rmw_introspect::QoSProfile::from_rmw(*qos_profile);
  info.timestamp = std::chrono::duration<double>(
    std::chrono::system_clock::now().time_since_epoch()).count();

  rmw_introspect::IntrospectionData::instance().record_publisher(info);

  return publisher;
}

// Destroy publisher
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_destroy_publisher(rmw_node_t * node, rmw_publisher_t * publisher)
{
  (void)node;
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    publisher,
    publisher->implementation_identifier,
    rmw_introspect_cpp_identifier,
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  delete publisher;
  return RMW_RET_OK;
}

// Publish (no-op)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_publish(
  const rmw_publisher_t * publisher,
  const void * ros_message,
  rmw_publisher_allocation_t * allocation)
{
  (void)publisher;
  (void)ros_message;
  (void)allocation;
  return RMW_RET_OK;
}

// Publish serialized message (no-op)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_publish_serialized_message(
  const rmw_publisher_t * publisher,
  const rmw_serialized_message_t * serialized_message,
  rmw_publisher_allocation_t * allocation)
{
  (void)publisher;
  (void)serialized_message;
  (void)allocation;
  return RMW_RET_OK;
}

// Borrow loaned message (unsupported)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_borrow_loaned_message(
  const rmw_publisher_t * publisher,
  const rosidl_message_type_support_t * type_support,
  void ** ros_message)
{
  (void)publisher;
  (void)type_support;
  (void)ros_message;
  return RMW_RET_UNSUPPORTED;
}

// Return loaned message (unsupported)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_return_loaned_message_from_publisher(
  const rmw_publisher_t * publisher,
  void * loaned_message)
{
  (void)publisher;
  (void)loaned_message;
  return RMW_RET_UNSUPPORTED;
}

// Get publisher actual QoS (stub)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_publisher_get_actual_qos(
  const rmw_publisher_t * publisher,
  rmw_qos_profile_t * qos)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(qos, RMW_RET_INVALID_ARGUMENT);

  *qos = rmw_qos_profile_default;
  return RMW_RET_OK;
}

// Publisher count matched subscriptions (stub)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_publisher_count_matched_subscriptions(
  const rmw_publisher_t * publisher,
  size_t * subscription_count)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(subscription_count, RMW_RET_INVALID_ARGUMENT);

  *subscription_count = 0;
  return RMW_RET_OK;
}

// Publisher assert liveliness (no-op)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_publisher_assert_liveliness(const rmw_publisher_t * publisher)
{
  (void)publisher;
  return RMW_RET_OK;
}

// Wait for all acked (stub - returns immediately)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_publisher_wait_for_all_acked(
  const rmw_publisher_t * publisher,
  rmw_time_t wait_timeout)
{
  (void)wait_timeout;
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    publisher,
    publisher->implementation_identifier,
    rmw_introspect_cpp_identifier,
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // In introspection mode, we don't actually publish messages,
  // so we can return OK immediately
  return RMW_RET_OK;
}

}  // extern "C"
