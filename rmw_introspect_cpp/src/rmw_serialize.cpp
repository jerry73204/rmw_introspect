#include "rmw/error_handling.h"
#include "rmw/rmw.h"

extern "C" {

// Serialization stubs - no-ops since we don't actually transmit data

rmw_ret_t rmw_serialize(const void *ros_message,
                        const rosidl_message_type_support_t *type_support,
                        rmw_serialized_message_t *serialized_message) {
  (void)ros_message;
  (void)type_support;
  (void)serialized_message;

  // No-op: We don't actually serialize messages in introspection mode
  return RMW_RET_OK;
}

rmw_ret_t rmw_deserialize(const rmw_serialized_message_t *serialized_message,
                          const rosidl_message_type_support_t *type_support,
                          void *ros_message) {
  (void)serialized_message;
  (void)type_support;
  (void)ros_message;

  // No-op: We don't actually deserialize messages in introspection mode
  return RMW_RET_OK;
}

rmw_ret_t rmw_get_serialized_message_size(
    const rosidl_message_type_support_t *type_support,
    const rosidl_runtime_c__Sequence__bound *message_bounds, size_t *size) {
  (void)type_support;
  (void)message_bounds;

  if (!size) {
    RMW_SET_ERROR_MSG("size is null");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // Return 0 since we don't do actual serialization
  *size = 0;
  return RMW_RET_OK;
}

} // extern "C"
