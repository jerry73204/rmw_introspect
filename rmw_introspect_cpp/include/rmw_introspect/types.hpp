#ifndef RMW_INTROSPECT__TYPES_HPP_
#define RMW_INTROSPECT__TYPES_HPP_

#include "rmw/types.h"
#include <cstdint>
#include <string>

namespace rmw_introspect {

/// QoS profile structure
struct QoSProfile {
  std::string reliability; // "reliable" or "best_effort"
  std::string durability;  // "transient_local" or "volatile"
  std::string history;     // "keep_last" or "keep_all"
  uint32_t depth;

  /// Create QoSProfile from rmw_qos_profile_t
  static QoSProfile from_rmw(const rmw_qos_profile_t &qos);
};

/// Publisher metadata
struct PublisherInfo {
  std::string node_name;
  std::string node_namespace;
  std::string topic_name;
  std::string message_type; // e.g., "sensor_msgs/msg/Image"
  QoSProfile qos;
  double timestamp; // When created
};

/// Subscription metadata
struct SubscriptionInfo {
  std::string node_name;
  std::string node_namespace;
  std::string topic_name;
  std::string message_type;
  QoSProfile qos;
  double timestamp;
};

/// Service metadata
struct ServiceInfo {
  std::string node_name;
  std::string node_namespace;
  std::string service_name;
  std::string service_type; // e.g., "std_srvs/srv/SetBool"
  QoSProfile qos;
  double timestamp;
};

/// Client metadata
struct ClientInfo {
  std::string node_name;
  std::string node_namespace;
  std::string service_name;
  std::string service_type;
  QoSProfile qos;
  double timestamp;
};

} // namespace rmw_introspect

#endif // RMW_INTROSPECT__TYPES_HPP_
