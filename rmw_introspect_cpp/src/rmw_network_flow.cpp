#include "rmw/rmw.h"
#include "rmw/error_handling.h"
#include "rmw/network_flow_endpoint_array.h"

extern "C" {

// Network flow endpoint stubs - return empty arrays
// Not needed for introspection

rmw_ret_t rmw_publisher_get_network_flow_endpoints(
  const rmw_publisher_t * publisher,
  rcutils_allocator_t * allocator,
  rmw_network_flow_endpoint_array_t * network_flow_endpoint_array)
{
  (void)publisher;

  if (!allocator) {
    RMW_SET_ERROR_MSG("allocator is null");
    return RMW_RET_INVALID_ARGUMENT;
  }
  if (!network_flow_endpoint_array) {
    RMW_SET_ERROR_MSG("network_flow_endpoint_array is null");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // Return empty array
  return rmw_network_flow_endpoint_array_init(network_flow_endpoint_array, 0, allocator);
}

rmw_ret_t rmw_subscription_get_network_flow_endpoints(
  const rmw_subscription_t * subscription,
  rcutils_allocator_t * allocator,
  rmw_network_flow_endpoint_array_t * network_flow_endpoint_array)
{
  (void)subscription;

  if (!allocator) {
    RMW_SET_ERROR_MSG("allocator is null");
    return RMW_RET_INVALID_ARGUMENT;
  }
  if (!network_flow_endpoint_array) {
    RMW_SET_ERROR_MSG("network_flow_endpoint_array is null");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // Return empty array
  return rmw_network_flow_endpoint_array_init(network_flow_endpoint_array, 0, allocator);
}

}  // extern "C"
