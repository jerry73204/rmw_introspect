#include "rmw_introspect/real_rmw.hpp"
#include "rcutils/logging_macros.h"
#include "rmw/error_handling.h"
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>

namespace rmw_introspect {

RealRMW::RealRMW()
    : lib_handle_(nullptr), name_(),
      // Core
      get_implementation_identifier(nullptr), get_serialization_format(nullptr),
      // Init
      init_options_init(nullptr), init_options_copy(nullptr),
      init_options_fini(nullptr), init(nullptr), shutdown(nullptr),
      context_fini(nullptr),
      // Node
      create_node(nullptr), destroy_node(nullptr),
      node_get_graph_guard_condition(nullptr),
      // Publisher
      create_publisher(nullptr), destroy_publisher(nullptr), publish(nullptr),
      publish_serialized_message(nullptr), publisher_get_actual_qos(nullptr),
      publisher_count_matched_subscriptions(nullptr),
      publisher_assert_liveliness(nullptr),
      publisher_wait_for_all_acked(nullptr),
      // Subscription
      create_subscription(nullptr), destroy_subscription(nullptr),
      take(nullptr), take_with_info(nullptr), take_serialized_message(nullptr),
      take_serialized_message_with_info(nullptr),
      subscription_get_actual_qos(nullptr),
      subscription_count_matched_publishers(nullptr),
      // Service
      create_service(nullptr), destroy_service(nullptr), take_request(nullptr),
      send_response(nullptr),
      // Client
      create_client(nullptr), destroy_client(nullptr), send_request(nullptr),
      take_response(nullptr),
      // Guard Condition
      create_guard_condition(nullptr), destroy_guard_condition(nullptr),
      trigger_guard_condition(nullptr),
      // Wait Set
      create_wait_set(nullptr), destroy_wait_set(nullptr), wait(nullptr),
      // Graph
      get_node_names(nullptr), get_node_names_with_enclaves(nullptr),
      count_publishers(nullptr), count_subscribers(nullptr),
      get_gid_for_publisher(nullptr), compare_gids_equal(nullptr),
      get_gid_for_client(nullptr),
      // Serialization
      serialize(nullptr), deserialize(nullptr),
      get_serialized_message_size(nullptr),
      // Topic and service names/types
      get_topic_names_and_types(nullptr), get_service_names_and_types(nullptr),
      get_publisher_names_and_types_by_node(nullptr),
      get_subscriber_names_and_types_by_node(nullptr),
      get_service_names_and_types_by_node(nullptr),
      get_client_names_and_types_by_node(nullptr),
      // QoS queries for service/client
      service_request_subscription_get_actual_qos(nullptr),
      service_response_publisher_get_actual_qos(nullptr),
      client_request_publisher_get_actual_qos(nullptr),
      client_response_subscription_get_actual_qos(nullptr),
      // Event handling
      publisher_event_init(nullptr), subscription_event_init(nullptr),
      take_event(nullptr), event_fini(nullptr) {}

RealRMW::~RealRMW() { unload(); }

bool RealRMW::load(const char *implementation_name) {
  if (lib_handle_) {
    RMW_SET_ERROR_MSG("RealRMW already loaded");
    return false;
  }

  // Validate name
  if (!implementation_name || !*implementation_name) {
    RMW_SET_ERROR_MSG("Invalid implementation name");
    return false;
  }

  if (strncmp(implementation_name, "rmw_", 4) != 0) {
    RMW_SET_ERROR_MSG("Implementation name must start with 'rmw_'");
    return false;
  }

  // Construct library name
  std::string lib_name = "lib";
  lib_name += implementation_name;
  lib_name += ".so";

  // Check verbose mode
  const char *verbose_env = std::getenv("RMW_INTROSPECT_VERBOSE");
  bool verbose = verbose_env && (*verbose_env == '1' || *verbose_env == 't' ||
                                 *verbose_env == 'T');

  if (verbose) {
    RCUTILS_LOG_INFO_NAMED("rmw_introspect", "Attempting to load %s",
                           lib_name.c_str());
  }

  // Load library
  lib_handle_ = dlopen(lib_name.c_str(), RTLD_LAZY | RTLD_LOCAL);
  if (!lib_handle_) {
    char error_msg[512];
    snprintf(error_msg, sizeof(error_msg), "Failed to load %s: %s",
             lib_name.c_str(), dlerror());
    RMW_SET_ERROR_MSG(error_msg);
    return false;
  }

  name_ = implementation_name;

  // Load all symbols
  bool success = true;

  // Core
  success &= load_symbol(get_implementation_identifier,
                         "rmw_get_implementation_identifier");
  success &=
      load_symbol(get_serialization_format, "rmw_get_serialization_format");

  // Init
  success &= load_symbol(init_options_init, "rmw_init_options_init");
  success &= load_symbol(init_options_copy, "rmw_init_options_copy");
  success &= load_symbol(init_options_fini, "rmw_init_options_fini");
  success &= load_symbol(init, "rmw_init");
  success &= load_symbol(shutdown, "rmw_shutdown");
  success &= load_symbol(context_fini, "rmw_context_fini");

  // Node
  success &= load_symbol(create_node, "rmw_create_node");
  success &= load_symbol(destroy_node, "rmw_destroy_node");
  success &= load_symbol(node_get_graph_guard_condition,
                         "rmw_node_get_graph_guard_condition");

  // Publisher
  success &= load_symbol(create_publisher, "rmw_create_publisher");
  success &= load_symbol(destroy_publisher, "rmw_destroy_publisher");
  success &= load_symbol(publish, "rmw_publish");
  success &=
      load_symbol(publish_serialized_message, "rmw_publish_serialized_message");
  success &=
      load_symbol(publisher_get_actual_qos, "rmw_publisher_get_actual_qos");
  success &= load_symbol(publisher_count_matched_subscriptions,
                         "rmw_publisher_count_matched_subscriptions");
  success &= load_symbol(publisher_assert_liveliness,
                         "rmw_publisher_assert_liveliness");
  success &= load_symbol(publisher_wait_for_all_acked,
                         "rmw_publisher_wait_for_all_acked");

  // Subscription
  success &= load_symbol(create_subscription, "rmw_create_subscription");
  success &= load_symbol(destroy_subscription, "rmw_destroy_subscription");
  success &= load_symbol(take, "rmw_take");
  success &= load_symbol(take_with_info, "rmw_take_with_info");
  success &=
      load_symbol(take_serialized_message, "rmw_take_serialized_message");
  success &= load_symbol(take_serialized_message_with_info,
                         "rmw_take_serialized_message_with_info");
  success &= load_symbol(subscription_get_actual_qos,
                         "rmw_subscription_get_actual_qos");
  success &= load_symbol(subscription_count_matched_publishers,
                         "rmw_subscription_count_matched_publishers");

  // Service
  success &= load_symbol(create_service, "rmw_create_service");
  success &= load_symbol(destroy_service, "rmw_destroy_service");
  success &= load_symbol(take_request, "rmw_take_request");
  success &= load_symbol(send_response, "rmw_send_response");

  // Client
  success &= load_symbol(create_client, "rmw_create_client");
  success &= load_symbol(destroy_client, "rmw_destroy_client");
  success &= load_symbol(send_request, "rmw_send_request");
  success &= load_symbol(take_response, "rmw_take_response");

  // Guard Condition
  success &= load_symbol(create_guard_condition, "rmw_create_guard_condition");
  success &=
      load_symbol(destroy_guard_condition, "rmw_destroy_guard_condition");
  success &=
      load_symbol(trigger_guard_condition, "rmw_trigger_guard_condition");

  // Wait Set
  success &= load_symbol(create_wait_set, "rmw_create_wait_set");
  success &= load_symbol(destroy_wait_set, "rmw_destroy_wait_set");
  success &= load_symbol(wait, "rmw_wait");

  // Graph
  success &= load_symbol(get_node_names, "rmw_get_node_names");
  success &= load_symbol(get_node_names_with_enclaves,
                         "rmw_get_node_names_with_enclaves");
  success &= load_symbol(count_publishers, "rmw_count_publishers");
  success &= load_symbol(count_subscribers, "rmw_count_subscribers");
  success &= load_symbol(get_gid_for_publisher, "rmw_get_gid_for_publisher");
  success &= load_symbol(compare_gids_equal, "rmw_compare_gids_equal");
  success &= load_symbol(get_gid_for_client, "rmw_get_gid_for_client");

  // Serialization
  success &= load_symbol(serialize, "rmw_serialize");
  success &= load_symbol(deserialize, "rmw_deserialize");
  success &= load_symbol(get_serialized_message_size,
                         "rmw_get_serialized_message_size");

  // Topic and service names/types
  success &=
      load_symbol(get_topic_names_and_types, "rmw_get_topic_names_and_types");
  success &= load_symbol(get_service_names_and_types,
                         "rmw_get_service_names_and_types");
  success &= load_symbol(get_publisher_names_and_types_by_node,
                         "rmw_get_publisher_names_and_types_by_node");
  success &= load_symbol(get_subscriber_names_and_types_by_node,
                         "rmw_get_subscriber_names_and_types_by_node");
  success &= load_symbol(get_service_names_and_types_by_node,
                         "rmw_get_service_names_and_types_by_node");
  success &= load_symbol(get_client_names_and_types_by_node,
                         "rmw_get_client_names_and_types_by_node");

  // QoS queries for service/client
  success &= load_symbol(service_request_subscription_get_actual_qos,
                         "rmw_service_request_subscription_get_actual_qos");
  success &= load_symbol(service_response_publisher_get_actual_qos,
                         "rmw_service_response_publisher_get_actual_qos");
  success &= load_symbol(client_request_publisher_get_actual_qos,
                         "rmw_client_request_publisher_get_actual_qos");
  success &= load_symbol(client_response_subscription_get_actual_qos,
                         "rmw_client_response_subscription_get_actual_qos");

  // Event handling
  success &= load_symbol(publisher_event_init, "rmw_publisher_event_init");
  success &=
      load_symbol(subscription_event_init, "rmw_subscription_event_init");
  success &= load_symbol(take_event, "rmw_take_event");
  success &= load_symbol(event_fini, "rmw_event_fini");

  if (!success) {
    if (verbose) {
      RCUTILS_LOG_ERROR_NAMED("rmw_introspect",
                              "Failed to load all symbols from %s",
                              lib_name.c_str());
    }
    unload();
    return false;
  }

  if (verbose) {
    RCUTILS_LOG_INFO_NAMED("rmw_introspect", "Successfully loaded %s",
                           lib_name.c_str());
  }

  return true;
}

void RealRMW::unload() {
  if (lib_handle_) {
    dlclose(lib_handle_);
    lib_handle_ = nullptr;
  }
  name_.clear();

  // Reset all function pointers
  get_implementation_identifier = nullptr;
  get_serialization_format = nullptr;
  init_options_init = nullptr;
  init_options_copy = nullptr;
  init_options_fini = nullptr;
  init = nullptr;
  shutdown = nullptr;
  context_fini = nullptr;
  create_node = nullptr;
  destroy_node = nullptr;
  node_get_graph_guard_condition = nullptr;
  create_publisher = nullptr;
  destroy_publisher = nullptr;
  publish = nullptr;
  publish_serialized_message = nullptr;
  publisher_get_actual_qos = nullptr;
  publisher_count_matched_subscriptions = nullptr;
  publisher_assert_liveliness = nullptr;
  publisher_wait_for_all_acked = nullptr;
  create_subscription = nullptr;
  destroy_subscription = nullptr;
  take = nullptr;
  take_with_info = nullptr;
  take_serialized_message = nullptr;
  take_serialized_message_with_info = nullptr;
  subscription_get_actual_qos = nullptr;
  subscription_count_matched_publishers = nullptr;
  create_service = nullptr;
  destroy_service = nullptr;
  take_request = nullptr;
  send_response = nullptr;
  create_client = nullptr;
  destroy_client = nullptr;
  send_request = nullptr;
  take_response = nullptr;
  create_guard_condition = nullptr;
  destroy_guard_condition = nullptr;
  trigger_guard_condition = nullptr;
  create_wait_set = nullptr;
  destroy_wait_set = nullptr;
  wait = nullptr;
  get_node_names = nullptr;
  get_node_names_with_enclaves = nullptr;
  count_publishers = nullptr;
  count_subscribers = nullptr;
  get_gid_for_publisher = nullptr;
  compare_gids_equal = nullptr;
  get_gid_for_client = nullptr;
  serialize = nullptr;
  deserialize = nullptr;
  get_serialized_message_size = nullptr;
  get_topic_names_and_types = nullptr;
  get_service_names_and_types = nullptr;
  get_publisher_names_and_types_by_node = nullptr;
  get_subscriber_names_and_types_by_node = nullptr;
  get_service_names_and_types_by_node = nullptr;
  get_client_names_and_types_by_node = nullptr;
  service_request_subscription_get_actual_qos = nullptr;
  service_response_publisher_get_actual_qos = nullptr;
  client_request_publisher_get_actual_qos = nullptr;
  client_response_subscription_get_actual_qos = nullptr;
  publisher_event_init = nullptr;
  subscription_event_init = nullptr;
  take_event = nullptr;
  event_fini = nullptr;
}

template <typename FuncPtr>
bool RealRMW::load_symbol(FuncPtr &func_ptr, const char *symbol_name,
                          bool required) {
  // Clear any existing error
  dlerror();

  func_ptr = reinterpret_cast<FuncPtr>(dlsym(lib_handle_, symbol_name));

  if (!func_ptr) {
    if (required) {
      char error_msg[512];
      const char *dl_error = dlerror();
      snprintf(error_msg, sizeof(error_msg), "Failed to load symbol %s: %s",
               symbol_name, dl_error ? dl_error : "unknown error");
      RMW_SET_ERROR_MSG(error_msg);
      return false;
    }
  }

  return true;
}

} // namespace rmw_introspect
