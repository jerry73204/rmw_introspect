#include <gtest/gtest.h>

#include "rcutils/allocator.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw_introspect/identifier.hpp"
#include "std_srvs/srv/empty.h"
#include "std_srvs/srv/detail/empty__type_support.h"

#include <cstdlib>
#include <cstring>

class ServiceClientIntermediateTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Set intermediate mode
    setenv("RMW_INTROSPECT_DELEGATE_TO", "rmw_fastrtps_cpp", 1);

    // Initialize allocator
    allocator_ = rcutils_get_default_allocator();

    // Initialize options
    rmw_ret_t ret = rmw_init_options_init(&init_options_, allocator_);
    ASSERT_EQ(RMW_RET_OK, ret);

    // Initialize context
    context_ = rmw_get_zero_initialized_context();
    ret = rmw_init(&init_options_, &context_);
    ASSERT_EQ(RMW_RET_OK, ret);

    // Create node
    node_ = rmw_create_node(&context_, "test_node", "/test");
    ASSERT_NE(nullptr, node_);
    ASSERT_STREQ(rmw_introspect_cpp_identifier,
                 node_->implementation_identifier);
  }

  void TearDown() override {
    if (node_) {
      rmw_ret_t ret = rmw_destroy_node(node_);
      EXPECT_EQ(RMW_RET_OK, ret);
    }

    if (rmw_context_is_valid(&context_)) {
      rmw_ret_t ret = rmw_shutdown(&context_);
      EXPECT_EQ(RMW_RET_OK, ret);

      ret = rmw_context_fini(&context_);
      EXPECT_EQ(RMW_RET_OK, ret);
    }

    rmw_ret_t ret = rmw_init_options_fini(&init_options_);
    EXPECT_EQ(RMW_RET_OK, ret);

    unsetenv("RMW_INTROSPECT_DELEGATE_TO");
  }

  rcutils_allocator_t allocator_;
  rmw_init_options_t init_options_;
  rmw_context_t context_;
  rmw_node_t *node_{nullptr};
};

TEST_F(ServiceClientIntermediateTest, CreateDestroyService) {
  // Get type support
  const rosidl_service_type_support_t *type_support =
      ROSIDL_GET_SRV_TYPE_SUPPORT(std_srvs, srv, Empty);
  ASSERT_NE(nullptr, type_support);

  // Create service
  rmw_qos_profile_t qos_profile = rmw_qos_profile_services_default;

  rmw_service_t *service = rmw_create_service(
      node_, type_support, "/test_service", &qos_profile);
  ASSERT_NE(nullptr, service);
  ASSERT_STREQ(rmw_introspect_cpp_identifier,
               service->implementation_identifier);
  ASSERT_NE(nullptr, service->data); // Should have wrapper

  // Destroy service
  rmw_ret_t ret = rmw_destroy_service(node_, service);
  EXPECT_EQ(RMW_RET_OK, ret);
}

TEST_F(ServiceClientIntermediateTest, CreateDestroyClient) {
  // Get type support
  const rosidl_service_type_support_t *type_support =
      ROSIDL_GET_SRV_TYPE_SUPPORT(std_srvs, srv, Empty);
  ASSERT_NE(nullptr, type_support);

  // Create client
  rmw_qos_profile_t qos_profile = rmw_qos_profile_services_default;

  rmw_client_t *client = rmw_create_client(
      node_, type_support, "/test_service", &qos_profile);
  ASSERT_NE(nullptr, client);
  ASSERT_STREQ(rmw_introspect_cpp_identifier,
               client->implementation_identifier);
  ASSERT_NE(nullptr, client->data); // Should have wrapper

  // Destroy client
  rmw_ret_t ret = rmw_destroy_client(node_, client);
  EXPECT_EQ(RMW_RET_OK, ret);
}

TEST_F(ServiceClientIntermediateTest, ServiceRequestResponse) {
  // Get type support
  const rosidl_service_type_support_t *type_support =
      ROSIDL_GET_SRV_TYPE_SUPPORT(std_srvs, srv, Empty);
  ASSERT_NE(nullptr, type_support);

  // Create service
  rmw_qos_profile_t qos_profile = rmw_qos_profile_services_default;
  rmw_service_t *service = rmw_create_service(
      node_, type_support, "/test_service", &qos_profile);
  ASSERT_NE(nullptr, service);

  // Create client
  rmw_client_t *client = rmw_create_client(
      node_, type_support, "/test_service", &qos_profile);
  ASSERT_NE(nullptr, client);

  // Send request
  std_srvs__srv__Empty_Request request;
  std_srvs__srv__Empty_Request__init(&request);
  int64_t sequence_id = 0;
  rmw_ret_t ret = rmw_send_request(client, &request, &sequence_id);
  EXPECT_EQ(RMW_RET_OK, ret);
  EXPECT_GT(sequence_id, 0);

  // Try to take request (may or may not succeed depending on timing)
  std_srvs__srv__Empty_Request received_request;
  std_srvs__srv__Empty_Request__init(&received_request);
  rmw_service_info_t request_header;
  bool taken = false;
  ret = rmw_take_request(service, &request_header, &received_request, &taken);
  EXPECT_EQ(RMW_RET_OK, ret);

  // If request was taken, send response
  if (taken) {
    std_srvs__srv__Empty_Response response;
    std_srvs__srv__Empty_Response__init(&response);
    rmw_request_id_t response_id;
    response_id.sequence_number = request_header.request_id.sequence_number;
    memcpy(response_id.writer_guid, request_header.request_id.writer_guid,
           sizeof(response_id.writer_guid));
    ret = rmw_send_response(service, &response_id, &response);
    EXPECT_EQ(RMW_RET_OK, ret);

    // Try to take response
    std_srvs__srv__Empty_Response received_response;
    std_srvs__srv__Empty_Response__init(&received_response);
    rmw_service_info_t response_header;
    bool response_taken = false;
    ret = rmw_take_response(client, &response_header, &received_response,
                           &response_taken);
    EXPECT_EQ(RMW_RET_OK, ret);

    std_srvs__srv__Empty_Response__fini(&received_response);
    std_srvs__srv__Empty_Response__fini(&response);
  }

  // Cleanup
  std_srvs__srv__Empty_Request__fini(&request);
  std_srvs__srv__Empty_Request__fini(&received_request);

  ret = rmw_destroy_client(node_, client);
  EXPECT_EQ(RMW_RET_OK, ret);

  ret = rmw_destroy_service(node_, service);
  EXPECT_EQ(RMW_RET_OK, ret);
}

TEST_F(ServiceClientIntermediateTest, ServiceGetActualQoS) {
  // Get type support
  const rosidl_service_type_support_t *type_support =
      ROSIDL_GET_SRV_TYPE_SUPPORT(std_srvs, srv, Empty);
  ASSERT_NE(nullptr, type_support);

  // Create service
  rmw_qos_profile_t qos_profile = rmw_qos_profile_services_default;
  rmw_service_t *service = rmw_create_service(
      node_, type_support, "/test_service", &qos_profile);
  ASSERT_NE(nullptr, service);

  // Get actual QoS for request subscription
  rmw_qos_profile_t actual_qos;
  rmw_ret_t ret = rmw_service_request_subscription_get_actual_qos(service, &actual_qos);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Get actual QoS for response publisher
  ret = rmw_service_response_publisher_get_actual_qos(service, &actual_qos);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Cleanup
  ret = rmw_destroy_service(node_, service);
  EXPECT_EQ(RMW_RET_OK, ret);
}

TEST_F(ServiceClientIntermediateTest, ClientGetActualQoS) {
  // Get type support
  const rosidl_service_type_support_t *type_support =
      ROSIDL_GET_SRV_TYPE_SUPPORT(std_srvs, srv, Empty);
  ASSERT_NE(nullptr, type_support);

  // Create client
  rmw_qos_profile_t qos_profile = rmw_qos_profile_services_default;
  rmw_client_t *client = rmw_create_client(
      node_, type_support, "/test_service", &qos_profile);
  ASSERT_NE(nullptr, client);

  // Get actual QoS for request publisher
  rmw_qos_profile_t actual_qos;
  rmw_ret_t ret = rmw_client_request_publisher_get_actual_qos(client, &actual_qos);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Get actual QoS for response subscription
  ret = rmw_client_response_subscription_get_actual_qos(client, &actual_qos);
  EXPECT_EQ(RMW_RET_OK, ret);

  // Cleanup
  ret = rmw_destroy_client(node_, client);
  EXPECT_EQ(RMW_RET_OK, ret);
}

TEST_F(ServiceClientIntermediateTest, ServiceServerIsAvailable) {
  // Get type support
  const rosidl_service_type_support_t *type_support =
      ROSIDL_GET_SRV_TYPE_SUPPORT(std_srvs, srv, Empty);
  ASSERT_NE(nullptr, type_support);

  // Create service
  rmw_qos_profile_t qos_profile = rmw_qos_profile_services_default;
  rmw_service_t *service = rmw_create_service(
      node_, type_support, "/test_service", &qos_profile);
  ASSERT_NE(nullptr, service);

  // Create client
  rmw_client_t *client = rmw_create_client(
      node_, type_support, "/test_service", &qos_profile);
  ASSERT_NE(nullptr, client);

  // Check if service is available (should forward to real RMW)
  bool is_available = false;
  rmw_ret_t ret = rmw_service_server_is_available(node_, client, &is_available);
  EXPECT_EQ(RMW_RET_OK, ret);
  // Note: is_available may be true or false depending on timing

  // Cleanup
  ret = rmw_destroy_client(node_, client);
  EXPECT_EQ(RMW_RET_OK, ret);

  ret = rmw_destroy_service(node_, service);
  EXPECT_EQ(RMW_RET_OK, ret);
}

TEST_F(ServiceClientIntermediateTest, MultipleServiceClients) {
  // Get type support
  const rosidl_service_type_support_t *type_support =
      ROSIDL_GET_SRV_TYPE_SUPPORT(std_srvs, srv, Empty);
  ASSERT_NE(nullptr, type_support);

  // Create multiple services
  rmw_qos_profile_t qos_profile = rmw_qos_profile_services_default;
  rmw_service_t *service1 = rmw_create_service(
      node_, type_support, "/test_service1", &qos_profile);
  ASSERT_NE(nullptr, service1);

  rmw_service_t *service2 = rmw_create_service(
      node_, type_support, "/test_service2", &qos_profile);
  ASSERT_NE(nullptr, service2);

  // Create multiple clients
  rmw_client_t *client1 = rmw_create_client(
      node_, type_support, "/test_service1", &qos_profile);
  ASSERT_NE(nullptr, client1);

  rmw_client_t *client2 = rmw_create_client(
      node_, type_support, "/test_service2", &qos_profile);
  ASSERT_NE(nullptr, client2);

  // Cleanup
  rmw_ret_t ret = rmw_destroy_client(node_, client1);
  EXPECT_EQ(RMW_RET_OK, ret);

  ret = rmw_destroy_client(node_, client2);
  EXPECT_EQ(RMW_RET_OK, ret);

  ret = rmw_destroy_service(node_, service1);
  EXPECT_EQ(RMW_RET_OK, ret);

  ret = rmw_destroy_service(node_, service2);
  EXPECT_EQ(RMW_RET_OK, ret);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
