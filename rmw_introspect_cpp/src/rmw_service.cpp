#include "rcutils/macros.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw_introspect/data.hpp"
#include "rmw_introspect/identifier.hpp"
#include "rmw_introspect/type_support.hpp"
#include "rmw_introspect/types.hpp"
#include "rmw_introspect/visibility_control.h"
#include <chrono>
#include <new>

extern "C" {

// Create service
RMW_INTROSPECT_PUBLIC
rmw_service_t *rmw_create_service(
    const rmw_node_t *node, const rosidl_service_type_support_t *type_support,
    const char *service_name, const rmw_qos_profile_t *qos_profile) {
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node, nullptr);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(type_support, nullptr);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(service_name, nullptr);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(qos_profile, nullptr);

  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(node, node->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return nullptr);

  // Allocate service structure
  rmw_service_t *service = new (std::nothrow) rmw_service_t;
  if (!service) {
    RMW_SET_ERROR_MSG("failed to allocate service");
    return nullptr;
  }

  service->implementation_identifier = rmw_introspect_cpp_identifier;
  service->data = nullptr;
  service->service_name = service_name;

  // Extract service type
  std::string service_type = rmw_introspect::extract_service_type(type_support);

  // Record service info
  rmw_introspect::ServiceInfo info;
  info.node_name = node->name;
  info.node_namespace = node->namespace_;
  info.service_name = service_name;
  info.service_type = service_type;
  info.qos = rmw_introspect::QoSProfile::from_rmw(*qos_profile);
  info.timestamp = std::chrono::duration<double>(
                       std::chrono::system_clock::now().time_since_epoch())
                       .count();

  rmw_introspect::IntrospectionData::instance().record_service(info);

  return service;
}

// Destroy service
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_destroy_service(rmw_node_t *node, rmw_service_t *service) {
  (void)node;
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(service, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(service, service->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  delete service;
  return RMW_RET_OK;
}

// Take request (no-op)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_take_request(const rmw_service_t *service,
                           rmw_service_info_t *request_header,
                           void *ros_request, bool *taken) {
  (void)service;
  (void)request_header;
  (void)ros_request;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(taken, RMW_RET_INVALID_ARGUMENT);
  *taken = false;
  return RMW_RET_OK;
}

// Send response (no-op)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_send_response(const rmw_service_t *service,
                            rmw_request_id_t *request_header,
                            void *ros_response) {
  (void)service;
  (void)request_header;
  (void)ros_response;
  return RMW_RET_OK;
}

// Service server is available (stub)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_service_server_is_available(const rmw_node_t *node,
                                          const rmw_client_t *client,
                                          bool *is_available) {
  (void)node;
  (void)client;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(is_available, RMW_RET_INVALID_ARGUMENT);
  *is_available = false;
  return RMW_RET_OK;
}

// Get service request subscription actual QoS (stub)
RMW_INTROSPECT_PUBLIC
rmw_ret_t
rmw_service_request_subscription_get_actual_qos(const rmw_service_t *service,
                                                rmw_qos_profile_t *qos) {
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(service, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(qos, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(service, service->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  *qos = rmw_qos_profile_default;
  return RMW_RET_OK;
}

// Get service response publisher actual QoS (stub)
RMW_INTROSPECT_PUBLIC
rmw_ret_t
rmw_service_response_publisher_get_actual_qos(const rmw_service_t *service,
                                              rmw_qos_profile_t *qos) {
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(service, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(qos, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(service, service->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  *qos = rmw_qos_profile_default;
  return RMW_RET_OK;
}

} // extern "C"
