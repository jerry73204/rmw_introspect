#include "rcutils/macros.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw_introspect/data.hpp"
#include "rmw_introspect/identifier.hpp"
#include "rmw_introspect/visibility_control.h"
#include <cstring>
#include <new>

// Internal node structure
struct rmw_node_impl_t {
  char *name;
  char *namespace_;
};

extern "C" {

// Create node
RMW_INTROSPECT_PUBLIC
rmw_node_t *rmw_create_node(rmw_context_t *context, const char *name,
                            const char *namespace_) {
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(context, nullptr);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(name, nullptr);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(namespace_, nullptr);

  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(context, context->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return nullptr);

  // Allocate node structure
  rmw_node_t *node = new (std::nothrow) rmw_node_t;
  if (!node) {
    RMW_SET_ERROR_MSG("failed to allocate node");
    return nullptr;
  }

  // Allocate implementation structure
  rmw_node_impl_t *impl = new (std::nothrow) rmw_node_impl_t;
  if (!impl) {
    delete node;
    RMW_SET_ERROR_MSG("failed to allocate node impl");
    return nullptr;
  }

  // Copy name and namespace
  impl->name = new (std::nothrow) char[strlen(name) + 1];
  impl->namespace_ = new (std::nothrow) char[strlen(namespace_) + 1];

  if (!impl->name || !impl->namespace_) {
    delete[] impl->name;
    delete[] impl->namespace_;
    delete impl;
    delete node;
    RMW_SET_ERROR_MSG("failed to allocate node name/namespace");
    return nullptr;
  }

  strcpy(impl->name, name);
  strcpy(impl->namespace_, namespace_);

  // Initialize node
  node->implementation_identifier = rmw_introspect_cpp_identifier;
  node->data = impl;
  node->name = impl->name;
  node->namespace_ = impl->namespace_;
  node->context = context;

  // Record node in introspection data
  rmw_introspect::IntrospectionData::instance().record_node(name, namespace_);

  return node;
}

// Destroy node
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_destroy_node(rmw_node_t *node) {
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(node, node->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  auto impl = static_cast<rmw_node_impl_t *>(node->data);
  if (impl) {
    delete[] impl->name;
    delete[] impl->namespace_;
    delete impl;
  }
  delete node;

  return RMW_RET_OK;
}

// Get node's graph guard condition (stub)
RMW_INTROSPECT_PUBLIC
const rmw_guard_condition_t *
rmw_node_get_graph_guard_condition(const rmw_node_t *node) {
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node, nullptr);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(node, node->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return nullptr);

  // Return a static stub guard condition
  static rmw_guard_condition_t stub_guard_condition = {
      rmw_introspect_cpp_identifier, nullptr};

  return &stub_guard_condition;
}

} // extern "C"
