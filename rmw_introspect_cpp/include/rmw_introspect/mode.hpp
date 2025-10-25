#ifndef RMW_INTROSPECT__MODE_HPP_
#define RMW_INTROSPECT__MODE_HPP_

namespace rmw_introspect {

class RealRMW;

namespace internal {

// Global state - initialized in rmw_init.cpp
extern RealRMW *g_real_rmw;

/// Check if running in intermediate layer mode (forwarding to real RMW)
inline bool is_intermediate_mode() { return g_real_rmw != nullptr; }

/// Check if running in recording-only mode (no real RMW)
inline bool is_recording_only_mode() { return g_real_rmw == nullptr; }

} // namespace internal
} // namespace rmw_introspect

#endif // RMW_INTROSPECT__MODE_HPP_
