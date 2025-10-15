#include "rmw/rmw.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rcutils/macros.h"
#include "rmw_introspect/identifier.hpp"
#include "rmw_introspect/visibility_control.h"

extern "C"
{

// Set callback for new message - accept but never invoke
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_subscription_set_on_new_message_callback(
  rmw_subscription_t * subscription,
  rmw_event_callback_t callback,
  const void * user_data)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    subscription,
    subscription->implementation_identifier,
    rmw_introspect_cpp_identifier,
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  (void)callback;
  (void)user_data;

  // Accept callback but never invoke - introspection doesn't receive messages
  return RMW_RET_OK;
}

// Set callback for new service request - accept but never invoke
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_service_set_on_new_request_callback(
  rmw_service_t * service,
  rmw_event_callback_t callback,
  const void * user_data)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(service, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    service,
    service->implementation_identifier,
    rmw_introspect_cpp_identifier,
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  (void)callback;
  (void)user_data;

  // Accept callback but never invoke - introspection doesn't receive requests
  return RMW_RET_OK;
}

// Set callback for new client response - accept but never invoke
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_client_set_on_new_response_callback(
  rmw_client_t * client,
  rmw_event_callback_t callback,
  const void * user_data)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(client, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    client,
    client->implementation_identifier,
    rmw_introspect_cpp_identifier,
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  (void)callback;
  (void)user_data;

  // Accept callback but never invoke - introspection doesn't receive responses
  return RMW_RET_OK;
}

// Note: rmw_event_set_callback is already implemented in rmw_event.cpp

}  // extern "C"
