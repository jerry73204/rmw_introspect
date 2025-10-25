#include "rcutils/macros.h"
#include "rmw/error_handling.h"
#include "rmw/features.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw_introspect/identifier.hpp"
#include "rmw_introspect/visibility_control.h"

extern "C" {

// Query feature support - always return false/unsupported
RMW_INTROSPECT_PUBLIC
bool rmw_feature_supported(rmw_feature_t feature) {
  (void)feature;

  // All features are unsupported in introspection mode
  return false;
}

} // extern "C"
