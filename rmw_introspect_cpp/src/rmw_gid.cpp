#include "rcutils/macros.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw_introspect/identifier.hpp"
#include "rmw_introspect/visibility_control.h"
#include <cstring>

extern "C" {

// Get GID for publisher - return stub GID (all zeros)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_get_gid_for_publisher(const rmw_publisher_t *publisher,
                                    rmw_gid_t *gid) {
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(gid, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(publisher,
                                   publisher->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // Return stub GID (all zeros)
  gid->implementation_identifier = rmw_introspect_cpp_identifier;
  memset(gid->data, 0, RMW_GID_STORAGE_SIZE);

  return RMW_RET_OK;
}

// Compare GIDs - always return equal (all GIDs are stub zeros)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_compare_gids_equal(const rmw_gid_t *gid1, const rmw_gid_t *gid2,
                                 bool *result) {
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(gid1, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(gid2, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(result, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(gid1, gid1->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(gid2, gid2->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // All GIDs are equal in introspection mode (all zeros)
  *result = true;
  return RMW_RET_OK;
}

} // extern "C"
