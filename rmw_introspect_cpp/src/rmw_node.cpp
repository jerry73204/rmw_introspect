#include "rcutils/macros.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw_introspect/data.hpp"
#include "rmw_introspect/forwarding.hpp"
#include "rmw_introspect/identifier.hpp"
#include "rmw_introspect/mode.hpp"
#include "rmw_introspect/visibility_control.h"
#include "rmw_introspect/wrappers.hpp"
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
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(context, nullptr);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(name, nullptr);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(namespace_, nullptr);

  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(context, context->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return nullptr);

  // Record node in introspection data
  rmw_introspect::IntrospectionData::instance().record_node(name, namespace_);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    auto *ctx_wrapper =
        static_cast<rmw_introspect::ContextWrapper *>(context->impl);
    rmw_context_t *real_context = ctx_wrapper->real_context;

    // Forward to real RMW - note: rmw_create_node takes rmw_node_options_t in
    // newer ROS 2 For Humble, it should be: rmw_create_node(context, name,
    // namespace_, domain_id) But the signature here is the older one without
    // options Let me check what's available
    rmw_node_t *real_node =
        g_real_rmw->create_node(real_context, name, namespace_, nullptr);

    if (!real_node) {
      return nullptr;
    }

    // Create wrapper
    auto *wrapper = new (std::nothrow)
        rmw_introspect::NodeWrapper(real_node, name, namespace_);
    if (!wrapper) {
      g_real_rmw->destroy_node(real_node);
      RMW_SET_ERROR_MSG("failed to allocate node wrapper");
      return nullptr;
    }

    // Create our node structure
    rmw_node_t *node = new (std::nothrow) rmw_node_t;
    if (!node) {
      g_real_rmw->destroy_node(real_node);
      delete wrapper;
      RMW_SET_ERROR_MSG("failed to allocate node");
      return nullptr;
    }

    node->implementation_identifier = rmw_introspect_cpp_identifier;
    node->data = wrapper;
    node->name = real_node->name;
    node->namespace_ = real_node->namespace_;
    node->context = context;

    return node;
  }

  // Recording-only mode (existing behavior)
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

  return node;
}

// Destroy node
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_destroy_node(rmw_node_t *node) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(node, node->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    auto *wrapper = static_cast<rmw_introspect::NodeWrapper *>(node->data);
    if (wrapper && wrapper->real_node) {
      rmw_ret_t ret = g_real_rmw->destroy_node(wrapper->real_node);
      if (ret != RMW_RET_OK) {
        return ret;
      }
    }
    delete wrapper;
    delete node;
    return RMW_RET_OK;
  }

  // Recording-only mode
  auto impl = static_cast<rmw_node_impl_t *>(node->data);
  if (impl) {
    delete[] impl->name;
    delete[] impl->namespace_;
    delete impl;
  }
  delete node;

  return RMW_RET_OK;
}

// Get node's graph guard condition
RMW_INTROSPECT_PUBLIC
const rmw_guard_condition_t *
rmw_node_get_graph_guard_condition(const rmw_node_t *node) {
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(node, nullptr);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(node, node->implementation_identifier,
                                   rmw_introspect_cpp_identifier,
                                   return nullptr);

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    rmw_node_t *real_node = unwrap_node(node);
    if (!real_node) {
      RMW_SET_ERROR_MSG("failed to unwrap node");
      return nullptr;
    }
    return g_real_rmw->node_get_graph_guard_condition(real_node);
  }

  // Recording-only mode: return stub
  static rmw_guard_condition_t stub_guard_condition = {
      rmw_introspect_cpp_identifier, nullptr};

  return &stub_guard_condition;
}

} // extern "C"
