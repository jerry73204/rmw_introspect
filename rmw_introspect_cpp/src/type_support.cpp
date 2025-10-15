#include "rmw_introspect/type_support.hpp"
#include "rosidl_typesupport_introspection_cpp/identifier.hpp"
#include "rosidl_typesupport_introspection_cpp/message_introspection.hpp"
#include "rosidl_typesupport_introspection_cpp/service_introspection.hpp"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "rosidl_typesupport_introspection_c/service_introspection.h"
#include "rosidl_typesupport_cpp/message_type_support_dispatch.hpp"
#include "rosidl_typesupport_cpp/service_type_support_dispatch.hpp"
#include <sstream>

namespace rmw_introspect {

std::string extract_message_type(const rosidl_message_type_support_t * type_support)
{
  if (!type_support) {
    return "unknown/msg/Unknown";
  }

  // Try C++ introspection type support first
  const rosidl_message_type_support_t * introspection_ts_cpp =
    rosidl_typesupport_cpp::get_message_typesupport_handle_function(
      type_support,
      rosidl_typesupport_introspection_cpp::typesupport_identifier
    );

  if (introspection_ts_cpp && introspection_ts_cpp->data) {
    // Get the members from C++ introspection type support
    const auto * members =
      static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(
      introspection_ts_cpp->data);

    if (members && members->message_namespace_ && members->message_name_) {
      // Format: package_name/msg/MessageName
      // Note: message_namespace_ contains "package_name::msg" in C++ format
      std::string ns = members->message_namespace_;

      // Replace "::" with "/"
      size_t pos = ns.find("::");
      if (pos != std::string::npos) {
        ns.replace(pos, 2, "/");
      }

      std::ostringstream oss;
      oss << ns << "/" << members->message_name_;
      return oss.str();
    }
  }

  // Try C introspection type support as fallback
  const rosidl_message_type_support_t * introspection_ts_c =
    rosidl_typesupport_cpp::get_message_typesupport_handle_function(
      type_support,
      rosidl_typesupport_introspection_c__identifier
    );

  if (introspection_ts_c && introspection_ts_c->data) {
    // Get the members from C introspection type support
    const auto * members_c =
      static_cast<const rosidl_typesupport_introspection_c__MessageMembers *>(
      introspection_ts_c->data);

    if (members_c && members_c->message_namespace_ && members_c->message_name_) {
      // Format: package_name/msg/MessageName
      // Note: message_namespace_ contains "package_name__msg" in C format (with __)
      std::string ns = members_c->message_namespace_;

      // Replace "__" with "/"
      size_t pos = ns.find("__");
      if (pos != std::string::npos) {
        ns.replace(pos, 2, "/");
      }

      std::ostringstream oss;
      oss << ns << "/" << members_c->message_name_;
      return oss.str();
    }
  }

  return "unknown/msg/Unknown";
}

std::string extract_service_type(const rosidl_service_type_support_t * type_support)
{
  if (!type_support) {
    return "unknown/srv/Unknown";
  }

  // Try C++ introspection type support first
  const rosidl_service_type_support_t * introspection_ts_cpp =
    rosidl_typesupport_cpp::get_service_typesupport_handle_function(
      type_support,
      rosidl_typesupport_introspection_cpp::typesupport_identifier
    );

  if (introspection_ts_cpp && introspection_ts_cpp->data) {
    // Get the members from C++ introspection type support
    const auto * members =
      static_cast<const rosidl_typesupport_introspection_cpp::ServiceMembers *>(
      introspection_ts_cpp->data);

    if (members && members->service_namespace_ && members->service_name_) {
      // Format: package_name/srv/ServiceName
      // Note: service_namespace_ contains "package_name::srv" in C++ format
      std::string ns = members->service_namespace_;

      // Replace "::" with "/"
      size_t pos = ns.find("::");
      if (pos != std::string::npos) {
        ns.replace(pos, 2, "/");
      }

      std::ostringstream oss;
      oss << ns << "/" << members->service_name_;
      return oss.str();
    }
  }

  // Try C introspection type support as fallback
  const rosidl_service_type_support_t * introspection_ts_c =
    rosidl_typesupport_cpp::get_service_typesupport_handle_function(
      type_support,
      rosidl_typesupport_introspection_c__identifier
    );

  if (introspection_ts_c && introspection_ts_c->data) {
    // Get the members from C introspection type support
    const auto * members_c =
      static_cast<const rosidl_typesupport_introspection_c__ServiceMembers *>(
      introspection_ts_c->data);

    if (members_c && members_c->service_namespace_ && members_c->service_name_) {
      // Format: package_name/srv/ServiceName
      // Note: service_namespace_ contains "package_name__srv" in C format (with __)
      std::string ns = members_c->service_namespace_;

      // Replace "__" with "/"
      size_t pos = ns.find("__");
      if (pos != std::string::npos) {
        ns.replace(pos, 2, "/");
      }

      std::ostringstream oss;
      oss << ns << "/" << members_c->service_name_;
      return oss.str();
    }
  }

  return "unknown/srv/Unknown";
}

}  // namespace rmw_introspect
