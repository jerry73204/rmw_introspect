#include "rmw/rmw.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rcutils/macros.h"
#include "rmw_introspect/identifier.hpp"
#include "rmw_introspect/visibility_control.h"
#include "rmw_introspect/data.hpp"
#include "rmw_introspect/types.hpp"
#include "rmw_introspect/type_support.hpp"
#include <new>
#include <chrono>

extern "C"
{

// Create client
RMW_INTROSPECT_PUBLIC
rmw_client_t * rmw_create_client(
  const rmw_node_t * node,
  const rosidl_service_type_support_t * type_support,
  const char * service_name,
  const rmw_qos_profile_t * qos_profile)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node, nullptr);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(type_support, nullptr);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(service_name, nullptr);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(qos_profile, nullptr);

  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node,
    node->implementation_identifier,
    rmw_introspect_cpp_identifier,
    return nullptr);

  // Allocate client structure
  rmw_client_t * client = new (std::nothrow) rmw_client_t;
  if (!client) {
    RMW_SET_ERROR_MSG("failed to allocate client");
    return nullptr;
  }

  client->implementation_identifier = rmw_introspect_cpp_identifier;
  client->data = nullptr;
  client->service_name = service_name;

  // Extract service type
  std::string service_type = rmw_introspect::extract_service_type(type_support);

  // Record client info
  rmw_introspect::ClientInfo info;
  info.node_name = node->name;
  info.node_namespace = node->namespace_;
  info.service_name = service_name;
  info.service_type = service_type;
  info.qos = rmw_introspect::QoSProfile::from_rmw(*qos_profile);
  info.timestamp = std::chrono::duration<double>(
    std::chrono::system_clock::now().time_since_epoch()).count();

  rmw_introspect::IntrospectionData::instance().record_client(info);

  return client;
}

// Destroy client
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_destroy_client(rmw_node_t * node, rmw_client_t * client)
{
  (void)node;
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(client, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    client,
    client->implementation_identifier,
    rmw_introspect_cpp_identifier,
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  delete client;
  return RMW_RET_OK;
}

// Send request (stub - returns sequence number)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_send_request(
  const rmw_client_t * client,
  const void * ros_request,
  int64_t * sequence_id)
{
  (void)client;
  (void)ros_request;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(sequence_id, RMW_RET_INVALID_ARGUMENT);
  *sequence_id = 1;  // Return a dummy sequence number
  return RMW_RET_OK;
}

// Take response (no-op)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_take_response(
  const rmw_client_t * client,
  rmw_service_info_t * request_header,
  void * ros_response,
  bool * taken)
{
  (void)client;
  (void)request_header;
  (void)ros_response;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(taken, RMW_RET_INVALID_ARGUMENT);
  *taken = false;
  return RMW_RET_OK;
}

// Get client request publisher actual QoS (stub)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_client_request_publisher_get_actual_qos(
  const rmw_client_t * client,
  rmw_qos_profile_t * qos)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(client, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(qos, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    client,
    client->implementation_identifier,
    rmw_introspect_cpp_identifier,
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  *qos = rmw_qos_profile_default;
  return RMW_RET_OK;
}

// Get client response subscription actual QoS (stub)
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_client_response_subscription_get_actual_qos(
  const rmw_client_t * client,
  rmw_qos_profile_t * qos)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(client, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(qos, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    client,
    client->implementation_identifier,
    rmw_introspect_cpp_identifier,
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  *qos = rmw_qos_profile_default;
  return RMW_RET_OK;
}

}  // extern "C"
