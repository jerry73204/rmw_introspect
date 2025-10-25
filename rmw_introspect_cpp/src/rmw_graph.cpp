#include "rcutils/allocator.h"
#include "rcutils/types/string_array.h"
#include "rmw/error_handling.h"
#include "rmw/get_node_info_and_types.h"
#include "rmw/names_and_types.h"
#include "rmw/rmw.h"

extern "C" {

// Graph query stubs - all return empty results
// These functions are used for runtime graph introspection, not needed for our
// use case

rmw_ret_t rmw_count_publishers(const rmw_node_t *node, const char *topic_name,
                               size_t *count) {
  (void)node;
  (void)topic_name;
  if (!count) {
    RMW_SET_ERROR_MSG("count is null");
    return RMW_RET_INVALID_ARGUMENT;
  }
  *count = 0;
  return RMW_RET_OK;
}

rmw_ret_t rmw_count_subscribers(const rmw_node_t *node, const char *topic_name,
                                size_t *count) {
  (void)node;
  (void)topic_name;
  if (!count) {
    RMW_SET_ERROR_MSG("count is null");
    return RMW_RET_INVALID_ARGUMENT;
  }
  *count = 0;
  return RMW_RET_OK;
}

rmw_ret_t rmw_get_node_names(const rmw_node_t *node,
                             rcutils_string_array_t *node_names,
                             rcutils_string_array_t *node_namespaces) {
  (void)node;
  if (!node_names || !node_namespaces) {
    RMW_SET_ERROR_MSG("node_names or node_namespaces is null");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // Return empty arrays
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcutils_ret_t ret = rcutils_string_array_init(node_names, 0, &allocator);
  if (ret != RCUTILS_RET_OK) {
    RMW_SET_ERROR_MSG("failed to initialize node_names");
    return RMW_RET_ERROR;
  }
  ret = rcutils_string_array_init(node_namespaces, 0, &allocator);
  if (ret != RCUTILS_RET_OK) {
    RMW_SET_ERROR_MSG("failed to initialize node_namespaces");
    rcutils_string_array_fini(node_names);
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}

rmw_ret_t rmw_get_node_names_with_enclaves(
    const rmw_node_t *node, rcutils_string_array_t *node_names,
    rcutils_string_array_t *node_namespaces, rcutils_string_array_t *enclaves) {
  (void)node;
  if (!node_names || !node_namespaces || !enclaves) {
    RMW_SET_ERROR_MSG("node_names, node_namespaces, or enclaves is null");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // Return empty arrays
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcutils_ret_t ret = rcutils_string_array_init(node_names, 0, &allocator);
  if (ret != RCUTILS_RET_OK) {
    RMW_SET_ERROR_MSG("failed to initialize node_names");
    return RMW_RET_ERROR;
  }
  ret = rcutils_string_array_init(node_namespaces, 0, &allocator);
  if (ret != RCUTILS_RET_OK) {
    RMW_SET_ERROR_MSG("failed to initialize node_namespaces");
    rcutils_string_array_fini(node_names);
    return RMW_RET_ERROR;
  }
  ret = rcutils_string_array_init(enclaves, 0, &allocator);
  if (ret != RCUTILS_RET_OK) {
    RMW_SET_ERROR_MSG("failed to initialize enclaves");
    rcutils_string_array_fini(node_names);
    rcutils_string_array_fini(node_namespaces);
    return RMW_RET_ERROR;
  }
  return RMW_RET_OK;
}

rmw_ret_t
rmw_get_topic_names_and_types(const rmw_node_t *node,
                              rcutils_allocator_t *allocator, bool no_demangle,
                              rmw_names_and_types_t *topic_names_and_types) {
  (void)node;
  (void)no_demangle;
  if (!allocator || !topic_names_and_types) {
    RMW_SET_ERROR_MSG("allocator or topic_names_and_types is null");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // Return empty list
  return rmw_names_and_types_init(topic_names_and_types, 0, allocator);
}

rmw_ret_t rmw_get_service_names_and_types(
    const rmw_node_t *node, rcutils_allocator_t *allocator,
    rmw_names_and_types_t *service_names_and_types) {
  (void)node;
  if (!allocator || !service_names_and_types) {
    RMW_SET_ERROR_MSG("allocator or service_names_and_types is null");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // Return empty list
  return rmw_names_and_types_init(service_names_and_types, 0, allocator);
}

rmw_ret_t rmw_get_publisher_names_and_types_by_node(
    const rmw_node_t *node, rcutils_allocator_t *allocator,
    const char *node_name, const char *node_namespace, bool no_demangle,
    rmw_names_and_types_t *topic_names_and_types) {
  (void)node;
  (void)node_name;
  (void)node_namespace;
  (void)no_demangle;
  if (!allocator || !topic_names_and_types) {
    RMW_SET_ERROR_MSG("allocator or topic_names_and_types is null");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // Return empty list
  return rmw_names_and_types_init(topic_names_and_types, 0, allocator);
}

rmw_ret_t rmw_get_subscriber_names_and_types_by_node(
    const rmw_node_t *node, rcutils_allocator_t *allocator,
    const char *node_name, const char *node_namespace, bool no_demangle,
    rmw_names_and_types_t *topic_names_and_types) {
  (void)node;
  (void)node_name;
  (void)node_namespace;
  (void)no_demangle;
  if (!allocator || !topic_names_and_types) {
    RMW_SET_ERROR_MSG("allocator or topic_names_and_types is null");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // Return empty list
  return rmw_names_and_types_init(topic_names_and_types, 0, allocator);
}

rmw_ret_t rmw_get_service_names_and_types_by_node(
    const rmw_node_t *node, rcutils_allocator_t *allocator,
    const char *node_name, const char *node_namespace,
    rmw_names_and_types_t *service_names_and_types) {
  (void)node;
  (void)node_name;
  (void)node_namespace;
  if (!allocator || !service_names_and_types) {
    RMW_SET_ERROR_MSG("allocator or service_names_and_types is null");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // Return empty list
  return rmw_names_and_types_init(service_names_and_types, 0, allocator);
}

rmw_ret_t rmw_get_client_names_and_types_by_node(
    const rmw_node_t *node, rcutils_allocator_t *allocator,
    const char *node_name, const char *node_namespace,
    rmw_names_and_types_t *service_names_and_types) {
  (void)node;
  (void)node_name;
  (void)node_namespace;
  if (!allocator || !service_names_and_types) {
    RMW_SET_ERROR_MSG("allocator or service_names_and_types is null");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // Return empty list
  return rmw_names_and_types_init(service_names_and_types, 0, allocator);
}

} // extern "C"
