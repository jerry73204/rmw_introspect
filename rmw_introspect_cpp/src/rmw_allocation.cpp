#include "rcutils/macros.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw_introspect/identifier.hpp"
#include "rmw_introspect/visibility_control.h"

extern "C" {

// All allocation functions return UNSUPPORTED - introspection doesn't use
// zero-copy

// Initialize publisher allocation
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_init_publisher_allocation(
    const rosidl_message_type_support_t *type_support,
    const rosidl_runtime_c__Sequence__bound *message_bounds,
    rmw_publisher_allocation_t *allocation) {
  (void)type_support;
  (void)message_bounds;
  (void)allocation;
  return RMW_RET_UNSUPPORTED;
}

// Finalize publisher allocation
RMW_INTROSPECT_PUBLIC
rmw_ret_t
rmw_fini_publisher_allocation(rmw_publisher_allocation_t *allocation) {
  (void)allocation;
  return RMW_RET_UNSUPPORTED;
}

// Initialize subscription allocation
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_init_subscription_allocation(
    const rosidl_message_type_support_t *type_support,
    const rosidl_runtime_c__Sequence__bound *message_bounds,
    rmw_subscription_allocation_t *allocation) {
  (void)type_support;
  (void)message_bounds;
  (void)allocation;
  return RMW_RET_UNSUPPORTED;
}

// Finalize subscription allocation
RMW_INTROSPECT_PUBLIC
rmw_ret_t
rmw_fini_subscription_allocation(rmw_subscription_allocation_t *allocation) {
  (void)allocation;
  return RMW_RET_UNSUPPORTED;
}

// Publish loaned message
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_publish_loaned_message(const rmw_publisher_t *publisher,
                                     void *ros_message,
                                     rmw_publisher_allocation_t *allocation) {
  (void)publisher;
  (void)ros_message;
  (void)allocation;
  return RMW_RET_UNSUPPORTED;
}

} // extern "C"
