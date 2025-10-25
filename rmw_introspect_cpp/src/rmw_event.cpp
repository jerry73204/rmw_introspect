#include "rmw/error_handling.h"
#include "rmw/event.h"
#include "rmw/rmw.h"
#include "rmw_introspect/identifier.hpp"
#include <cstring>

extern "C" {

// QoS event stubs - minimal implementation
// Events are not needed for introspection

rmw_ret_t rmw_publisher_event_init(rmw_event_t *rmw_event,
                                   const rmw_publisher_t *publisher,
                                   rmw_event_type_t event_type) {
  (void)rmw_event;
  (void)publisher;
  (void)event_type;

  // Events are not supported in introspection mode
  // rclcpp will handle RMW_RET_UNSUPPORTED gracefully
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t rmw_subscription_event_init(rmw_event_t *rmw_event,
                                      const rmw_subscription_t *subscription,
                                      rmw_event_type_t event_type) {
  (void)rmw_event;
  (void)subscription;
  (void)event_type;

  // Events are not supported in introspection mode
  // rclcpp will handle RMW_RET_UNSUPPORTED gracefully
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t rmw_take_event(const rmw_event_t *event_handle, void *event_info,
                         bool *taken) {
  (void)event_handle;
  (void)event_info;

  if (!taken) {
    RMW_SET_ERROR_MSG("taken is null");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // No events available in introspection mode
  *taken = false;
  return RMW_RET_OK;
}

rmw_ret_t rmw_event_set_callback(rmw_event_t *event,
                                 rmw_event_callback_t callback,
                                 const void *user_data) {
  (void)event;
  (void)callback;
  (void)user_data;

  // No-op: callbacks not supported in introspection mode
  return RMW_RET_OK;
}

} // extern "C"
