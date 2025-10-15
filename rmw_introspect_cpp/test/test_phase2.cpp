#include <gtest/gtest.h>
#include "rmw/rmw.h"
#include "rmw/init.h"
#include "std_srvs/srv/set_bool.hpp"
#include "std_srvs/srv/detail/set_bool__rosidl_typesupport_introspection_cpp.hpp"
#include "rmw_introspect/data.hpp"
#include <chrono>

// Test service creation
TEST(TestPhase2, ServiceCreation) {
  rmw_init_options_t options = rmw_get_zero_initialized_init_options();
  rmw_context_t context = rmw_get_zero_initialized_context();
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  rmw_init_options_init(&options, allocator);
  rmw_init(&options, &context);

  rmw_node_t * node = rmw_create_node(&context, "srv_node", "/");
  ASSERT_NE(node, nullptr);

  // Get introspection type support for std_srvs/srv/SetBool
  const rosidl_service_type_support_t * type_support =
    ROSIDL_TYPESUPPORT_INTERFACE__SERVICE_SYMBOL_NAME(
      rosidl_typesupport_introspection_cpp, std_srvs, srv, SetBool)();

  rmw_qos_profile_t qos = rmw_qos_profile_services_default;

  // Create service
  rmw_service_t * service = rmw_create_service(
    node, type_support, "test_service", &qos);
  ASSERT_NE(service, nullptr);
  EXPECT_STREQ(service->service_name, "test_service");

  // Verify service recorded
  auto & data = rmw_introspect::IntrospectionData::instance();
  const auto & services = data.get_services();
  bool found = false;
  for (const auto & s : services) {
    if (s.service_name == "test_service" && s.service_type == "std_srvs/srv/SetBool") {
      found = true;
      EXPECT_EQ(s.node_name, "srv_node");
      break;
    }
  }
  EXPECT_TRUE(found);

  // Test take request (no-op)
  rmw_service_info_t header;
  std_srvs::srv::SetBool::Request request;
  bool taken = true;
  rmw_ret_t ret = rmw_take_request(service, &header, &request, &taken);
  EXPECT_EQ(ret, RMW_RET_OK);
  EXPECT_FALSE(taken);

  // Clean up
  rmw_destroy_service(node, service);
  rmw_destroy_node(node);
  rmw_shutdown(&context);
  rmw_context_fini(&context);
  rmw_init_options_fini(&options);
}

// Test client creation
TEST(TestPhase2, ClientCreation) {
  rmw_init_options_t options = rmw_get_zero_initialized_init_options();
  rmw_context_t context = rmw_get_zero_initialized_context();
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  rmw_init_options_init(&options, allocator);
  rmw_init(&options, &context);

  rmw_node_t * node = rmw_create_node(&context, "cli_node", "/");
  ASSERT_NE(node, nullptr);

  const rosidl_service_type_support_t * type_support =
    ROSIDL_TYPESUPPORT_INTERFACE__SERVICE_SYMBOL_NAME(
      rosidl_typesupport_introspection_cpp, std_srvs, srv, SetBool)();

  rmw_qos_profile_t qos = rmw_qos_profile_services_default;

  // Create client
  rmw_client_t * client = rmw_create_client(
    node, type_support, "test_client_service", &qos);
  ASSERT_NE(client, nullptr);
  EXPECT_STREQ(client->service_name, "test_client_service");

  // Verify client recorded
  auto & data = rmw_introspect::IntrospectionData::instance();
  const auto & clients = data.get_clients();
  bool found = false;
  for (const auto & c : clients) {
    if (c.service_name == "test_client_service" && c.service_type == "std_srvs/srv/SetBool") {
      found = true;
      EXPECT_EQ(c.node_name, "cli_node");
      break;
    }
  }
  EXPECT_TRUE(found);

  // Test send request (stub)
  std_srvs::srv::SetBool::Request request;
  int64_t sequence_id = 0;
  rmw_ret_t ret = rmw_send_request(client, &request, &sequence_id);
  EXPECT_EQ(ret, RMW_RET_OK);
  EXPECT_EQ(sequence_id, 1);  // Should return dummy sequence number

  // Clean up
  rmw_destroy_client(node, client);
  rmw_destroy_node(node);
  rmw_shutdown(&context);
  rmw_context_fini(&context);
  rmw_init_options_fini(&options);
}

// Test wait set creation and rmw_wait timeout
TEST(TestPhase2, WaitOperations) {
  rmw_init_options_t options = rmw_get_zero_initialized_init_options();
  rmw_context_t context = rmw_get_zero_initialized_context();
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  rmw_init_options_init(&options, allocator);
  rmw_init(&options, &context);

  // Create wait set
  rmw_wait_set_t * wait_set = rmw_create_wait_set(&context, 10);
  ASSERT_NE(wait_set, nullptr);

  // Test rmw_wait - should return timeout immediately
  auto start = std::chrono::steady_clock::now();

  rmw_time_t timeout{1, 0};  // 1 second timeout
  rmw_ret_t ret = rmw_wait(nullptr, nullptr, nullptr, nullptr, nullptr, wait_set, &timeout);

  auto end = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

  EXPECT_EQ(ret, RMW_RET_TIMEOUT);
  EXPECT_LT(duration, 100);  // Should return in <100ms, not wait full timeout

  // Clean up
  rmw_destroy_wait_set(wait_set);
  rmw_shutdown(&context);
  rmw_context_fini(&context);
  rmw_init_options_fini(&options);
}

// Test guard condition
TEST(TestPhase2, GuardCondition) {
  rmw_init_options_t options = rmw_get_zero_initialized_init_options();
  rmw_context_t context = rmw_get_zero_initialized_context();
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  rmw_init_options_init(&options, allocator);
  rmw_init(&options, &context);

  // Create guard condition
  rmw_guard_condition_t * guard_condition = rmw_create_guard_condition(&context);
  ASSERT_NE(guard_condition, nullptr);

  // Trigger guard condition (no-op)
  rmw_ret_t ret = rmw_trigger_guard_condition(guard_condition);
  EXPECT_EQ(ret, RMW_RET_OK);

  // Clean up
  rmw_destroy_guard_condition(guard_condition);
  rmw_shutdown(&context);
  rmw_context_fini(&context);
  rmw_init_options_fini(&options);
}

int main(int argc, char ** argv) {
  // Clear introspection data before tests
  rmw_introspect::IntrospectionData::instance().clear();

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
