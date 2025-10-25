#ifndef RMW_INTROSPECT__TYPE_SUPPORT_HPP_
#define RMW_INTROSPECT__TYPE_SUPPORT_HPP_

#include "rosidl_typesupport_introspection_cpp/message_introspection.hpp"
#include "rosidl_typesupport_introspection_cpp/service_introspection.hpp"
#include <string>

namespace rmw_introspect {

/// Extract message type name from type support
/// Returns format: "package_name/msg/MessageName"
std::string
extract_message_type(const rosidl_message_type_support_t *type_support);

/// Extract service type name from type support
/// Returns format: "package_name/srv/ServiceName"
std::string
extract_service_type(const rosidl_service_type_support_t *type_support);

} // namespace rmw_introspect

#endif // RMW_INTROSPECT__TYPE_SUPPORT_HPP_
