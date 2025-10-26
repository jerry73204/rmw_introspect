#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

#include "rcutils/allocator.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw_introspect/identifier.hpp"
#include "test_msgs/msg/basic_types.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"

using namespace std::chrono;

// Benchmark configuration
constexpr size_t NUM_ITERATIONS = 1000;
constexpr size_t WARMUP_ITERATIONS = 100;

struct BenchmarkResults {
  double mean_latency_us;
  double min_latency_us;
  double max_latency_us;
  double stddev_latency_us;
};

BenchmarkResults calculate_statistics(const std::vector<double> &latencies) {
  BenchmarkResults results;

  // Calculate mean
  double sum = 0.0;
  for (double latency : latencies) {
    sum += latency;
  }
  results.mean_latency_us = sum / latencies.size();

  // Calculate min and max
  results.min_latency_us = latencies[0];
  results.max_latency_us = latencies[0];
  for (double latency : latencies) {
    if (latency < results.min_latency_us)
      results.min_latency_us = latency;
    if (latency > results.max_latency_us)
      results.max_latency_us = latency;
  }

  // Calculate standard deviation
  double variance = 0.0;
  for (double latency : latencies) {
    double diff = latency - results.mean_latency_us;
    variance += diff * diff;
  }
  results.stddev_latency_us = std::sqrt(variance / latencies.size());

  return results;
}

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  std::cout << "RMW Introspect Pub-Sub Latency Benchmark\n";
  std::cout << "=========================================\n\n";

  // Check if intermediate mode is enabled
  const char *delegate_to = std::getenv("RMW_INTROSPECT_DELEGATE_TO");
  if (delegate_to) {
    std::cout << "Intermediate mode: delegating to " << delegate_to << "\n";
  } else {
    std::cout << "Recording-only mode (no delegation)\n";
  }
  std::cout << "Iterations: " << NUM_ITERATIONS << " (after " << WARMUP_ITERATIONS
            << " warmup)\n\n";

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

  // Create node
  rmw_node_t *node = rmw_create_node(&context, "benchmark_node", "/benchmark");
  if (!node) {
    std::cerr << "Failed to create node\n";
    rmw_shutdown(&context);
    rmw_context_fini(&context);
    return 1;
  }

  // Get type support
  const rosidl_message_type_support_t *type_support =
      ROSIDL_GET_MSG_TYPE_SUPPORT(test_msgs, msg, BasicTypes);

  // Create publisher
  rmw_publisher_options_t pub_options = rmw_get_default_publisher_options();
  rmw_publisher_t *publisher = rmw_create_publisher(
      node, type_support, "benchmark_topic", &rmw_qos_profile_default, &pub_options);
  if (!publisher) {
    std::cerr << "Failed to create publisher\n";
    rmw_destroy_node(node);
    rmw_shutdown(&context);
    rmw_context_fini(&context);
    return 1;
  }

  // Create subscription
  rmw_subscription_options_t sub_options = rmw_get_default_subscription_options();
  rmw_subscription_t *subscription = rmw_create_subscription(
      node, type_support, "benchmark_topic", &rmw_qos_profile_default, &sub_options);
  if (!subscription) {
    std::cerr << "Failed to create subscription\n";
    rmw_destroy_publisher(node, publisher);
    rmw_destroy_node(node);
    rmw_shutdown(&context);
    rmw_context_fini(&context);
    return 1;
  }

  // Prepare test message
  test_msgs__msg__BasicTypes msg;
  test_msgs__msg__BasicTypes__init(&msg);
  msg.int32_value = 42;

  test_msgs__msg__BasicTypes received_msg;
  test_msgs__msg__BasicTypes__init(&received_msg);

  std::vector<double> latencies;
  latencies.reserve(NUM_ITERATIONS);

  std::cout << "Running warmup iterations...\n";
  for (size_t i = 0; i < WARMUP_ITERATIONS; ++i) {
    rmw_publish(publisher, &msg, nullptr);
  }

  std::cout << "Running benchmark...\n";
  for (size_t i = 0; i < NUM_ITERATIONS; ++i) {
    auto start = high_resolution_clock::now();

    // Publish message
    ret = rmw_publish(publisher, &msg, nullptr);
    if (ret != RMW_RET_OK) {
      std::cerr << "Publish failed at iteration " << i << "\n";
      break;
    }

    // Take message (in intermediate mode this would actually receive)
    // In recording-only mode, this will return immediately
    bool taken = false;
    rmw_message_info_t message_info = rmw_get_zero_initialized_message_info();
    ret = rmw_take_with_info(subscription, &received_msg, &taken, &message_info,
                             nullptr);

    auto end = high_resolution_clock::now();

    if (ret == RMW_RET_OK) {
      double latency_us =
          duration_cast<microseconds>(end - start).count();
      latencies.push_back(latency_us);
    }

    // Small delay between iterations
    std::this_thread::sleep_for(microseconds(100));
  }

  // Calculate and display results
  if (!latencies.empty()) {
    BenchmarkResults results = calculate_statistics(latencies);

    std::cout << "\nBenchmark Results:\n";
    std::cout << "-----------------\n";
    std::cout << "Mean latency:   " << results.mean_latency_us << " μs\n";
    std::cout << "Min latency:    " << results.min_latency_us << " μs\n";
    std::cout << "Max latency:    " << results.max_latency_us << " μs\n";
    std::cout << "Std deviation:  " << results.stddev_latency_us << " μs\n";
    std::cout << "Samples:        " << latencies.size() << "\n";
  } else {
    std::cout << "\nNo latency measurements collected\n";
  }

  // Cleanup
  test_msgs__msg__BasicTypes__fini(&msg);
  test_msgs__msg__BasicTypes__fini(&received_msg);
  rmw_destroy_subscription(node, subscription);
  rmw_destroy_publisher(node, publisher);
  rmw_destroy_node(node);
  rmw_shutdown(&context);
  rmw_context_fini(&context);

  return 0;
}
