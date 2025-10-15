#include "rmw/rmw.h"
#include "rmw/error_handling.h"
#include "rmw/init.h"
#include "rmw/init_options.h"
#include "rcutils/macros.h"
#include "rcutils/types/rcutils_ret.h"
#include "rmw_introspect/identifier.hpp"
#include "rmw_introspect/visibility_control.h"
#include "rmw_introspect/data.hpp"
#include <cstdlib>
#include <string>

// Define the identifier symbol (declared in identifier.hpp)
extern "C" const char * const rmw_introspect_cpp_identifier = "rmw_introspect_cpp";

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
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(options, RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(context, RMW_RET_INVALID_ARGUMENT);

  if (options->implementation_identifier != rmw_introspect_cpp_identifier) {
    RMW_SET_ERROR_MSG("expected options to be initialized");
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION;
  }

  context->instance_id = options->instance_id;
  context->implementation_identifier = rmw_introspect_cpp_identifier;
  context->actual_domain_id = options->domain_id;
  context->impl = nullptr;  // No implementation-specific data

  return RMW_RET_OK;
}

// Shutdown RMW
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_shutdown(rmw_context_t * context)
{
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

  return RMW_RET_OK;
}

// Finalize context
RMW_INTROSPECT_PUBLIC
rmw_ret_t rmw_context_fini(rmw_context_t * context)
{
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(context, RMW_RET_INVALID_ARGUMENT);

  if (context->implementation_identifier != rmw_introspect_cpp_identifier) {
    RMW_SET_ERROR_MSG("expected context to be initialized");
    return RMW_RET_INCORRECT_RMW_IMPLEMENTATION;
  }

  *context = rmw_get_zero_initialized_context();
  return RMW_RET_OK;
}

}  // extern "C"
