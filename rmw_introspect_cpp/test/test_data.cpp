#include <gtest/gtest.h>
#include <fstream>
#include "rmw_introspect/data.hpp"
#include "rmw/types.h"

using rmw_introspect::IntrospectionData;
using rmw_introspect::PublisherInfo;
using rmw_introspect::QoSProfile;

// Test singleton instance
TEST(TestData, SingletonInstance) {
  auto & instance1 = IntrospectionData::instance();
  auto & instance2 = IntrospectionData::instance();

  EXPECT_EQ(&instance1, &instance2);
}

// Test recording node
TEST(TestData, RecordNode) {
  auto & data = IntrospectionData::instance();
  data.clear();

  data.record_node("test_node", "/test_namespace");

  const auto & nodes = data.get_nodes();
  ASSERT_EQ(nodes.size(), 1u);
  EXPECT_EQ(nodes[0], "/test_namespace/test_node");

  data.clear();
}

// Test recording publisher
TEST(TestData, RecordPublisher) {
  auto & data = IntrospectionData::instance();
  data.clear();

  PublisherInfo pub_info;
  pub_info.node_name = "test_node";
  pub_info.node_namespace = "/";
  pub_info.topic_name = "test_topic";
  pub_info.message_type = "std_msgs/msg/String";
  pub_info.qos.reliability = "reliable";
  pub_info.qos.durability = "volatile";
  pub_info.qos.history = "keep_last";
  pub_info.qos.depth = 10;
  pub_info.timestamp = 0.0;

  data.record_publisher(pub_info);

  const auto & publishers = data.get_publishers();
  ASSERT_EQ(publishers.size(), 1u);
  EXPECT_EQ(publishers[0].node_name, "test_node");
  EXPECT_EQ(publishers[0].topic_name, "test_topic");
  EXPECT_EQ(publishers[0].message_type, "std_msgs/msg/String");

  data.clear();
}

// Test QoS conversion
TEST(TestData, QoSConversion) {
  rmw_qos_profile_t rmw_qos;
  rmw_qos.reliability = RMW_QOS_POLICY_RELIABILITY_RELIABLE;
  rmw_qos.durability = RMW_QOS_POLICY_DURABILITY_VOLATILE;
  rmw_qos.history = RMW_QOS_POLICY_HISTORY_KEEP_LAST;
  rmw_qos.depth = 10;

  QoSProfile qos = QoSProfile::from_rmw(rmw_qos);

  EXPECT_EQ(qos.reliability, "reliable");
  EXPECT_EQ(qos.durability, "volatile");
  EXPECT_EQ(qos.history, "keep_last");
  EXPECT_EQ(qos.depth, 10u);
}

// Test JSON export
TEST(TestData, JsonExport) {
  auto & data = IntrospectionData::instance();
  data.clear();

  // Record some data
  data.record_node("test_node", "/");

  PublisherInfo pub_info;
  pub_info.node_name = "test_node";
  pub_info.node_namespace = "/";
  pub_info.topic_name = "test_topic";
  pub_info.message_type = "std_msgs/msg/String";
  pub_info.qos.reliability = "reliable";
  pub_info.qos.durability = "volatile";
  pub_info.qos.history = "keep_last";
  pub_info.qos.depth = 10;
  pub_info.timestamp = 0.0;

  data.record_publisher(pub_info);

  // Export to JSON
  std::string path = "/tmp/test_rmw_introspect.json";
  data.export_to_json(path);

  // Verify file exists and contains expected data
  std::ifstream file(path);
  ASSERT_TRUE(file.is_open());

  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());

  EXPECT_TRUE(content.find("\"format_version\": \"1.0\"") != std::string::npos);
  EXPECT_TRUE(content.find("\"rmw_implementation\": \"rmw_introspect_cpp\"") != std::string::npos);
  EXPECT_TRUE(content.find("\"test_node\"") != std::string::npos);
  EXPECT_TRUE(content.find("\"test_topic\"") != std::string::npos);
  EXPECT_TRUE(content.find("\"std_msgs/msg/String\"") != std::string::npos);

  // Clean up
  std::remove(path.c_str());
  data.clear();
}

int main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
