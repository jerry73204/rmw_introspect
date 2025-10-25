#include <gtest/gtest.h>
#include "rmw_introspect/real_rmw.hpp"
#include <cstdlib>

using rmw_introspect::RealRMW;

class TestRealRMW : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Clear any existing environment variable
    unsetenv("RMW_INTROSPECT_DELEGATE_TO");
  }

  void TearDown() override
  {
    unsetenv("RMW_INTROSPECT_DELEGATE_TO");
  }
};

TEST_F(TestRealRMW, ConstructorInitializesNull)
{
  RealRMW rmw;
  EXPECT_FALSE(rmw.is_loaded());
  EXPECT_EQ(rmw.get_name(), "");
}

TEST_F(TestRealRMW, LoadInvalidName)
{
  RealRMW rmw;
  EXPECT_FALSE(rmw.load(nullptr));
  EXPECT_FALSE(rmw.load(""));
  EXPECT_FALSE(rmw.load("invalid"));  // Doesn't start with rmw_
  EXPECT_FALSE(rmw.is_loaded());
}

TEST_F(TestRealRMW, LoadNonexistentLibrary)
{
  RealRMW rmw;
  EXPECT_FALSE(rmw.load("rmw_nonexistent_implementation"));
  EXPECT_FALSE(rmw.is_loaded());
}

// This test requires rmw_fastrtps_cpp to be installed
TEST_F(TestRealRMW, LoadFastRTPS)
{
  RealRMW rmw;
  bool loaded = rmw.load("rmw_fastrtps_cpp");

  if (!loaded) {
    GTEST_SKIP() << "rmw_fastrtps_cpp not available, skipping test";
  }

  EXPECT_TRUE(rmw.is_loaded());
  EXPECT_EQ(rmw.get_name(), "rmw_fastrtps_cpp");

  // Verify critical function pointers are loaded
  EXPECT_NE(rmw.get_implementation_identifier, nullptr);
  EXPECT_NE(rmw.init, nullptr);
  EXPECT_NE(rmw.shutdown, nullptr);
  EXPECT_NE(rmw.create_node, nullptr);
  EXPECT_NE(rmw.create_publisher, nullptr);
  EXPECT_NE(rmw.create_subscription, nullptr);

  // Verify identifier
  EXPECT_NE(rmw.get_implementation_identifier(), nullptr);
  EXPECT_STREQ(rmw.get_implementation_identifier(), "rmw_fastrtps_cpp");
}

// This test requires rmw_cyclonedds_cpp to be installed
TEST_F(TestRealRMW, LoadCycloneDDS)
{
  RealRMW rmw;
  bool loaded = rmw.load("rmw_cyclonedds_cpp");

  if (!loaded) {
    GTEST_SKIP() << "rmw_cyclonedds_cpp not available, skipping test";
  }

  EXPECT_TRUE(rmw.is_loaded());
  EXPECT_EQ(rmw.get_name(), "rmw_cyclonedds_cpp");

  // Verify identifier
  EXPECT_NE(rmw.get_implementation_identifier(), nullptr);
  EXPECT_STREQ(rmw.get_implementation_identifier(), "rmw_cyclonedds_cpp");
}

TEST_F(TestRealRMW, UnloadAfterLoad)
{
  RealRMW rmw;

  if (!rmw.load("rmw_fastrtps_cpp")) {
    GTEST_SKIP() << "rmw_fastrtps_cpp not available, skipping test";
  }

  EXPECT_TRUE(rmw.is_loaded());

  rmw.unload();

  EXPECT_FALSE(rmw.is_loaded());
  EXPECT_EQ(rmw.get_name(), "");
  EXPECT_EQ(rmw.get_implementation_identifier, nullptr);
}

TEST_F(TestRealRMW, LoadTwiceWithoutUnload)
{
  RealRMW rmw;

  if (!rmw.load("rmw_fastrtps_cpp")) {
    GTEST_SKIP() << "rmw_fastrtps_cpp not available, skipping test";
  }

  EXPECT_TRUE(rmw.is_loaded());

  // Try to load again without unloading
  EXPECT_FALSE(rmw.load("rmw_fastrtps_cpp"));
}

TEST_F(TestRealRMW, ReloadAfterUnload)
{
  RealRMW rmw;

  if (!rmw.load("rmw_fastrtps_cpp")) {
    GTEST_SKIP() << "rmw_fastrtps_cpp not available, skipping test";
  }

  rmw.unload();

  // Should be able to load again after unload
  bool loaded = rmw.load("rmw_fastrtps_cpp");
  EXPECT_TRUE(loaded);
  EXPECT_TRUE(rmw.is_loaded());
}

TEST_F(TestRealRMW, DestructorUnloads)
{
  {
    RealRMW rmw;
    if (!rmw.load("rmw_fastrtps_cpp")) {
      GTEST_SKIP() << "rmw_fastrtps_cpp not available, skipping test";
    }
    EXPECT_TRUE(rmw.is_loaded());
    // Destructor should be called here
  }
  // If we get here without crashing, destructor worked
  SUCCEED();
}

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
