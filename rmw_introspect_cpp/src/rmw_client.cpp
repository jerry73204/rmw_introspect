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

// Create client
RMW_INTROSPECT_PUBLIC
rmw_client_t *rmw_create_client(
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

  // Record client info (for both modes)
  rmw_introspect::ClientInfo info;
  info.node_name = node->name;
  info.node_namespace = node->namespace_;
  info.service_name = service_name;
  info.service_type = service_type;
  info.qos = rmw_introspect::QoSProfile::from_rmw(*qos_profile);
  info.timestamp = std::chrono::duration<double>(
                       std::chrono::system_clock::now().time_since_epoch())
                       .count();

  rmw_introspect::IntrospectionData::instance().record_client(info);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_node_t *real_node = unwrap_node(node);
    if (!real_node) {
      RMW_SET_ERROR_MSG("failed to unwrap node");
      return nullptr;
    }

    // Forward to real RMW
    rmw_client_t *real_client = g_real_rmw->create_client(
        real_node, type_support, service_name, qos_profile);

    if (!real_client) {
      return nullptr;
    }

    // Create wrapper
    auto *wrapper = new (std::nothrow) rmw_introspect::ClientWrapper(
        real_client, service_name, service_type, *qos_profile);
    if (!wrapper) {
      g_real_rmw->destroy_client(real_node, real_client);
      RMW_SET_ERROR_MSG("failed to allocate client wrapper");
      return nullptr;
    }

    // Create our client structure
    rmw_client_t *client = new (std::nothrow) rmw_client_t;
    if (!client) {
      g_real_rmw->destroy_client(real_node, real_client);
      delete wrapper;
      RMW_SET_ERROR_MSG("failed to allocate client");
      return nullptr;
    }

    client->implementation_identifier = rmw_introspect_cpp_identifier;
    client->data = wrapper;
    client->service_name = real_client->service_name;

    return client;
  }

  // Recording-only mode (existing behavior)
  rmw_client_t *client = new (std::nothrow) rmw_client_t;
  if (!client) {
    RMW_SET_ERROR_MSG("failed to allocate client");
    return nullptr;
  }

  client->implementation_identifier = rmw_introspect_cpp_identifier;
  client->data = nullptr;
  client->service_name = service_name;

  return client;
}

// Destroy client
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_destroy_client(rmw_node_t *node, rmw_client_t *client) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(client, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(client, client->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    auto *wrapper = static_cast<rmw_introspect::ClientWrapper *>(client->data);
    if (wrapper && wrapper->real_client) {
      rmw_node_t *real_node = unwrap_node(node);
      if (!real_node) {
        RMW_SET_ERROR_MSG("failed to unwrap node");
        return RMW_RET_ERROR;
      }
      rmw_ret_t ret =
          g_real_rmw->destroy_client(real_node, wrapper->real_client);
      if (ret != RMW_RET_OK) {
        return ret;
      }
    }
    delete wrapper;
    delete client;
    return RMW_RET_OK;
  }

  // Recording-only mode
  delete client;
  return RMW_RET_OK;
}

// Send request
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_send_request(const rmw_client_t *client, const void *ros_request,
                           int64_t *sequence_id) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(client, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(ros_request, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(sequence_id, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_client_t *real_client = unwrap_client(client);
    if (!real_client) {
      RMW_SET_ERROR_MSG("failed to unwrap client");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->send_request(real_client, ros_request, sequence_id);
  }

  // Recording-only mode: return dummy sequence number
  *sequence_id = 1;
  return RMW_RET_OK;
}

// Take response
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_take_response(const rmw_client_t *client,
                            rmw_service_info_t *request_header,
                            void *ros_response, bool *taken) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(client, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(request_header, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(ros_response, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(taken, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_client_t *real_client = unwrap_client(client);
    if (!real_client) {
      RMW_SET_ERROR_MSG("failed to unwrap client");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->take_response(real_client, request_header, ros_response,
                                     taken);
  }

  // Recording-only mode: no-op
  *taken = false;
  return RMW_RET_OK;
}

// Get client request publisher actual QoS
RMW_INTROSPECT_PUBLIC
rmw_ret_t
rmw_client_request_publisher_get_actual_qos(const rmw_client_t *client,
                                            rmw_qos_profile_t *qos) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(client, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(qos, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(client, client->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_client_t *real_client = unwrap_client(client);
    if (!real_client) {
      RMW_SET_ERROR_MSG("failed to unwrap client");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->client_request_publisher_get_actual_qos(real_client,
                                                               qos);
  }

  // Recording-only mode: return default
  *qos = rmw_qos_profile_default;
  return RMW_RET_OK;
}

// Get client response subscription actual QoS
RMW_INTROSPECT_PUBLIC
rmw_ret_t
rmw_client_response_subscription_get_actual_qos(const rmw_client_t *client,
                                                rmw_qos_profile_t *qos) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(client, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(qos, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(client, client->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_client_t *real_client = unwrap_client(client);
    if (!real_client) {
      RMW_SET_ERROR_MSG("failed to unwrap client");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->client_response_subscription_get_actual_qos(real_client,
                                                                   qos);
  }

  // Recording-only mode: return default
  *qos = rmw_qos_profile_default;
  return RMW_RET_OK;
}

} // extern "C"
