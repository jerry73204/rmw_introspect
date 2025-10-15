#include "rmw/rmw.h"
#include "rmw/error_handling.h"
#include "rmw/validate_full_topic_name.h"
#include "rmw/validate_namespace.h"
#include "rmw/validate_node_name.h"

extern "C" {

// Validation functions - delegate to rcutils validators

rmw_ret_t rmw_validate_full_topic_name(
  const char * topic_name,
  int * validation_result,
  size_t * invalid_index)
{
  if (!topic_name) {
    RMW_SET_ERROR_MSG("topic_name is null");
    return RMW_RET_INVALID_ARGUMENT;
  }
  if (!validation_result) {
    RMW_SET_ERROR_MSG("validation_result is null");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // Use rmw validation
  return rmw_validate_full_topic_name_with_size(
    topic_name,
    strlen(topic_name),
    validation_result,
    invalid_index);
}

rmw_ret_t rmw_validate_node_name(
  const char * node_name,
  int * validation_result,
  size_t * invalid_index)
{
  if (!node_name) {
    RMW_SET_ERROR_MSG("node_name is null");
    return RMW_RET_INVALID_ARGUMENT;
  }
  if (!validation_result) {
    RMW_SET_ERROR_MSG("validation_result is null");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // Use rmw validation
  return rmw_validate_node_name_with_size(
    node_name,
    strlen(node_name),
    validation_result,
    invalid_index);
}

rmw_ret_t rmw_validate_namespace(
  const char * namespace_,
  int * validation_result,
  size_t * invalid_index)
{
  if (!namespace_) {
    RMW_SET_ERROR_MSG("namespace is null");
    return RMW_RET_INVALID_ARGUMENT;
  }
  if (!validation_result) {
    RMW_SET_ERROR_MSG("validation_result is null");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // Use rmw validation
  return rmw_validate_namespace_with_size(
    namespace_,
    strlen(namespace_),
    validation_result,
    invalid_index);
}

}  // extern "C"
