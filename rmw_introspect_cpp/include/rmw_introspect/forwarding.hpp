#ifndef RMW_INTROSPECT__FORWARDING_HPP_
#define RMW_INTROSPECT__FORWARDING_HPP_

#include "rmw/rmw.h"
#include "rmw_introspect/wrappers.hpp"

namespace rmw_introspect {
namespace internal {

// --- Unwrapping Helpers ---

/// Unwrap context to get real RMW context
inline rmw_context_t *unwrap_context(const rmw_context_t *ctx) {
  if (!ctx || !ctx->impl)
    return nullptr;
  auto *wrapper = static_cast<ContextWrapper *>(ctx->impl);
  return wrapper->real_context;
}

/// Unwrap node to get real RMW node
inline rmw_node_t *unwrap_node(const rmw_node_t *node) {
  if (!node || !node->data)
    return nullptr;
  auto *wrapper = static_cast<NodeWrapper *>(node->data);
  return wrapper->real_node;
}

/// Unwrap publisher to get real RMW publisher
inline rmw_publisher_t *unwrap_publisher(const rmw_publisher_t *pub) {
  if (!pub || !pub->data)
    return nullptr;
  auto *wrapper = static_cast<PublisherWrapper *>(pub->data);
  return wrapper->real_publisher;
}

/// Unwrap subscription to get real RMW subscription
inline rmw_subscription_t *unwrap_subscription(const rmw_subscription_t *sub) {
  if (!sub || !sub->data)
    return nullptr;
  auto *wrapper = static_cast<SubscriptionWrapper *>(sub->data);
  return wrapper->real_subscription;
}

/// Unwrap service to get real RMW service
inline rmw_service_t *unwrap_service(const rmw_service_t *srv) {
  if (!srv || !srv->data)
    return nullptr;
  auto *wrapper = static_cast<ServiceWrapper *>(srv->data);
  return wrapper->real_service;
}

/// Unwrap client to get real RMW client
inline rmw_client_t *unwrap_client(const rmw_client_t *cli) {
  if (!cli || !cli->data)
    return nullptr;
  auto *wrapper = static_cast<ClientWrapper *>(cli->data);
  return wrapper->real_client;
}

/// Unwrap guard condition to get real RMW guard condition
inline rmw_guard_condition_t *
unwrap_guard_condition(const rmw_guard_condition_t *gc) {
  if (!gc || !gc->data)
    return nullptr;
  auto *wrapper = static_cast<GuardConditionWrapper *>(gc->data);
  return wrapper->real_guard_condition;
}

/// Unwrap wait set to get real RMW wait set
inline rmw_wait_set_t *unwrap_wait_set(const rmw_wait_set_t *ws) {
  if (!ws || !ws->data)
    return nullptr;
  auto *wrapper = static_cast<WaitSetWrapper *>(ws->data);
  return wrapper->real_wait_set;
}

} // namespace internal
} // namespace rmw_introspect

#endif // RMW_INTROSPECT__FORWARDING_HPP_
