#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>

#include "rcutils/allocator.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw_introspect/identifier.hpp"
#include "std_msgs/msg/string.h"
#include "std_srvs/srv/empty.h"
#include "test_msgs/msg/basic_types.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_cpp/service_type_support.hpp"

using namespace std::chrono;

// Stress test configuration
constexpr size_t NUM_NODES = 10;
constexpr size_t NUM_PUBLISHERS_PER_NODE = 5;
constexpr size_t NUM_SUBSCRIPTIONS_PER_NODE = 5;
constexpr size_t NUM_SERVICES_PER_NODE = 2;
constexpr size_t NUM_CLIENTS_PER_NODE = 2;
constexpr size_t NUM_CREATE_DESTROY_CYCLES = 100;
constexpr size_t NUM_PUBLISH_ITERATIONS = 1000;

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  std::cout << "RMW Introspect Stress Test\n";
  std::cout << "===========================\n\n";

  const char *delegate_to = std::getenv("RMW_INTROSPECT_DELEGATE_TO");
  if (delegate_to) {
    std::cout << "Intermediate mode: delegating to " << delegate_to << "\n";
  } else {
    std::cout << "Recording-only mode (no delegation)\n";
  }

  std::cout << "\nTest configuration:\n";
  std::cout << "  Nodes: " << NUM_NODES << "\n";
  std::cout << "  Publishers per node: " << NUM_PUBLISHERS_PER_NODE << "\n";
  std::cout << "  Subscriptions per node: " << NUM_SUBSCRIPTIONS_PER_NODE << "\n";
  std::cout << "  Services per node: " << NUM_SERVICES_PER_NODE << "\n";
  std::cout << "  Clients per node: " << NUM_CLIENTS_PER_NODE << "\n";
  std::cout << "  Create/destroy cycles: " << NUM_CREATE_DESTROY_CYCLES << "\n";
  std::cout << "  Publish iterations: " << NUM_PUBLISH_ITERATIONS << "\n\n";

  // Initialize context
  rmw_init_options_t init_options = rmw_get_zero_initialized_init_options();
  rmw_ret_t ret =
      rmw_init_options_init(&init_options, rcutils_get_default_allocator());
  if (ret != RMW_RET_OK) {
    std::cerr << "Failed to initialize init options\n";
    return 1;
  }

  rmw_context_t context = rmw_get_zero_initialized_context();
  ret = rmw_init(&init_options, &context);
  if (ret != RMW_RET_OK) {
    std::cerr << "Failed to initialize context\n";
    rmw_init_options_fini(&init_options);
    return 1;
  }

  rmw_init_options_fini(&init_options);

  // Get type supports
  const rosidl_message_type_support_t *msg_type_support =
      ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, BasicTypes);
  const rosidl_service_type_support_t *srv_type_support =
      ROSIDL_GET_SRV_TYPE_SUPPORT(std_srvs, srv, Empty);

  // Test 1: Mass entity creation
  std::cout << "Test 1: Creating " << NUM_NODES << " nodes with publishers, "
            << "subscriptions, services, and clients...\n";

  auto start_create = high_resolution_clock::now();

  std::vector<rmw_node_t *> nodes;
  std::vector<std::vector<rmw_publisher_t *>> publishers;
  std::vector<std::vector<rmw_subscription_t *>> subscriptions;
  std::vector<std::vector<rmw_service_t *>> services;
  std::vector<std::vector<rmw_client_t *>> clients;

  for (size_t i = 0; i < NUM_NODES; ++i) {
    std::string node_name = "stress_node_" + std::to_string(i);
    rmw_node_t *node =
        rmw_create_node(&context, node_name.c_str(), "/stress_test");
    if (!node) {
      std::cerr << "Failed to create node " << i << "\n";
      goto cleanup;
    }
    nodes.push_back(node);

    std::vector<rmw_publisher_t *> node_publishers;
    for (size_t j = 0; j < NUM_PUBLISHERS_PER_NODE; ++j) {
      std::string topic_name = "stress_topic_" + std::to_string(i) + "_" +
                               std::to_string(j);
      rmw_publisher_options_t pub_options = rmw_get_default_publisher_options();
      rmw_publisher_t *pub =
          rmw_create_publisher(node, msg_type_support, topic_name.c_str(),
                              &rmw_qos_profile_default, &pub_options);
      if (!pub) {
        std::cerr << "Failed to create publisher " << j << " on node " << i
                  << "\n";
        goto cleanup;
      }
      node_publishers.push_back(pub);
    }
    publishers.push_back(node_publishers);

    std::vector<rmw_subscription_t *> node_subscriptions;
    for (size_t j = 0; j < NUM_SUBSCRIPTIONS_PER_NODE; ++j) {
      std::string topic_name = "stress_topic_" + std::to_string(i) + "_" +
                               std::to_string(j);
      rmw_subscription_options_t sub_options =
          rmw_get_default_subscription_options();
      rmw_subscription_t *sub = rmw_create_subscription(
          node, msg_type_support, topic_name.c_str(), &rmw_qos_profile_default,
          &sub_options);
      if (!sub) {
        std::cerr << "Failed to create subscription " << j << " on node " << i
                  << "\n";
        goto cleanup;
      }
      node_subscriptions.push_back(sub);
    }
    subscriptions.push_back(node_subscriptions);

    std::vector<rmw_service_t *> node_services;
    for (size_t j = 0; j < NUM_SERVICES_PER_NODE; ++j) {
      std::string service_name =
          "stress_service_" + std::to_string(i) + "_" + std::to_string(j);
      rmw_service_t *srv = rmw_create_service(
          node, srv_type_support, service_name.c_str(), &rmw_qos_profile_default);
      if (!srv) {
        std::cerr << "Failed to create service " << j << " on node " << i
                  << "\n";
        goto cleanup;
      }
      node_services.push_back(srv);
    }
    services.push_back(node_services);

    std::vector<rmw_client_t *> node_clients;
    for (size_t j = 0; j < NUM_CLIENTS_PER_NODE; ++j) {
      std::string service_name =
          "stress_service_" + std::to_string(i) + "_" + std::to_string(j);
      rmw_client_t *client = rmw_create_client(
          node, srv_type_support, service_name.c_str(), &rmw_qos_profile_default);
      if (!client) {
        std::cerr << "Failed to create client " << j << " on node " << i
                  << "\n";
        goto cleanup;
      }
      node_clients.push_back(client);
    }
    clients.push_back(node_clients);
  }

  {
    auto end_create = high_resolution_clock::now();
    auto create_duration = duration_cast<milliseconds>(end_create - start_create);

    std::cout << "✓ Created all entities in " << create_duration.count()
              << " ms\n\n";
  }

  // Test 2: High-frequency publishing
  std::cout << "Test 2: High-frequency message publishing (" << NUM_PUBLISH_ITERATIONS
            << " iterations)...\n";

  {
    test_msgs__msg__BasicTypes msg;
    test_msgs__msg__BasicTypes__init(&msg);
    msg.int32_value = 42;

    auto start_publish = high_resolution_clock::now();

    for (size_t iter = 0; iter < NUM_PUBLISH_ITERATIONS; ++iter) {
      for (size_t i = 0; i < nodes.size(); ++i) {
        for (size_t j = 0; j < publishers[i].size(); ++j) {
          ret = rmw_publish(publishers[i][j], &msg, nullptr);
          if (ret != RMW_RET_OK) {
            std::cerr << "Publish failed\n";
            test_msgs__msg__BasicTypes__fini(&msg);
            goto cleanup;
          }
        }
      }
    }

    auto end_publish = high_resolution_clock::now();
    auto publish_duration =
        duration_cast<milliseconds>(end_publish - start_publish);

    test_msgs__msg__BasicTypes__fini(&msg);

    size_t total_publishes =
        NUM_PUBLISH_ITERATIONS * nodes.size() * NUM_PUBLISHERS_PER_NODE;
    double publish_rate = (total_publishes * 1000.0) / publish_duration.count();

    std::cout << "✓ Published " << total_publishes << " messages in "
              << publish_duration.count() << " ms\n";
    std::cout << "  Rate: " << publish_rate << " msg/s\n\n";
  }

  // Test 3: Rapid create/destroy cycles
  std::cout << "Test 3: Rapid create/destroy cycles (" << NUM_CREATE_DESTROY_CYCLES
            << " cycles)...\n";

  {
    auto start_cycle = high_resolution_clock::now();

    for (size_t cycle = 0; cycle < NUM_CREATE_DESTROY_CYCLES; ++cycle) {
    // Create temporary publisher
    rmw_publisher_options_t pub_options = rmw_get_default_publisher_options();
    rmw_publisher_t *temp_pub = rmw_create_publisher(
        nodes[0], msg_type_support, "temp_topic", &rmw_qos_profile_default,
        &pub_options);
    if (!temp_pub) {
      std::cerr << "Failed to create temporary publisher in cycle " << cycle
                << "\n";
      goto cleanup;
    }

    // Immediately destroy it
    ret = rmw_destroy_publisher(nodes[0], temp_pub);
    if (ret != RMW_RET_OK) {
      std::cerr << "Failed to destroy temporary publisher in cycle " << cycle
                << "\n";
      goto cleanup;
    }

    // Create temporary subscription
    rmw_subscription_options_t sub_options =
        rmw_get_default_subscription_options();
    rmw_subscription_t *temp_sub = rmw_create_subscription(
        nodes[0], msg_type_support, "temp_topic", &rmw_qos_profile_default,
        &sub_options);
    if (!temp_sub) {
      std::cerr << "Failed to create temporary subscription in cycle " << cycle
                << "\n";
      goto cleanup;
    }

    // Immediately destroy it
    ret = rmw_destroy_subscription(nodes[0], temp_sub);
    if (ret != RMW_RET_OK) {
      std::cerr << "Failed to destroy temporary subscription in cycle " << cycle
                << "\n";
      goto cleanup;
    }
    }

    auto end_cycle = high_resolution_clock::now();
    auto cycle_duration = duration_cast<milliseconds>(end_cycle - start_cycle);

    std::cout << "✓ Completed " << NUM_CREATE_DESTROY_CYCLES << " cycles in "
              << cycle_duration.count() << " ms\n";
    std::cout << "  Average cycle time: "
              << (cycle_duration.count() / (double)NUM_CREATE_DESTROY_CYCLES)
              << " ms\n\n";
  }

cleanup:
  // Cleanup all entities
  std::cout << "Cleaning up...\n";

  auto start_cleanup = high_resolution_clock::now();

  for (size_t i = 0; i < nodes.size(); ++i) {
    for (auto *client : clients[i]) {
      rmw_destroy_client(nodes[i], client);
    }
    for (auto *service : services[i]) {
      rmw_destroy_service(nodes[i], service);
    }
    for (auto *sub : subscriptions[i]) {
      rmw_destroy_subscription(nodes[i], sub);
    }
    for (auto *pub : publishers[i]) {
      rmw_destroy_publisher(nodes[i], pub);
    }
    rmw_destroy_node(nodes[i]);
  }

  auto end_cleanup = high_resolution_clock::now();
  auto cleanup_duration = duration_cast<milliseconds>(end_cleanup - start_cleanup);

  std::cout << "✓ Cleanup completed in " << cleanup_duration.count() << " ms\n";

  rmw_shutdown(&context);
  rmw_context_fini(&context);

  std::cout << "\n✓ Stress test completed successfully!\n";

  return 0;
}
