#ifndef RMW_INTROSPECT__WRAPPERS_HPP_
#define RMW_INTROSPECT__WRAPPERS_HPP_

#include "rmw/rmw.h"
#include "rmw/types.h"
#include <string>

namespace rmw_introspect {

class RealRMW;

/// Wrapper for rmw_context_t
struct ContextWrapper {
  rmw_context_t *real_context;
  RealRMW *real_rmw;
  std::string real_rmw_name;

  ContextWrapper();
  ~ContextWrapper();

  // Disable copy/move
  ContextWrapper(const ContextWrapper &) = delete;
  ContextWrapper &operator=(const ContextWrapper &) = delete;
};

/// Wrapper for rmw_node_t
struct NodeWrapper {
  rmw_node_t *real_node;
  std::string name;
  std::string namespace_;

  NodeWrapper(rmw_node_t *real, const char *n, const char *ns);
  ~NodeWrapper() = default;
};

/// Wrapper for rmw_publisher_t
struct PublisherWrapper {
  rmw_publisher_t *real_publisher;
  std::string topic_name;
  std::string message_type;
  rmw_qos_profile_t qos;

  PublisherWrapper(rmw_publisher_t *real, const std::string &topic,
                   const std::string &type, const rmw_qos_profile_t &q);
  ~PublisherWrapper() = default;
};

/// Wrapper for rmw_subscription_t
struct SubscriptionWrapper {
  rmw_subscription_t *real_subscription;
  std::string topic_name;
  std::string message_type;
  rmw_qos_profile_t qos;

  SubscriptionWrapper(rmw_subscription_t *real, const std::string &topic,
                      const std::string &type, const rmw_qos_profile_t &q);
  ~SubscriptionWrapper() = default;
};

/// Wrapper for rmw_service_t
struct ServiceWrapper {
  rmw_service_t *real_service;
  std::string service_name;
  std::string service_type;
  rmw_qos_profile_t qos;

  ServiceWrapper(rmw_service_t *real, const std::string &name,
                 const std::string &type, const rmw_qos_profile_t &q);
  ~ServiceWrapper() = default;
};

/// Wrapper for rmw_client_t
struct ClientWrapper {
  rmw_client_t *real_client;
  std::string service_name;
  std::string service_type;
  rmw_qos_profile_t qos;

  ClientWrapper(rmw_client_t *real, const std::string &name,
                const std::string &type, const rmw_qos_profile_t &q);
  ~ClientWrapper() = default;
};

/// Wrapper for rmw_guard_condition_t
struct GuardConditionWrapper {
  rmw_guard_condition_t *real_guard_condition;

  explicit GuardConditionWrapper(rmw_guard_condition_t *real);
  ~GuardConditionWrapper() = default;
};

/// Wrapper for rmw_wait_set_t
struct WaitSetWrapper {
  rmw_wait_set_t *real_wait_set;

  explicit WaitSetWrapper(rmw_wait_set_t *real);
  ~WaitSetWrapper() = default;
};

} // namespace rmw_introspect

#endif // RMW_INTROSPECT__WRAPPERS_HPP_
