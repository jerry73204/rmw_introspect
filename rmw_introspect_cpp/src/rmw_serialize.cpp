#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw_introspect/forwarding.hpp"
#include "rmw_introspect/mode.hpp"

extern "C" {

rmw_ret_t rmw_serialize(const void *ros_message,
                        const rosidl_message_type_support_t *type_support,
                        rmw_serialized_message_t *serialized_message) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(ros_message, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(type_support, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(serialized_message, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    return g_real_rmw->serialize(ros_message, type_support, serialized_message);
  }

  // Recording-only mode: no-op
  return RMW_RET_OK;
}

rmw_ret_t rmw_deserialize(const rmw_serialized_message_t *serialized_message,
                          const rosidl_message_type_support_t *type_support,
                          void *ros_message) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(serialized_message, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(type_support, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(ros_message, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    return g_real_rmw->deserialize(serialized_message, type_support,
                                   ros_message);
  }

  // Recording-only mode: no-op
  return RMW_RET_OK;
}

rmw_ret_t rmw_get_serialized_message_size(
    const void *ros_message, const rosidl_message_type_support_t *type_support,
    size_t *size) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(ros_message, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(type_support, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(size, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    return g_real_rmw->get_serialized_message_size(ros_message, type_support,
                                                   size);
  }

  // Recording-only mode: return 0
  *size = 0;
  return RMW_RET_OK;
}

} // extern "C"
