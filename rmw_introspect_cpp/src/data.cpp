#include "rmw_introspect/data.hpp"
#include <fstream>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace rmw_introspect {

// QoSProfile implementation
QoSProfile QoSProfile::from_rmw(const rmw_qos_profile_t & qos)
{
  QoSProfile profile;

  // Reliability
  switch (qos.reliability) {
    case RMW_QOS_POLICY_RELIABILITY_RELIABLE:
      profile.reliability = "reliable";
      break;
    case RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT:
      profile.reliability = "best_effort";
      break;
    default:
      profile.reliability = "unknown";
      break;
  }

  // Durability
  switch (qos.durability) {
    case RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL:
      profile.durability = "transient_local";
      break;
    case RMW_QOS_POLICY_DURABILITY_VOLATILE:
      profile.durability = "volatile";
      break;
    default:
      profile.durability = "unknown";
      break;
  }

  // History
  switch (qos.history) {
    case RMW_QOS_POLICY_HISTORY_KEEP_LAST:
      profile.history = "keep_last";
      break;
    case RMW_QOS_POLICY_HISTORY_KEEP_ALL:
      profile.history = "keep_all";
      break;
    default:
      profile.history = "unknown";
      break;
  }

  profile.depth = static_cast<uint32_t>(qos.depth);

  return profile;
}

// IntrospectionData implementation
IntrospectionData & IntrospectionData::instance()
{
  static IntrospectionData instance;
  return instance;
}

void IntrospectionData::record_node(const std::string & name, const std::string & ns)
{
  std::lock_guard<std::mutex> lock(mutex_);
  nodes_.push_back(ns + "/" + name);
}

void IntrospectionData::record_publisher(const PublisherInfo & info)
{
  std::lock_guard<std::mutex> lock(mutex_);
  publishers_.push_back(info);
}

void IntrospectionData::record_subscription(const SubscriptionInfo & info)
{
  std::lock_guard<std::mutex> lock(mutex_);
  subscriptions_.push_back(info);
}

void IntrospectionData::record_service(const ServiceInfo & info)
{
  std::lock_guard<std::mutex> lock(mutex_);
  services_.push_back(info);
}

void IntrospectionData::record_client(const ClientInfo & info)
{
  std::lock_guard<std::mutex> lock(mutex_);
  clients_.push_back(info);
}

void IntrospectionData::clear()
{
  std::lock_guard<std::mutex> lock(mutex_);
  nodes_.clear();
  publishers_.clear();
  subscriptions_.clear();
  services_.clear();
  clients_.clear();
}

void IntrospectionData::export_to_json(const std::string & path)
{
  std::lock_guard<std::mutex> lock(mutex_);
  std::ofstream file(path);

  if (!file.is_open()) {
    return;
  }

  // Get current timestamp
  auto now = std::chrono::system_clock::now();
  auto time_t = std::chrono::system_clock::to_time_t(now);
  std::stringstream timestamp;
  timestamp << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");

  file << "{\n";
  file << "  \"format_version\": \"1.0\",\n";
  file << "  \"timestamp\": \"" << timestamp.str() << "\",\n";
  file << "  \"rmw_implementation\": \"rmw_introspect_cpp\",\n";

  // Nodes
  file << "  \"nodes\": [\n";
  for (size_t i = 0; i < nodes_.size(); ++i) {
    file << "    \"" << nodes_[i] << "\"";
    if (i < nodes_.size() - 1) {
      file << ",";
    }
    file << "\n";
  }
  file << "  ],\n";

  // Publishers
  file << "  \"publishers\": [\n";
  for (size_t i = 0; i < publishers_.size(); ++i) {
    const auto & pub = publishers_[i];
    file << "    {\n";
    file << "      \"node_name\": \"" << pub.node_name << "\",\n";
    file << "      \"node_namespace\": \"" << pub.node_namespace << "\",\n";
    file << "      \"topic_name\": \"" << pub.topic_name << "\",\n";
    file << "      \"message_type\": \"" << pub.message_type << "\",\n";
    file << "      \"qos\": {\n";
    file << "        \"reliability\": \"" << pub.qos.reliability << "\",\n";
    file << "        \"durability\": \"" << pub.qos.durability << "\",\n";
    file << "        \"history\": \"" << pub.qos.history << "\",\n";
    file << "        \"depth\": " << pub.qos.depth << "\n";
    file << "      }\n";
    file << "    }";
    if (i < publishers_.size() - 1) {
      file << ",";
    }
    file << "\n";
  }
  file << "  ],\n";

  // Subscriptions
  file << "  \"subscriptions\": [\n";
  for (size_t i = 0; i < subscriptions_.size(); ++i) {
    const auto & sub = subscriptions_[i];
    file << "    {\n";
    file << "      \"node_name\": \"" << sub.node_name << "\",\n";
    file << "      \"node_namespace\": \"" << sub.node_namespace << "\",\n";
    file << "      \"topic_name\": \"" << sub.topic_name << "\",\n";
    file << "      \"message_type\": \"" << sub.message_type << "\",\n";
    file << "      \"qos\": {\n";
    file << "        \"reliability\": \"" << sub.qos.reliability << "\",\n";
    file << "        \"durability\": \"" << sub.qos.durability << "\",\n";
    file << "        \"history\": \"" << sub.qos.history << "\",\n";
    file << "        \"depth\": " << sub.qos.depth << "\n";
    file << "      }\n";
    file << "    }";
    if (i < subscriptions_.size() - 1) {
      file << ",";
    }
    file << "\n";
  }
  file << "  ],\n";

  // Services
  file << "  \"services\": [],\n";

  // Clients
  file << "  \"clients\": []\n";

  file << "}\n";
}

void IntrospectionData::export_to_yaml(const std::string & path)
{
  (void)path;
  // TODO: Implement YAML export in future phases
}

}  // namespace rmw_introspect
