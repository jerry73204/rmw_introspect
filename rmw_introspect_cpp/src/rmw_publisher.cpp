#include "rcutils/macros.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw_introspect/data.hpp"
#include "rmw_introspect/forwarding.hpp"
#include "rmw_introspect/identifier.hpp"
#include "rmw_introspect/mode.hpp"
#include "rmw_introspect/type_support.hpp"
#include "rmw_introspect/types.hpp"
#include "rmw_introspect/visibility_control.h"
#include "rmw_introspect/wrappers.hpp"
#include <chrono>
#include <new>

extern "C" {

// Create publisher
RMW_INTROSPECT_PUBLIC
rmw_publisher_t *rmw_create_publisher(
    const rmw_node_t *node, const rosidl_message_type_support_t *type_support,
    const char *topic_name, const rmw_qos_profile_t *qos_profile,
    const rmw_publisher_options_t *publisher_options) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node, nullptr);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(type_support, nullptr);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(topic_name, nullptr);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(qos_profile, nullptr);

  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(node, node->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return nullptr);

  // Extract message type
  std::string message_type = rmw_introspect::extract_message_type(type_support);

  // Record publisher info (for both modes)
  rmw_introspect::PublisherInfo info;
  info.node_name = node->name;
  info.node_namespace = node->namespace_;
  info.topic_name = topic_name;
  info.message_type = message_type;
  info.qos = rmw_introspect::QoSProfile::from_rmw(*qos_profile);
  info.timestamp = std::chrono::duration<double>(
                       std::chrono::system_clock::now().time_since_epoch())
                       .count();

  rmw_introspect::IntrospectionData::instance().record_publisher(info);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_node_t *real_node = unwrap_node(node);
    if (!real_node) {
      RMW_SET_ERROR_MSG("failed to unwrap node");
      return nullptr;
    }

    // Forward to real RMW
    rmw_publisher_t *real_publisher = g_real_rmw->create_publisher(
        real_node, type_support, topic_name, qos_profile, publisher_options);

    if (!real_publisher) {
      return nullptr;
    }

    // Create wrapper
    auto *wrapper = new (std::nothrow) rmw_introspect::PublisherWrapper(
        real_publisher, topic_name, message_type, *qos_profile);
    if (!wrapper) {
      g_real_rmw->destroy_publisher(real_node, real_publisher);
      RMW_SET_ERROR_MSG("failed to allocate publisher wrapper");
      return nullptr;
    }

    // Create our publisher structure
    rmw_publisher_t *publisher = new (std::nothrow) rmw_publisher_t;
    if (!publisher) {
      g_real_rmw->destroy_publisher(real_node, real_publisher);
      delete wrapper;
      RMW_SET_ERROR_MSG("failed to allocate publisher");
      return nullptr;
    }

    publisher->implementation_identifier = rmw_introspect_cpp_identifier;
    publisher->data = wrapper;
    publisher->topic_name = real_publisher->topic_name;
    publisher->options = *publisher_options;
    publisher->can_loan_messages = real_publisher->can_loan_messages;

    return publisher;
  }

  // Recording-only mode (existing behavior)
  rmw_publisher_t *publisher = new (std::nothrow) rmw_publisher_t;
  if (!publisher) {
    RMW_SET_ERROR_MSG("failed to allocate publisher");
    return nullptr;
  }

  publisher->implementation_identifier = rmw_introspect_cpp_identifier;
  publisher->data = nullptr;
  publisher->topic_name = topic_name;
  publisher->options = *publisher_options;
  publisher->can_loan_messages = false;

  return publisher;
}

// Destroy publisher
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_destroy_publisher(rmw_node_t *node, rmw_publisher_t *publisher) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(publisher,
                                   publisher->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    auto *wrapper =
        static_cast<rmw_introspect::PublisherWrapper *>(publisher->data);
    if (wrapper && wrapper->real_publisher) {
      rmw_node_t *real_node = unwrap_node(node);
      if (!real_node) {
        RMW_SET_ERROR_MSG("failed to unwrap node");
        return RMW_RET_ERROR;
      }
      rmw_ret_t ret =
          g_real_rmw->destroy_publisher(real_node, wrapper->real_publisher);
      if (ret != RMW_RET_OK) {
        return ret;
      }
    }
    delete wrapper;
    delete publisher;
    return RMW_RET_OK;
  }

  // Recording-only mode
  delete publisher;
  return RMW_RET_OK;
}

// Publish
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_publish(const rmw_publisher_t *publisher, const void *ros_message,
                      rmw_publisher_allocation_t *allocation) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(ros_message, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_publisher_t *real_publisher = unwrap_publisher(publisher);
    if (!real_publisher) {
      RMW_SET_ERROR_MSG("failed to unwrap publisher");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->publish(real_publisher, ros_message, allocation);
  }

  // Recording-only mode: no-op
  return RMW_RET_OK;
}

// Publish serialized message
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_publish_serialized_message(
    const rmw_publisher_t *publisher,
    const rmw_serialized_message_t *serialized_message,
    rmw_publisher_allocation_t *allocation) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(serialized_message, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_publisher_t *real_publisher = unwrap_publisher(publisher);
    if (!real_publisher) {
      RMW_SET_ERROR_MSG("failed to unwrap publisher");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->publish_serialized_message(
        real_publisher, serialized_message, allocation);
  }

  // Recording-only mode: no-op
  return RMW_RET_OK;
}

// Borrow loaned message
RMW_INTROSPECT_PUBLIC
rmw_ret_t
rmw_borrow_loaned_message(const rmw_publisher_t *publisher,
                          const rosidl_message_type_support_t *type_support,
                          void **ros_message) {
  (void)publisher;
  (void)type_support;
  (void)ros_message;
  return RMW_RET_UNSUPPORTED;
}

// Return loaned message
RMW_INTROSPECT_PUBLIC
rmw_ret_t
rmw_return_loaned_message_from_publisher(const rmw_publisher_t *publisher,
                                         void *loaned_message) {
  (void)publisher;
  (void)loaned_message;
  return RMW_RET_UNSUPPORTED;
}

// Get publisher actual QoS
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_publisher_get_actual_qos(const rmw_publisher_t *publisher,
                                       rmw_qos_profile_t *qos) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(qos, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_publisher_t *real_publisher = unwrap_publisher(publisher);
    if (!real_publisher) {
      RMW_SET_ERROR_MSG("failed to unwrap publisher");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->publisher_get_actual_qos(real_publisher, qos);
  }

  // Recording-only mode: return default
  *qos = rmw_qos_profile_default;
  return RMW_RET_OK;
}

// Publisher count matched subscriptions
RMW_INTROSPECT_PUBLIC
rmw_ret_t
rmw_publisher_count_matched_subscriptions(const rmw_publisher_t *publisher,
                                          size_t *subscription_count) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(subscription_count, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_publisher_t *real_publisher = unwrap_publisher(publisher);
    if (!real_publisher) {
      RMW_SET_ERROR_MSG("failed to unwrap publisher");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->publisher_count_matched_subscriptions(
        real_publisher, subscription_count);
  }

  // Recording-only mode: return 0
  *subscription_count = 0;
  return RMW_RET_OK;
}

// Publisher assert liveliness
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_publisher_assert_liveliness(const rmw_publisher_t *publisher) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_publisher_t *real_publisher = unwrap_publisher(publisher);
    if (!real_publisher) {
      RMW_SET_ERROR_MSG("failed to unwrap publisher");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->publisher_assert_liveliness(real_publisher);
  }

  // Recording-only mode: no-op
  return RMW_RET_OK;
}

// Wait for all acked
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_publisher_wait_for_all_acked(const rmw_publisher_t *publisher,
                                           rmw_time_t wait_timeout) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(publisher,
                                   publisher->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_publisher_t *real_publisher = unwrap_publisher(publisher);
    if (!real_publisher) {
      RMW_SET_ERROR_MSG("failed to unwrap publisher");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->publisher_wait_for_all_acked(real_publisher,
                                                    wait_timeout);
  }

  // Recording-only mode: return immediately
  return RMW_RET_OK;
}

} // extern "C"
