#ifndef RMW_INTROSPECT__DATA_HPP_
#define RMW_INTROSPECT__DATA_HPP_

#include "rmw_introspect/types.hpp"
#include <mutex>
#include <string>
#include <vector>

namespace rmw_introspect {

/// Singleton class for storing introspection data
class IntrospectionData {
public:
  /// Get singleton instance
  static IntrospectionData &instance();

  /// Record node creation
  void record_node(const std::string &name, const std::string &ns);

  /// Record publisher creation
  void record_publisher(const PublisherInfo &info);

  /// Record subscription creation
  void record_subscription(const SubscriptionInfo &info);

  /// Record service creation
  void record_service(const ServiceInfo &info);

  /// Record client creation
  void record_client(const ClientInfo &info);

  /// Export data to JSON file
  void export_to_json(const std::string &path);

  /// Export data to YAML file
  void export_to_yaml(const std::string &path);

  /// Clear all recorded data (for testing)
  void clear();

  /// Get recorded data (for testing)
  const std::vector<std::string> &get_nodes() const { return nodes_; }
  const std::vector<PublisherInfo> &get_publishers() const {
    return publishers_;
  }
  const std::vector<SubscriptionInfo> &get_subscriptions() const {
    return subscriptions_;
  }
  const std::vector<ServiceInfo> &get_services() const { return services_; }
  const std::vector<ClientInfo> &get_clients() const { return clients_; }

private:
  IntrospectionData() = default;
  ~IntrospectionData() = default;

  // Delete copy and move constructors/operators
  IntrospectionData(const IntrospectionData &) = delete;
  IntrospectionData &operator=(const IntrospectionData &) = delete;
  IntrospectionData(IntrospectionData &&) = delete;
  IntrospectionData &operator=(IntrospectionData &&) = delete;

  std::vector<std::string> nodes_;
  std::vector<PublisherInfo> publishers_;
  std::vector<SubscriptionInfo> subscriptions_;
  std::vector<ServiceInfo> services_;
  std::vector<ClientInfo> clients_;

  mutable std::mutex mutex_; // Thread safety
};

} // namespace rmw_introspect

#endif // RMW_INTROSPECT__DATA_HPP_
