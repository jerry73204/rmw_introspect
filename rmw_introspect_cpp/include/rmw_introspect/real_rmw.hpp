#ifndef RMW_INTROSPECT__REAL_RMW_HPP_
#define RMW_INTROSPECT__REAL_RMW_HPP_

#include "rmw/init.h"
#include "rmw/rmw.h"
#include "rmw/types.h"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_runtime_c/service_type_support_struct.h"
#include <string>

namespace rmw_introspect {

/// Container for real RMW function pointers loaded via dlopen
class RealRMW {
public:
  RealRMW();
  ~RealRMW();

  /// Load RMW implementation from shared library
  /// @param implementation_name Name without "lib" prefix or ".so" suffix
  ///                            (e.g., "rmw_fastrtps_cpp")
  /// @return true on success, false on failure
  bool load(const char *implementation_name);

  /// Unload the library
  void unload();

  /// Get the implementation name
  const std::string &get_name() const { return name_; }

  /// Check if loaded
  bool is_loaded() const { return lib_handle_ != nullptr; }

  // --- Function Pointers (public for direct access) ---

  // Core
  const char *(*get_implementation_identifier)(void);
  const char *(*get_serialization_format)(void);

  // Init
  rmw_ret_t (*init_options_init)(rmw_init_options_t *, rcutils_allocator_t);
  rmw_ret_t (*init_options_copy)(const rmw_init_options_t *,
                                 rmw_init_options_t *);
  rmw_ret_t (*init_options_fini)(rmw_init_options_t *);
  rmw_ret_t (*init)(const rmw_init_options_t *, rmw_context_t *);
  rmw_ret_t (*shutdown)(rmw_context_t *);
  rmw_ret_t (*context_fini)(rmw_context_t *);

  // Node
  rmw_node_t *(*create_node)(rmw_context_t *, const char *, const char *,
                             const rmw_node_options_t *);
  rmw_ret_t (*destroy_node)(rmw_node_t *);
  const rmw_guard_condition_t *(*node_get_graph_guard_condition)(
      const rmw_node_t *);

  // Publisher
  rmw_publisher_t *(*create_publisher)(const rmw_node_t *,
                                       const rosidl_message_type_support_t *,
                                       const char *, const rmw_qos_profile_t *,
                                       const rmw_publisher_options_t *);
  rmw_ret_t (*destroy_publisher)(rmw_node_t *, rmw_publisher_t *);
  rmw_ret_t (*publish)(const rmw_publisher_t *, const void *,
                       rmw_publisher_allocation_t *);
  rmw_ret_t (*publish_serialized_message)(const rmw_publisher_t *,
                                          const rmw_serialized_message_t *,
                                          rmw_publisher_allocation_t *);
  rmw_ret_t (*publisher_get_actual_qos)(const rmw_publisher_t *,
                                        rmw_qos_profile_t *);
  rmw_ret_t (*publisher_count_matched_subscriptions)(const rmw_publisher_t *,
                                                     size_t *);
  rmw_ret_t (*publisher_assert_liveliness)(const rmw_publisher_t *);
  rmw_ret_t (*publisher_wait_for_all_acked)(const rmw_publisher_t *,
                                            rmw_time_t);

  // Subscription
  rmw_subscription_t *(*create_subscription)(
      const rmw_node_t *, const rosidl_message_type_support_t *, const char *,
      const rmw_qos_profile_t *, const rmw_subscription_options_t *);
  rmw_ret_t (*destroy_subscription)(rmw_node_t *, rmw_subscription_t *);
  rmw_ret_t (*take)(const rmw_subscription_t *, void *, bool *,
                    rmw_subscription_allocation_t *);
  rmw_ret_t (*take_with_info)(const rmw_subscription_t *, void *, bool *,
                              rmw_message_info_t *,
                              rmw_subscription_allocation_t *);
  rmw_ret_t (*take_serialized_message)(const rmw_subscription_t *,
                                       rmw_serialized_message_t *, bool *,
                                       rmw_subscription_allocation_t *);
  rmw_ret_t (*take_serialized_message_with_info)(
      const rmw_subscription_t *, rmw_serialized_message_t *, bool *,
      rmw_message_info_t *, rmw_subscription_allocation_t *);
  rmw_ret_t (*subscription_get_actual_qos)(const rmw_subscription_t *,
                                           rmw_qos_profile_t *);
  rmw_ret_t (*subscription_count_matched_publishers)(const rmw_subscription_t *,
                                                     size_t *);

  // Service
  rmw_service_t *(*create_service)(const rmw_node_t *,
                                   const rosidl_service_type_support_t *,
                                   const char *, const rmw_qos_profile_t *);
  rmw_ret_t (*destroy_service)(rmw_node_t *, rmw_service_t *);
  rmw_ret_t (*take_request)(const rmw_service_t *, rmw_service_info_t *, void *,
                            bool *);
  rmw_ret_t (*send_response)(const rmw_service_t *, rmw_request_id_t *, void *);

  // Client
  rmw_client_t *(*create_client)(const rmw_node_t *,
                                 const rosidl_service_type_support_t *,
                                 const char *, const rmw_qos_profile_t *);
  rmw_ret_t (*destroy_client)(rmw_node_t *, rmw_client_t *);
  rmw_ret_t (*send_request)(const rmw_client_t *, const void *, int64_t *);
  rmw_ret_t (*take_response)(const rmw_client_t *, rmw_service_info_t *, void *,
                             bool *);

  // Guard Condition
  rmw_guard_condition_t *(*create_guard_condition)(rmw_context_t *);
  rmw_ret_t (*destroy_guard_condition)(rmw_guard_condition_t *);
  rmw_ret_t (*trigger_guard_condition)(const rmw_guard_condition_t *);

  // Wait Set
  rmw_wait_set_t *(*create_wait_set)(rmw_context_t *, size_t);
  rmw_ret_t (*destroy_wait_set)(rmw_wait_set_t *);
  rmw_ret_t (*wait)(rmw_subscriptions_t *, rmw_guard_conditions_t *,
                    rmw_services_t *, rmw_clients_t *, rmw_events_t *,
                    rmw_wait_set_t *, const rmw_time_t *);

  // Graph
  rmw_ret_t (*get_node_names)(const rmw_node_t *, rcutils_string_array_t *,
                              rcutils_string_array_t *);
  rmw_ret_t (*get_node_names_with_enclaves)(const rmw_node_t *,
                                            rcutils_string_array_t *,
                                            rcutils_string_array_t *,
                                            rcutils_string_array_t *);
  rmw_ret_t (*count_publishers)(const rmw_node_t *, const char *, size_t *);
  rmw_ret_t (*count_subscribers)(const rmw_node_t *, const char *, size_t *);
  rmw_ret_t (*get_gid_for_publisher)(const rmw_publisher_t *, rmw_gid_t *);
  rmw_ret_t (*compare_gids_equal)(const rmw_gid_t *, const rmw_gid_t *, bool *);
  rmw_ret_t (*get_gid_for_client)(const rmw_client_t *, rmw_gid_t *);

  // Serialization
  rmw_ret_t (*serialize)(const void *, const rosidl_message_type_support_t *,
                         rmw_serialized_message_t *);
  rmw_ret_t (*deserialize)(const rmw_serialized_message_t *,
                           const rosidl_message_type_support_t *, void *);
  rmw_ret_t (*get_serialized_message_size)(
      const void *, const rosidl_message_type_support_t *, size_t *);

  // Topic and service names/types
  rmw_ret_t (*get_topic_names_and_types)(const rmw_node_t *,
                                         rcutils_allocator_t *, bool,
                                         rmw_names_and_types_t *);
  rmw_ret_t (*get_service_names_and_types)(const rmw_node_t *,
                                           rcutils_allocator_t *,
                                           rmw_names_and_types_t *);
  rmw_ret_t (*get_publisher_names_and_types_by_node)(const rmw_node_t *,
                                                     rcutils_allocator_t *,
                                                     const char *, const char *,
                                                     bool,
                                                     rmw_names_and_types_t *);
  rmw_ret_t (*get_subscriber_names_and_types_by_node)(const rmw_node_t *,
                                                      rcutils_allocator_t *,
                                                      const char *,
                                                      const char *, bool,
                                                      rmw_names_and_types_t *);
  rmw_ret_t (*get_service_names_and_types_by_node)(const rmw_node_t *,
                                                   rcutils_allocator_t *,
                                                   const char *, const char *,
                                                   rmw_names_and_types_t *);
  rmw_ret_t (*get_client_names_and_types_by_node)(const rmw_node_t *,
                                                  rcutils_allocator_t *,
                                                  const char *, const char *,
                                                  rmw_names_and_types_t *);

  // QoS queries for service/client
  rmw_ret_t (*service_request_subscription_get_actual_qos)(
      const rmw_service_t *, rmw_qos_profile_t *);
  rmw_ret_t (*service_response_publisher_get_actual_qos)(const rmw_service_t *,
                                                         rmw_qos_profile_t *);
  rmw_ret_t (*client_request_publisher_get_actual_qos)(const rmw_client_t *,
                                                       rmw_qos_profile_t *);
  rmw_ret_t (*client_response_subscription_get_actual_qos)(const rmw_client_t *,
                                                           rmw_qos_profile_t *);

  // Event handling
  rmw_ret_t (*publisher_event_init)(rmw_event_t *, const rmw_publisher_t *,
                                    rmw_event_type_t);
  rmw_ret_t (*subscription_event_init)(rmw_event_t *,
                                       const rmw_subscription_t *,
                                       rmw_event_type_t);
  rmw_ret_t (*take_event)(const rmw_event_t *, void *, bool *);
  rmw_ret_t (*event_fini)(rmw_event_t *);

private:
  void *lib_handle_;
  std::string name_;

  /// Template helper to load a symbol from the shared library
  template <typename FuncPtr>
  bool load_symbol(FuncPtr &func_ptr, const char *symbol_name,
                   bool required = true);
};

} // namespace rmw_introspect

#endif // RMW_INTROSPECT__REAL_RMW_HPP_
