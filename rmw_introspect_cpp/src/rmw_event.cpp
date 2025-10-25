#include "rmw/error_handling.h"
#include "rmw/event.h"
#include "rmw/rmw.h"
#include "rmw_introspect/forwarding.hpp"
#include "rmw_introspect/identifier.hpp"
#include "rmw_introspect/mode.hpp"
#include "rmw_introspect/wrappers.hpp"
#include <cstring>

extern "C" {

rmw_ret_t rmw_publisher_event_init(rmw_event_t *rmw_event,
                                   const rmw_publisher_t *publisher,
                                   rmw_event_type_t event_type) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(rmw_event, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_publisher_t *real_publisher = unwrap_publisher(publisher);
    if (!real_publisher) {
      RMW_SET_ERROR_MSG("failed to unwrap publisher");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->publisher_event_init(rmw_event, real_publisher,
                                            event_type);
  }

  // Recording-only mode: events are not supported
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t rmw_subscription_event_init(rmw_event_t *rmw_event,
                                      const rmw_subscription_t *subscription,
                                      rmw_event_type_t event_type) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(rmw_event, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_subscription_t *real_subscription = unwrap_subscription(subscription);
    if (!real_subscription) {
      RMW_SET_ERROR_MSG("failed to unwrap subscription");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->subscription_event_init(rmw_event, real_subscription,
                                               event_type);
  }

  // Recording-only mode: events are not supported
  return RMW_RET_UNSUPPORTED;
}

rmw_ret_t rmw_take_event(const rmw_event_t *event_handle, void *event_info,
                         bool *taken) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(event_handle, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(event_info, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(taken, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    return g_real_rmw->take_event(event_handle, event_info, taken);
  }

  // Recording-only mode: no events available
  *taken = false;
  return RMW_RET_OK;
}

rmw_ret_t rmw_event_fini(rmw_event_t *event) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(event, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    return g_real_rmw->event_fini(event);
  }

  // Recording-only mode: no-op
  return RMW_RET_OK;
}

rmw_ret_t rmw_event_set_callback(rmw_event_t *event,
                                 rmw_event_callback_t callback,
                                 const void *user_data) {
  (void)event;
  (void)callback;
  (void)user_data;

  // No-op: callbacks not currently supported
  // This would need special handling in intermediate mode if needed
  return RMW_RET_OK;
}

} // extern "C"
