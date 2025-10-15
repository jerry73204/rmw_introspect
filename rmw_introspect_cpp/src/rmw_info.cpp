#include "rmw/rmw.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/get_topic_endpoint_info.h"
#include "rmw/topic_endpoint_info_array.h"
#include "rmw/names_and_types.h"
#include "rcutils/macros.h"
#include "rmw_introspect/identifier.hpp"
#include "rmw_introspect/visibility_control.h"

extern "C"
{

// Get publishers info by topic - return empty array
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_get_publishers_info_by_topic(
  const rmw_node_t * node,
  rcutils_allocator_t * allocator,
  const char * topic_name,
  bool no_mangle,
  rmw_topic_endpoint_info_array_t * publishers_info)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(allocator, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(topic_name, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(publishers_info, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node,
    node->implementation_identifier,
    rmw_introspect_cpp_identifier,
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  (void)no_mangle;

  // Return empty array - introspection doesn't track runtime graph state
  rmw_ret_t ret = rmw_topic_endpoint_info_array_init_with_size(
    publishers_info, 0, allocator);
  return ret;
}

// Get subscriptions info by topic - return empty array
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_get_subscriptions_info_by_topic(
  const rmw_node_t * node,
  rcutils_allocator_t * allocator,
  const char * topic_name,
  bool no_mangle,
  rmw_topic_endpoint_info_array_t * subscriptions_info)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(allocator, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(topic_name, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(subscriptions_info, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node,
    node->implementation_identifier,
    rmw_introspect_cpp_identifier,
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  (void)no_mangle;

  // Return empty array - introspection doesn't track runtime graph state
  rmw_ret_t ret = rmw_topic_endpoint_info_array_init_with_size(
    subscriptions_info, 0, allocator);
  return ret;
}

}  // extern "C"
