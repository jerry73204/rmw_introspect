#include "rcutils/macros.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw_introspect/forwarding.hpp"
#include "rmw_introspect/identifier.hpp"
#include "rmw_introspect/mode.hpp"
#include "rmw_introspect/visibility_control.h"
#include "rmw_introspect/wrappers.hpp"
#include <cstring>

extern "C" {

// Get GID for publisher
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_get_gid_for_publisher(const rmw_publisher_t *publisher,
                                    rmw_gid_t *gid) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(gid, RMW_RET_INVALID_ARGUMENT);
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
    return g_real_rmw->get_gid_for_publisher(real_publisher, gid);
  }

  // Recording-only mode: return stub GID (all zeros)
  gid->implementation_identifier = rmw_introspect_cpp_identifier;
  memset(gid->data, 0, RMW_GID_STORAGE_SIZE);

  return RMW_RET_OK;
}

// Get GID for client
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_get_gid_for_client(const rmw_client_t *client, rmw_gid_t *gid) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(client, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(gid, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(client, client->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_client_t *real_client = unwrap_client(client);
    if (!real_client) {
      RMW_SET_ERROR_MSG("failed to unwrap client");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->get_gid_for_client(real_client, gid);
  }

  // Recording-only mode: return stub GID (all zeros)
  gid->implementation_identifier = rmw_introspect_cpp_identifier;
  memset(gid->data, 0, RMW_GID_STORAGE_SIZE);

  return RMW_RET_OK;
}

// Compare GIDs
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_compare_gids_equal(const rmw_gid_t *gid1, const rmw_gid_t *gid2,
                                 bool *result) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(gid1, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(gid2, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(result, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    return g_real_rmw->compare_gids_equal(gid1, gid2, result);
  }

  // Recording-only mode: all GIDs are equal (all zeros)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(gid1, gid1->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(gid2, gid2->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  *result = true;
  return RMW_RET_OK;
}

} // extern "C"
