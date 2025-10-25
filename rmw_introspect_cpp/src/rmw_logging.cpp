#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw_introspect/visibility_control.h"

extern "C" {

// Set log severity (stub - no-op in introspection mode)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_set_log_severity(rmw_log_severity_t severity) {
  (void)severity;
  // In introspection mode, we don't need to configure logging
  // since we're only collecting metadata, not running middleware
  return RMW_RET_OK;
}

} // extern "C"
