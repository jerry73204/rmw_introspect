#include "rmw/rmw.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rcutils/macros.h"
#include "rmw_introspect/identifier.hpp"
#include "rmw_introspect/visibility_control.h"
#include <new>

extern "C"
{

// Check QoS profile compatibility - always return compatible
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_qos_profile_check_compatible(
  const rmw_qos_profile_t publisher_profile,
  const rmw_qos_profile_t subscription_profile,
  rmw_qos_compatibility_type_t * compatibility,
  char * reason,
  size_t reason_size)
{
  (void)publisher_profile;
  (void)subscription_profile;
  (void)reason;
  (void)reason_size;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(compatibility, RMW_RET_INVALID_ARGUMENT);

  // Always report compatible - introspection doesn't perform actual communication
  *compatibility = RMW_QOS_COMPATIBILITY_OK;
  return RMW_RET_OK;
}

}  // extern "C"
