#include "rmw/rmw.h"
#include "rmw/error_handling.h"
#include "rmw/init.h"
#include "rmw/init_options.h"
#include "rcutils/macros.h"
#include "rcutils/types/rcutils_ret.h"
#include "rcutils/logging_macros.h"
#include "rmw_introspect/identifier.hpp"
#include "rmw_introspect/visibility_control.h"
#include "rmw_introspect/data.hpp"
#include "rmw_introspect/real_rmw.hpp"
#include "rmw_introspect/wrappers.hpp"
#include "rmw_introspect/mode.hpp"
#include <cstdlib>
#include <string>
#include <mutex>
#include <atomic>

// Define the identifier symbol (declared in identifier.hpp)
extern "C" const char * const rmw_introspect_cpp_identifier = "rmw_introspect_cpp";

// Global state for intermediate layer mode
namespace rmw_introspect {
namespace internal {

RealRMW* g_real_rmw = nullptr;
std::mutex g_init_mutex;
std::atomic<size_t> g_context_count{0};

}  // namespace internal
}  // namespace rmw_introspect

extern "C"
{

// Implementation identifier
RMW_INTROSPECT_PUBLIC
const char * rmw_get_implementation_identifier(void)
{
  return rmw_introspect_cpp_identifier;
}

// Serialization format
RMW_INTROSPECT_PUBLIC
const char * rmw_get_serialization_format(void)
{
  return "introspect";  // Not actually used
}

// Initialize init options
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_init_options_init(
  rmw_init_options_t * init_options,
  rcutils_allocator_t allocator)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(init_options, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ALLOCATOR(&allocator, return RMW_RET_INVALID_ARGUMENT);

  if (init_options->implementation_identifier != nullptr) {
    RMW_SET_ERROR_MSG("expected zero-initialized init_options");
    return RMW_RET_INVALID_ARGUMENT;
  }

  init_options->instance_id = 0;
  init_options->implementation_identifier = rmw_introspect_cpp_identifier;
  init_options->allocator = allocator;
  init_options->impl = nullptr;  // No implementation-specific data needed
  init_options->enclave = nullptr;
  init_options->domain_id = RMW_DEFAULT_DOMAIN_ID;
  init_options->security_options = rmw_get_zero_initialized_security_options();
  init_options->localhost_only = RMW_LOCALHOST_ONLY_DEFAULT;

  return RMW_RET_OK;
}

// Copy init options
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_init_options_copy(
  const rmw_init_options_t * src,
  rmw_init_options_t * dst)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(src, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(dst, RMW_RET_INVALID_ARGUMENT);

  if (src->implementation_identifier != rmw_introspect_cpp_identifier) {
    RMW_SET_ERROR_MSG("expected src to be initialized");
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION;
  }

  if (dst->implementation_identifier != nullptr) {
    RMW_SET_ERROR_MSG("expected dst to be zero-initialized");
    return RMW_RET_INVALID_ARGUMENT;
  }

  *dst = *src;
  return RMW_RET_OK;
}

// Finalize init options
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_init_options_fini(rmw_init_options_t * init_options)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(init_options, RMW_RET_INVALID_ARGUMENT);

  if (init_options->implementation_identifier != rmw_introspect_cpp_identifier) {
    RMW_SET_ERROR_MSG("expected init_options to be initialized");
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION;
  }

  *init_options = rmw_get_zero_initialized_init_options();
  return RMW_RET_OK;
}

// Initialize RMW
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_init(
  const rmw_init_options_t * options,
  rmw_context_t * context)
{
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(options, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(context, RMW_RET_INVALID_ARGUMENT);

  if (options->implementation_identifier != rmw_introspect_cpp_identifier) {
    RMW_SET_ERROR_MSG("expected options to be initialized");
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION;
  }

  std::lock_guard<std::mutex> lock(g_init_mutex);

  // First initialization? Check if we should load real RMW
  if (g_context_count == 0) {
    const char* delegate_to = std::getenv("RMW_INTROSPECT_DELEGATE_TO");
    if (delegate_to && *delegate_to) {
      // Verbose logging
      const char* verbose_env = std::getenv("RMW_INTROSPECT_VERBOSE");
      bool verbose = verbose_env && (*verbose_env == '1' || *verbose_env == 't' || *verbose_env == 'T');

      if (verbose) {
        RCUTILS_LOG_INFO_NAMED("rmw_introspect", "Attempting to load real RMW: %s", delegate_to);
      }

      g_real_rmw = new rmw_introspect::RealRMW;
      if (!g_real_rmw->load(delegate_to)) {
        delete g_real_rmw;
        g_real_rmw = nullptr;
        return RMW_RET_ERROR;
      }

      if (verbose) {
        RCUTILS_LOG_INFO_NAMED("rmw_introspect", "Real RMW loaded successfully: %s", delegate_to);
      }
    }
  }

  ++g_context_count;

  // Intermediate mode: forward to real RMW
  if (is_intermediate_mode()) {
    // Create wrapper
    auto* wrapper = new rmw_introspect::ContextWrapper;
    wrapper->real_rmw = g_real_rmw;
    wrapper->real_rmw_name = g_real_rmw->get_name();
    wrapper->real_context = new rmw_context_t;
    *wrapper->real_context = rmw_get_zero_initialized_context();

    // Create real init options with correct identifier
    rmw_init_options_t real_options = *options;
    real_options.implementation_identifier = g_real_rmw->get_implementation_identifier();

    // Forward to real RMW
    rmw_ret_t ret = g_real_rmw->init(&real_options, wrapper->real_context);
    if (ret != RMW_RET_OK) {
      delete wrapper->real_context;
      delete wrapper;
      --g_context_count;
      return ret;
    }

    // Set up our context
    context->implementation_identifier = rmw_introspect_cpp_identifier;
    context->impl = wrapper;
    context->instance_id = options->instance_id;
    context->actual_domain_id = wrapper->real_context->actual_domain_id;

    return RMW_RET_OK;
  } else {
    // Recording-only mode (existing behavior)
    context->instance_id = options->instance_id;
    context->implementation_identifier = rmw_introspect_cpp_identifier;
    context->actual_domain_id = options->domain_id;
    context->impl = nullptr;

    return RMW_RET_OK;
  }
}

// Shutdown RMW
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_shutdown(rmw_context_t * context)
{
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(context, RMW_RET_INVALID_ARGUMENT);

  if (context->implementation_identifier != rmw_introspect_cpp_identifier) {
    RMW_SET_ERROR_MSG("expected context to be initialized");
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION;
  }

  // Check if auto-export is enabled
  const char * auto_export_env = std::getenv("RMW_INTROSPECT_AUTO_EXPORT");
  bool auto_export = true;  // Default to enabled
  if (auto_export_env && std::string(auto_export_env) == "0") {
    auto_export = false;
  }

  // Export introspection data if enabled
  if (auto_export) {
    const char * output_path = std::getenv("RMW_INTROSPECT_OUTPUT");
    if (output_path) {
      rmw_introspect::IntrospectionData::instance().export_to_json(output_path);
    }
  }

  // If in intermediate mode, forward shutdown to real RMW
  if (is_intermediate_mode() && context->impl) {
    auto* wrapper = static_cast<rmw_introspect::ContextWrapper*>(context->impl);
    if (wrapper->real_context) {
      return g_real_rmw->shutdown(wrapper->real_context);
    }
  }

  return RMW_RET_OK;
}

// Finalize context
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_context_fini(rmw_context_t * context)
{
  using namespace rmw_introspect::internal;

  RCUTILS_CHECK_ARGUMENT_FOR_NULL(context, RMW_RET_INVALID_ARGUMENT);

  if (context->implementation_identifier != rmw_introspect_cpp_identifier) {
    RMW_SET_ERROR_MSG("expected context to be initialized");
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION;
  }

  std::lock_guard<std::mutex> lock(g_init_mutex);

  // If in intermediate mode, clean up wrapper and forward to real RMW
  if (is_intermediate_mode() && context->impl) {
    auto* wrapper = static_cast<rmw_introspect::ContextWrapper*>(context->impl);
    if (wrapper->real_context) {
      rmw_ret_t ret = g_real_rmw->context_fini(wrapper->real_context);
      delete wrapper->real_context;
      if (ret != RMW_RET_OK) {
        return ret;
      }
    }
    delete wrapper;
  }

  *context = rmw_get_zero_initialized_context();

  // Decrement context count and unload real RMW if this is the last context
  --g_context_count;
  if (g_context_count == 0 && g_real_rmw) {
    delete g_real_rmw;
    g_real_rmw = nullptr;
  }

  return RMW_RET_OK;
}

}  // extern "C"
