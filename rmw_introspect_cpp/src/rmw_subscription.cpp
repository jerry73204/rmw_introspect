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

// Create subscription
RMW_INTROSPECT_PUBLIC
rmw_subscription_t *rmw_create_subscription(
    const rmw_node_t *node, const rosidl_message_type_support_t *type_support,
    const char *topic_name, const rmw_qos_profile_t *qos_profile,
    const rmw_subscription_options_t *subscription_options) {
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

  // Record subscription info (for both modes)
  rmw_introspect::SubscriptionInfo info;
  info.node_name = node->name;
  info.node_namespace = node->namespace_;
  info.topic_name = topic_name;
  info.message_type = message_type;
  info.qos = rmw_introspect::QoSProfile::from_rmw(*qos_profile);
  info.timestamp = std::chrono::duration<double>(
                       std::chrono::system_clock::now().time_since_epoch())
                       .count();

  rmw_introspect::IntrospectionData::instance().record_subscription(info);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_node_t *real_node = unwrap_node(node);
    if (!real_node) {
      RMW_SET_ERROR_MSG("failed to unwrap node");
      return nullptr;
    }

    // Forward to real RMW
    rmw_subscription_t *real_subscription = g_real_rmw->create_subscription(
        real_node, type_support, topic_name, qos_profile, subscription_options);

    if (!real_subscription) {
      return nullptr;
    }

    // Create wrapper
    auto *wrapper = new (std::nothrow) rmw_introspect::SubscriptionWrapper(
        real_subscription, topic_name, message_type, *qos_profile);
    if (!wrapper) {
      g_real_rmw->destroy_subscription(real_node, real_subscription);
      RMW_SET_ERROR_MSG("failed to allocate subscription wrapper");
      return nullptr;
    }

    // Create our subscription structure
    rmw_subscription_t *subscription = new (std::nothrow) rmw_subscription_t;
    if (!subscription) {
      g_real_rmw->destroy_subscription(real_node, real_subscription);
      delete wrapper;
      RMW_SET_ERROR_MSG("failed to allocate subscription");
      return nullptr;
    }

    subscription->implementation_identifier = rmw_introspect_cpp_identifier;
    subscription->data = wrapper;
    subscription->topic_name = real_subscription->topic_name;
    subscription->options = *subscription_options;
    subscription->can_loan_messages = real_subscription->can_loan_messages;
    subscription->is_cft_enabled = real_subscription->is_cft_enabled;

    return subscription;
  }

  // Recording-only mode (existing behavior)
  rmw_subscription_t *subscription = new (std::nothrow) rmw_subscription_t;
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

  return subscription;
}

// Destroy subscription
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_destroy_subscription(rmw_node_t *node,
                                   rmw_subscription_t *subscription) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(subscription,
                                   subscription->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    auto *wrapper =
        static_cast<rmw_introspect::SubscriptionWrapper *>(subscription->data);
    if (wrapper && wrapper->real_subscription) {
      rmw_node_t *real_node = unwrap_node(node);
      if (!real_node) {
        RMW_SET_ERROR_MSG("failed to unwrap node");
        return RMW_RET_ERROR;
      }
      rmw_ret_t ret = g_real_rmw->destroy_subscription(
          real_node, wrapper->real_subscription);
      if (ret != RMW_RET_OK) {
        return ret;
      }
    }
    delete wrapper;
    delete subscription;
    return RMW_RET_OK;
  }

  // Recording-only mode
  delete subscription;
  return RMW_RET_OK;
}

// Take message
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_take(const rmw_subscription_t *subscription, void *ros_message,
                   bool *taken, rmw_subscription_allocation_t *allocation) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(ros_message, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(taken, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_subscription_t *real_subscription = unwrap_subscription(subscription);
    if (!real_subscription) {
      RMW_SET_ERROR_MSG("failed to unwrap subscription");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->take(real_subscription, ros_message, taken, allocation);
  }

  // Recording-only mode: no-op
  *taken = false;
  return RMW_RET_OK;
}

// Take message with info
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_take_with_info(const rmw_subscription_t *subscription,
                             void *ros_message, bool *taken,
                             rmw_message_info_t *message_info,
                             rmw_subscription_allocation_t *allocation) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(ros_message, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(taken, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(message_info, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_subscription_t *real_subscription = unwrap_subscription(subscription);
    if (!real_subscription) {
      RMW_SET_ERROR_MSG("failed to unwrap subscription");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->take_with_info(real_subscription, ros_message, taken,
                                      message_info, allocation);
  }

  // Recording-only mode: no-op
  *taken = false;
  return RMW_RET_OK;
}

// Take serialized message
RMW_INTROSPECT_PUBLIC
rmw_ret_t
rmw_take_serialized_message(const rmw_subscription_t *subscription,
                            rmw_serialized_message_t *serialized_message,
                            bool *taken,
                            rmw_subscription_allocation_t *allocation) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(serialized_message, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(taken, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_subscription_t *real_subscription = unwrap_subscription(subscription);
    if (!real_subscription) {
      RMW_SET_ERROR_MSG("failed to unwrap subscription");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->take_serialized_message(
        real_subscription, serialized_message, taken, allocation);
  }

  // Recording-only mode: no-op
  *taken = false;
  return RMW_RET_OK;
}

// Take serialized message with info
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_take_serialized_message_with_info(
    const rmw_subscription_t *subscription,
    rmw_serialized_message_t *serialized_message, bool *taken,
    rmw_message_info_t *message_info,
    rmw_subscription_allocation_t *allocation) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(serialized_message, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(taken, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(message_info, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_subscription_t *real_subscription = unwrap_subscription(subscription);
    if (!real_subscription) {
      RMW_SET_ERROR_MSG("failed to unwrap subscription");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->take_serialized_message_with_info(
        real_subscription, serialized_message, taken, message_info, allocation);
  }

  // Recording-only mode: no-op
  *taken = false;
  return RMW_RET_OK;
}

// Take loaned message (unsupported)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_take_loaned_message(const rmw_subscription_t *subscription,
                                  void **loaned_message, bool *taken,
                                  rmw_subscription_allocation_t *allocation) {
  (void)subscription;
  (void)loaned_message;
  (void)taken;
  (void)allocation;
  return RMW_RET_UNSUPPORTED;
}

// Take loaned message with info (unsupported)
RMW_INTROSPECT_PUBLIC
rmw_ret_t
rmw_take_loaned_message_with_info(const rmw_subscription_t *subscription,
                                  void **loaned_message, bool *taken,
                                  rmw_message_info_t *message_info,
                                  rmw_subscription_allocation_t *allocation) {
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
    const rmw_subscription_t *subscription, void *loaned_message) {
  (void)subscription;
  (void)loaned_message;
  return RMW_RET_UNSUPPORTED;
}

// Get subscription actual QoS
RMW_INTROSPECT_PUBLIC
rmw_ret_t
rmw_subscription_get_actual_qos(const rmw_subscription_t *subscription,
                                rmw_qos_profile_t *qos) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(qos, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_subscription_t *real_subscription = unwrap_subscription(subscription);
    if (!real_subscription) {
      RMW_SET_ERROR_MSG("failed to unwrap subscription");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->subscription_get_actual_qos(real_subscription, qos);
  }

  // Recording-only mode: return default
  *qos = rmw_qos_profile_default;
  return RMW_RET_OK;
}

// Subscription count matched publishers
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_subscription_count_matched_publishers(
    const rmw_subscription_t *subscription, size_t *publisher_count) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(publisher_count, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_subscription_t *real_subscription = unwrap_subscription(subscription);
    if (!real_subscription) {
      RMW_SET_ERROR_MSG("failed to unwrap subscription");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->subscription_count_matched_publishers(real_subscription,
                                                             publisher_count);
  }

  // Recording-only mode: return 0
  *publisher_count = 0;
  return RMW_RET_OK;
}

// Set content filter (unsupported)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_subscription_set_content_filter(
    rmw_subscription_t *subscription,
    const rmw_subscription_content_filter_options_t *options) {
  (void)subscription;
  (void)options;
  return RMW_RET_UNSUPPORTED;
}

// Get content filter (unsupported)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_subscription_get_content_filter(
    const rmw_subscription_t *subscription, rcutils_allocator_t *allocator,
    rmw_subscription_content_filter_options_t *options) {
  (void)subscription;
  (void)allocator;
  (void)options;
  return RMW_RET_UNSUPPORTED;
}

} // extern "C"
