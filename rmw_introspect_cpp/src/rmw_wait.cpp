#include "rcutils/macros.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw_introspect/forwarding.hpp"
#include "rmw_introspect/identifier.hpp"
#include "rmw_introspect/mode.hpp"
#include "rmw_introspect/real_rmw.hpp"
#include "rmw_introspect/visibility_control.h"
#include "rmw_introspect/wrappers.hpp"
#include <new>
#include <vector>

extern "C" {

// Create wait set
RMW_INTROSPECT_PUBLIC
rmw_wait_set_t *rmw_create_wait_set(rmw_context_t *context,
                                    size_t max_conditions) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(context, nullptr);

  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(context, context->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return nullptr);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_context_t *real_context = unwrap_context(context);
    if (!real_context) {
      RMW_SET_ERROR_MSG("failed to unwrap context");
      return nullptr;
    }

    // Forward to real RMW
    rmw_wait_set_t *real_wait_set =
        g_real_rmw->create_wait_set(real_context, max_conditions);

    if (!real_wait_set) {
      return nullptr;
    }

    // Create wrapper
    auto *wrapper =
        new (std::nothrow) rmw_introspect::WaitSetWrapper(real_wait_set);
    if (!wrapper) {
      g_real_rmw->destroy_wait_set(real_wait_set);
      RMW_SET_ERROR_MSG("failed to allocate wait set wrapper");
      return nullptr;
    }

    // Create our wait set structure
    rmw_wait_set_t *wait_set = new (std::nothrow) rmw_wait_set_t;
    if (!wait_set) {
      g_real_rmw->destroy_wait_set(real_wait_set);
      delete wrapper;
      RMW_SET_ERROR_MSG("failed to allocate wait set");
      return nullptr;
    }

    wait_set->implementation_identifier = rmw_introspect_cpp_identifier;
    wait_set->data = wrapper;

    return wait_set;
  }

  // Recording-only mode (existing behavior)
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
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(wait_set, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(wait_set,
                                   wait_set->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    auto *wrapper =
        static_cast<rmw_introspect::WaitSetWrapper *>(wait_set->data);
    if (wrapper && wrapper->real_wait_set) {
      rmw_ret_t ret = g_real_rmw->destroy_wait_set(wrapper->real_wait_set);
      if (ret != RMW_RET_OK) {
        return ret;
      }
    }
    delete wrapper;
    delete wait_set;
    return RMW_RET_OK;
  }

  // Recording-only mode
  delete wait_set;
  return RMW_RET_OK;
}

// Wait - unwrap all handles and forward to real RMW
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_wait(rmw_subscriptions_t *subscriptions,
                   rmw_guard_conditions_t *guard_conditions,
                   rmw_services_t *services, rmw_clients_t *clients,
                   rmw_events_t *events, rmw_wait_set_t *wait_set,
                   const rmw_time_t *wait_timeout) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(wait_set, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: unwrap all handles and forward
  if (is_intermediate_mode()) {
    rmw_wait_set_t *real_wait_set = unwrap_wait_set(wait_set);
    if (!real_wait_set) {
      RMW_SET_ERROR_MSG("failed to unwrap wait set");
      return RMW_RET_ERROR;
    }

    // Unwrap subscriptions array
    std::vector<void *> real_subs_storage;
    rmw_subscriptions_t real_subscriptions;
    if (subscriptions && subscriptions->subscriber_count > 0) {
      real_subs_storage.resize(subscriptions->subscriber_count);
      for (size_t i = 0; i < subscriptions->subscriber_count; ++i) {
        real_subs_storage[i] = unwrap_subscription(
            static_cast<rmw_subscription_t *>(subscriptions->subscribers[i]));
      }
      real_subscriptions.subscriber_count = subscriptions->subscriber_count;
      real_subscriptions.subscribers = real_subs_storage.data();
    } else {
      real_subscriptions.subscriber_count = 0;
      real_subscriptions.subscribers = nullptr;
    }

    // Unwrap guard conditions array
    std::vector<void *> real_gcs_storage;
    rmw_guard_conditions_t real_guard_conditions;
    if (guard_conditions && guard_conditions->guard_condition_count > 0) {
      real_gcs_storage.resize(guard_conditions->guard_condition_count);
      for (size_t i = 0; i < guard_conditions->guard_condition_count; ++i) {
        real_gcs_storage[i] =
            unwrap_guard_condition(static_cast<rmw_guard_condition_t *>(
                guard_conditions->guard_conditions[i]));
      }
      real_guard_conditions.guard_condition_count =
          guard_conditions->guard_condition_count;
      real_guard_conditions.guard_conditions = real_gcs_storage.data();
    } else {
      real_guard_conditions.guard_condition_count = 0;
      real_guard_conditions.guard_conditions = nullptr;
    }

    // Unwrap services array
    std::vector<void *> real_srvs_storage;
    rmw_services_t real_services;
    if (services && services->service_count > 0) {
      real_srvs_storage.resize(services->service_count);
      for (size_t i = 0; i < services->service_count; ++i) {
        real_srvs_storage[i] =
            unwrap_service(static_cast<rmw_service_t *>(services->services[i]));
      }
      real_services.service_count = services->service_count;
      real_services.services = real_srvs_storage.data();
    } else {
      real_services.service_count = 0;
      real_services.services = nullptr;
    }

    // Unwrap clients array
    std::vector<void *> real_clients_storage;
    rmw_clients_t real_clients;
    if (clients && clients->client_count > 0) {
      real_clients_storage.resize(clients->client_count);
      for (size_t i = 0; i < clients->client_count; ++i) {
        real_clients_storage[i] =
            unwrap_client(static_cast<rmw_client_t *>(clients->clients[i]));
      }
      real_clients.client_count = clients->client_count;
      real_clients.clients = real_clients_storage.data();
    } else {
      real_clients.client_count = 0;
      real_clients.clients = nullptr;
    }

    // Events are passed through as-is (they don't have wrappers in our
    // implementation)
    rmw_events_t *real_events = events;

    // Call real RMW wait
    rmw_ret_t ret = g_real_rmw->wait(
        subscriptions ? &real_subscriptions : nullptr,
        guard_conditions ? &real_guard_conditions : nullptr,
        services ? &real_services : nullptr, clients ? &real_clients : nullptr,
        real_events, real_wait_set, wait_timeout);

    // Update ready flags in original arrays based on real arrays
    if (ret == RMW_RET_OK) {
      if (subscriptions && subscriptions->subscriber_count > 0) {
        for (size_t i = 0; i < subscriptions->subscriber_count; ++i) {
          subscriptions->subscribers[i] = real_subscriptions.subscribers[i]
                                              ? subscriptions->subscribers[i]
                                              : nullptr;
        }
      }
      if (guard_conditions && guard_conditions->guard_condition_count > 0) {
        for (size_t i = 0; i < guard_conditions->guard_condition_count; ++i) {
          guard_conditions->guard_conditions[i] =
              real_guard_conditions.guard_conditions[i]
                  ? guard_conditions->guard_conditions[i]
                  : nullptr;
        }
      }
      if (services && services->service_count > 0) {
        for (size_t i = 0; i < services->service_count; ++i) {
          services->services[i] =
              real_services.services[i] ? services->services[i] : nullptr;
        }
      }
      if (clients && clients->client_count > 0) {
        for (size_t i = 0; i < clients->client_count; ++i) {
          clients->clients[i] =
              real_clients.clients[i] ? clients->clients[i] : nullptr;
        }
      }
    }

    return ret;
  }

  // Recording-only mode: Immediately return timeout to allow nodes to exit
  // gracefully This is critical for introspection - we don't actually wait for
  // events
  return RMW_RET_TIMEOUT;
}

// Create guard condition
RMW_INTROSPECT_PUBLIC
rmw_guard_condition_t *rmw_create_guard_condition(rmw_context_t *context) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(context, nullptr);

  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(context, context->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return nullptr);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_context_t *real_context = unwrap_context(context);
    if (!real_context) {
      RMW_SET_ERROR_MSG("failed to unwrap context");
      return nullptr;
    }

    // Forward to real RMW
    rmw_guard_condition_t *real_guard_condition =
        g_real_rmw->create_guard_condition(real_context);

    if (!real_guard_condition) {
      return nullptr;
    }

    // Create wrapper
    auto *wrapper = new (std::nothrow)
        rmw_introspect::GuardConditionWrapper(real_guard_condition);
    if (!wrapper) {
      g_real_rmw->destroy_guard_condition(real_guard_condition);
      RMW_SET_ERROR_MSG("failed to allocate guard condition wrapper");
      return nullptr;
    }

    // Create our guard condition structure
    rmw_guard_condition_t *guard_condition =
        new (std::nothrow) rmw_guard_condition_t;
    if (!guard_condition) {
      g_real_rmw->destroy_guard_condition(real_guard_condition);
      delete wrapper;
      RMW_SET_ERROR_MSG("failed to allocate guard condition");
      return nullptr;
    }

    guard_condition->implementation_identifier = rmw_introspect_cpp_identifier;
    guard_condition->data = wrapper;
    guard_condition->context = context;

    return guard_condition;
  }

  // Recording-only mode (existing behavior)
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
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(guard_condition, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(guard_condition,
                                   guard_condition->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    auto *wrapper = static_cast<rmw_introspect::GuardConditionWrapper *>(
        guard_condition->data);
    if (wrapper && wrapper->real_guard_condition) {
      rmw_ret_t ret =
          g_real_rmw->destroy_guard_condition(wrapper->real_guard_condition);
      if (ret != RMW_RET_OK) {
        return ret;
      }
    }
    delete wrapper;
    delete guard_condition;
    return RMW_RET_OK;
  }

  // Recording-only mode
  delete guard_condition;
  return RMW_RET_OK;
}

// Trigger guard condition
RMW_INTROSPECT_PUBLIC
rmw_ret_t
rmw_trigger_guard_condition(const rmw_guard_condition_t *guard_condition) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(guard_condition, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_guard_condition_t *real_guard_condition =
        unwrap_guard_condition(guard_condition);
    if (!real_guard_condition) {
      RMW_SET_ERROR_MSG("failed to unwrap guard condition");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->trigger_guard_condition(real_guard_condition);
  }

  // Recording-only mode: no-op
  return RMW_RET_OK;
}

} // extern "C"
