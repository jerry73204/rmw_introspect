#include "rmw_introspect/wrappers.hpp"
#include "rmw_introspect/real_rmw.hpp"

namespace rmw_introspect {

// ContextWrapper
ContextWrapper::ContextWrapper()
  : real_context(nullptr),
    real_rmw(nullptr),
    real_rmw_name()
{
}

ContextWrapper::~ContextWrapper() {
  // Note: real_rmw is managed globally, not deleted here
  // real_context should be cleaned up by the caller
}

// NodeWrapper
NodeWrapper::NodeWrapper(rmw_node_t* real, const char* n, const char* ns)
  : real_node(real),
    name(n ? n : ""),
    namespace_(ns ? ns : "")
{
}

// PublisherWrapper
PublisherWrapper::PublisherWrapper(rmw_publisher_t* real, const std::string& topic,
                                   const std::string& type, const rmw_qos_profile_t& q)
  : real_publisher(real),
    topic_name(topic),
    message_type(type),
    qos(q)
{
}

// SubscriptionWrapper
SubscriptionWrapper::SubscriptionWrapper(rmw_subscription_t* real, const std::string& topic,
                                         const std::string& type, const rmw_qos_profile_t& q)
  : real_subscription(real),
    topic_name(topic),
    message_type(type),
    qos(q)
{
}

// ServiceWrapper
ServiceWrapper::ServiceWrapper(rmw_service_t* real, const std::string& name,
                               const std::string& type, const rmw_qos_profile_t& q)
  : real_service(real),
    service_name(name),
    service_type(type),
    qos(q)
{
}

// ClientWrapper
ClientWrapper::ClientWrapper(rmw_client_t* real, const std::string& name,
                             const std::string& type, const rmw_qos_profile_t& q)
  : real_client(real),
    service_name(name),
    service_type(type),
    qos(q)
{
}

// GuardConditionWrapper
GuardConditionWrapper::GuardConditionWrapper(rmw_guard_condition_t* real)
  : real_guard_condition(real)
{
}

// WaitSetWrapper
WaitSetWrapper::WaitSetWrapper(rmw_wait_set_t* real)
  : real_wait_set(real)
{
}

}  // namespace rmw_introspect
