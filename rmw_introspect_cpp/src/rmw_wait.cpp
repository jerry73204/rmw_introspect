#include "rcutils/macros.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw_introspect/identifier.hpp"
#include "rmw_introspect/visibility_control.h"
#include <new>

extern "C" {

// Create wait set
RMW_INTROSPECT_PUBLIC
rmw_wait_set_t *rmw_create_wait_set(rmw_context_t *context,
                                    size_t max_conditions) {
  (void)max_conditions;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(context, nullptr);

  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(context, context->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return nullptr);

  // Allocate wait set structure
  rmw_wait_set_t *wait_set = new (std::nothrow) rmw_wait_set_t;
  if (!wait_set) {
    RMW_SET_ERROR_MSG("failed to allocate wait set");
    return nullptr;
  }

  wait_set->implementation_identifier = rmw_introspect_cpp_identifier;
  wait_set->data = nullptr;

  return wait_set;
}

// Destroy wait set
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_destroy_wait_set(rmw_wait_set_t *wait_set) {
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(wait_set, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(wait_set,
                                   wait_set->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  delete wait_set;
  return RMW_RET_OK;
}

// Wait - CRITICAL: Return timeout immediately to allow graceful shutdown
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_wait(rmw_subscriptions_t *subscriptions,
                   rmw_guard_conditions_t *guard_conditions,
                   rmw_services_t *services, rmw_clients_t *clients,
                   rmw_events_t *events, rmw_wait_set_t *wait_set,
                   const rmw_time_t *wait_timeout) {
  (void)subscriptions;
  (void)guard_conditions;
  (void)services;
  (void)clients;
  (void)events;
  (void)wait_set;
  (void)wait_timeout;

  // Immediately return timeout to allow nodes to exit gracefully
  // This is critical for introspection - we don't actually wait for events
  return RMW_RET_TIMEOUT;
}

// Create guard condition
RMW_INTROSPECT_PUBLIC
rmw_guard_condition_t *rmw_create_guard_condition(rmw_context_t *context) {
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(context, nullptr);

  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(context, context->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return nullptr);

  // Allocate guard condition structure
  rmw_guard_condition_t *guard_condition =
      new (std::nothrow) rmw_guard_condition_t;
  if (!guard_condition) {
    RMW_SET_ERROR_MSG("failed to allocate guard condition");
    return nullptr;
  }

  guard_condition->implementation_identifier = rmw_introspect_cpp_identifier;
  guard_condition->data = nullptr;
  guard_condition->context = context;

  return guard_condition;
}

// Destroy guard condition
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_destroy_guard_condition(rmw_guard_condition_t *guard_condition) {
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(guard_condition, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(guard_condition,
                                   guard_condition->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  delete guard_condition;
  return RMW_RET_OK;
}

// Trigger guard condition (no-op)
RMW_INTROSPECT_PUBLIC
rmw_ret_t
rmw_trigger_guard_condition(const rmw_guard_condition_t *guard_condition) {
  (void)guard_condition;
  return RMW_RET_OK;
}

} // extern "C"
