#include "rcutils/allocator.h"
#include "rcutils/types/string_array.h"
#include "rmw/error_handling.h"
#include "rmw/get_node_info_and_types.h"
#include "rmw/names_and_types.h"
#include "rmw/rmw.h"
#include "rmw_introspect/forwarding.hpp"
#include "rmw_introspect/identifier.hpp"
#include "rmw_introspect/mode.hpp"
#include "rmw_introspect/real_rmw.hpp"
#include "rmw_introspect/wrappers.hpp"

extern "C" {

rmw_ret_t rmw_count_publishers(const rmw_node_t *node, const char *topic_name,
                               size_t *count) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(topic_name, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(count, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_node_t *real_node = unwrap_node(node);
    if (!real_node) {
      RMW_SET_ERROR_MSG("failed to unwrap node");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->count_publishers(real_node, topic_name, count);
  }

  // Recording-only mode: return 0
  *count = 0;
  return RMW_RET_OK;
}

rmw_ret_t rmw_count_subscribers(const rmw_node_t *node, const char *topic_name,
                                size_t *count) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(topic_name, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(count, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_node_t *real_node = unwrap_node(node);
    if (!real_node) {
      RMW_SET_ERROR_MSG("failed to unwrap node");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->count_subscribers(real_node, topic_name, count);
  }

  // Recording-only mode: return 0
  *count = 0;
  return RMW_RET_OK;
}

rmw_ret_t rmw_get_node_names(const rmw_node_t *node,
                             rcutils_string_array_t *node_names,
                             rcutils_string_array_t *node_namespaces) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node_names, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node_namespaces, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_node_t *real_node = unwrap_node(node);
    if (!real_node) {
      RMW_SET_ERROR_MSG("failed to unwrap node");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->get_node_names(real_node, node_names, node_namespaces);
  }

  // Recording-only mode: return empty arrays
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
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node_names, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node_namespaces, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(enclaves, RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_node_t *real_node = unwrap_node(node);
    if (!real_node) {
      RMW_SET_ERROR_MSG("failed to unwrap node");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->get_node_names_with_enclaves(real_node, node_names,
                                                    node_namespaces, enclaves);
  }

  // Recording-only mode: return empty arrays
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
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(allocator, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(topic_names_and_types,
                                  RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_node_t *real_node = unwrap_node(node);
    if (!real_node) {
      RMW_SET_ERROR_MSG("failed to unwrap node");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->get_topic_names_and_types(
        real_node, allocator, no_demangle, topic_names_and_types);
  }

  // Recording-only mode: return empty list
  return rmw_names_and_types_init(topic_names_and_types, 0, allocator);
}

rmw_ret_t rmw_get_service_names_and_types(
    const rmw_node_t *node, rcutils_allocator_t *allocator,
    rmw_names_and_types_t *service_names_and_types) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(allocator, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(service_names_and_types,
                                  RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_node_t *real_node = unwrap_node(node);
    if (!real_node) {
      RMW_SET_ERROR_MSG("failed to unwrap node");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->get_service_names_and_types(real_node, allocator,
                                                   service_names_and_types);
  }

  // Recording-only mode: return empty list
  return rmw_names_and_types_init(service_names_and_types, 0, allocator);
}

rmw_ret_t rmw_get_publisher_names_and_types_by_node(
    const rmw_node_t *node, rcutils_allocator_t *allocator,
    const char *node_name, const char *node_namespace, bool no_demangle,
    rmw_names_and_types_t *topic_names_and_types) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(allocator, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node_name, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node_namespace, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(topic_names_and_types,
                                  RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_node_t *real_node = unwrap_node(node);
    if (!real_node) {
      RMW_SET_ERROR_MSG("failed to unwrap node");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->get_publisher_names_and_types_by_node(
        real_node, allocator, node_name, node_namespace, no_demangle,
        topic_names_and_types);
  }

  // Recording-only mode: return empty list
  return rmw_names_and_types_init(topic_names_and_types, 0, allocator);
}

rmw_ret_t rmw_get_subscriber_names_and_types_by_node(
    const rmw_node_t *node, rcutils_allocator_t *allocator,
    const char *node_name, const char *node_namespace, bool no_demangle,
    rmw_names_and_types_t *topic_names_and_types) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(allocator, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node_name, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node_namespace, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(topic_names_and_types,
                                  RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_node_t *real_node = unwrap_node(node);
    if (!real_node) {
      RMW_SET_ERROR_MSG("failed to unwrap node");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->get_subscriber_names_and_types_by_node(
        real_node, allocator, node_name, node_namespace, no_demangle,
        topic_names_and_types);
  }

  // Recording-only mode: return empty list
  return rmw_names_and_types_init(topic_names_and_types, 0, allocator);
}

rmw_ret_t rmw_get_service_names_and_types_by_node(
    const rmw_node_t *node, rcutils_allocator_t *allocator,
    const char *node_name, const char *node_namespace,
    rmw_names_and_types_t *service_names_and_types) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(allocator, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node_name, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node_namespace, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(service_names_and_types,
                                  RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_node_t *real_node = unwrap_node(node);
    if (!real_node) {
      RMW_SET_ERROR_MSG("failed to unwrap node");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->get_service_names_and_types_by_node(
        real_node, allocator, node_name, node_namespace,
        service_names_and_types);
  }

  // Recording-only mode: return empty list
  return rmw_names_and_types_init(service_names_and_types, 0, allocator);
}

rmw_ret_t rmw_get_client_names_and_types_by_node(
    const rmw_node_t *node, rcutils_allocator_t *allocator,
    const char *node_name, const char *node_namespace,
    rmw_names_and_types_t *service_names_and_types) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(allocator, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node_name, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node_namespace, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(service_names_and_types,
                                  RMW_RET_INVALID_ARGUMENT);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_node_t *real_node = unwrap_node(node);
    if (!real_node) {
      RMW_SET_ERROR_MSG("failed to unwrap node");
      return RMW_RET_ERROR;
    }
    return g_real_rmw->get_client_names_and_types_by_node(
        real_node, allocator, node_name, node_namespace,
        service_names_and_types);
  }

  // Recording-only mode: return empty list
  return rmw_names_and_types_init(service_names_and_types, 0, allocator);
}

} // extern "C"
