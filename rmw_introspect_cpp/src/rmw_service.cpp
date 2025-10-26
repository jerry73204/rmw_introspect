#include "rcutils/macros.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw_introspect/data.hpp"
#include "rmw_introspect/forwarding.hpp"
#include "rmw_introspect/identifier.hpp"
#include "rmw_introspect/mode.hpp"
#include "rmw_introspect/real_rmw.hpp"
#include "rmw_introspect/type_support.hpp"
#include "rmw_introspect/types.hpp"
#include "rmw_introspect/visibility_control.h"
#include "rmw_introspect/wrappers.hpp"
#include <chrono>
#include <new>

extern "C" {

// Create service
RMW_INTROSPECT_PUBLIC
rmw_service_t *rmw_create_service(
    const rmw_node_t *node, const rosidl_service_type_support_t *type_support,
    const char *service_name, const rmw_qos_profile_t *qos_profile) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node, nullptr);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(type_support, nullptr);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(service_name, nullptr);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(qos_profile, nullptr);

  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(node, node->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return nullptr);

  // Extract service type
  std::string service_type = rmw_introspect::extract_service_type(type_support);

  // Record service info (for both modes)
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

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_node_t *real_node = unwrap_node(node);
    if (!real_node) {
      RMW_SET_ERROR_MSG("failed to unwrap node");
      return nullptr;
    }

    // Forward to real RMW
    rmw_service_t *real_service = g_real_rmw->create_service(
        real_node, type_support, service_name, qos_profile);

    if (!real_service) {
      return nullptr;
    }

    // Create wrapper
    auto *wrapper = new (std::nothrow) rmw_introspect::ServiceWrapper(
        real_service, service_name, service_type, *qos_profile);
    if (!wrapper) {
      g_real_rmw->destroy_service(real_node, real_service);
      RMW_SET_ERROR_MSG("failed to allocate service wrapper");
      return nullptr;
    }

    // Create our service structure
    rmw_service_t *service = new (std::nothrow) rmw_service_t;
    if (!service) {
      g_real_rmw->destroy_service(real_node, real_service);
      delete wrapper;
      RMW_SET_ERROR_MSG("failed to allocate service");
      return nullptr;
    }

    service->implementation_identifier = rmw_introspect_cpp_identifier;
    service->data = wrapper;
    service->service_name = real_service->service_name;

    return service;
  }

  // Recording-only mode (existing behavior)
  rmw_service_t *service = new (std::nothrow) rmw_service_t;
  if (!service) {
    RMW_SET_ERROR_MSG("failed to allocate service");
    return nullptr;
  }

  service->implementation_identifier = rmw_introspect_cpp_identifier;
  service->data = nullptr;
  service->service_name = service_name;

  return service;
}

// Destroy service
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_destroy_service(rmw_node_t *node, rmw_service_t *service) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(service, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(service, service->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    auto *wrapper =
        static_cast<rmw_introspect::ServiceWrapper *>(service->data);
    if (wrapper && wrapper->real_service) {
      rmw_node_t *real_node = unwrap_node(node);
      if (!real_node) {
        RMW_SET_ERROR_MSG("failed to unwrap node");
        return RMW_RET_ERROR;
      }
      rmw_ret_t ret =
          g_real_rmw->destroy_service(real_node, wrapper->real_service);
      if (ret != RMW_RET_OK) {
        return ret;
      }
    }
    delete wrapper;
    delete service;
    return RMW_RET_OK;
  }

  // Recording-only mode
  delete service;
  return RMW_RET_OK;
}

// Take request
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_take_request(const rmw_service_t *service,
                           rmw_service_info_t *request_header,
                           void *ros_request, bool *taken) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(service, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(request_header, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(ros_request, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(taken, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_service_t *real_service = unwrap_service(service);
    if (!real_service) {
      RMW_SET_ERROR_MSG("failed to unwrap service");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->take_request(real_service, request_header, ros_request,
                                    taken);
  }

  // Recording-only mode: no-op
  *taken = false;
  return RMW_RET_OK;
}

// Send response
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_send_response(const rmw_service_t *service,
                            rmw_request_id_t *request_header,
                            void *ros_response) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(service, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(request_header, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(ros_response, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_service_t *real_service = unwrap_service(service);
    if (!real_service) {
      RMW_SET_ERROR_MSG("failed to unwrap service");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->send_response(real_service, request_header,
                                     ros_response);
  }

  // Recording-only mode: no-op
  return RMW_RET_OK;
}

// Service server is available (stub)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_service_server_is_available(const rmw_node_t *node,
                                          const rmw_client_t *client,
                                          bool *is_available) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(client, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(is_available, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_node_t *real_node = unwrap_node(node);
    rmw_client_t *real_client = unwrap_client(client);
    if (!real_node || !real_client) {
      RMW_SET_ERROR_MSG("failed to unwrap node or client");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->service_server_is_available(real_node, real_client,
                                                   is_available);
  }

  // Recording-only mode: return false
  *is_available = false;
  return RMW_RET_OK;
}

// Get service request subscription actual QoS
RMW_INTROSPECT_PUBLIC
rmw_ret_t
rmw_service_request_subscription_get_actual_qos(const rmw_service_t *service,
                                                rmw_qos_profile_t *qos) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(service, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(qos, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(service, service->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_service_t *real_service = unwrap_service(service);
    if (!real_service) {
      RMW_SET_ERROR_MSG("failed to unwrap service");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->service_request_subscription_get_actual_qos(real_service,
                                                                   qos);
  }

  // Recording-only mode: return default
  *qos = rmw_qos_profile_default;
  return RMW_RET_OK;
}

// Get service response publisher actual QoS
RMW_INTROSPECT_PUBLIC
rmw_ret_t
rmw_service_response_publisher_get_actual_qos(const rmw_service_t *service,
                                              rmw_qos_profile_t *qos) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(service, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(qos, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(service, service->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_service_t *real_service = unwrap_service(service);
    if (!real_service) {
      RMW_SET_ERROR_MSG("failed to unwrap service");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->service_response_publisher_get_actual_qos(real_service,
                                                                 qos);
  }

  // Recording-only mode: return default
  *qos = rmw_qos_profile_default;
  return RMW_RET_OK;
}

} // extern "C"
