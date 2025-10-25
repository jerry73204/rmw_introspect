#include <gtest/gtest.h>
#include "rmw/rmw.h"
#include "rmw/error_handling.h"
#include "rmw_introspect/mode.hpp"
#include "rmw_introspect/wrappers.hpp"
#include <cstdlib>

class TestInitIntermediate : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // Clear environment
    unsetenv("RMW_INTROSPECT_DELEGATE_TO");
    unsetenv("RMW_INTROSPECT_VERBOSE");
  }

  void TearDown() override
  {
    unsetenv("RMW_INTROSPECT_DELEGATE_TO");
    unsetenv("RMW_INTROSPECT_VERBOSE");
  }
};

TEST_F(TestInitIntermediate, RecordingOnlyModeByDefault)
{
  // Create init options
  rmw_init_options_t options = rmw_get_zero_initialized_init_options();
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  ASSERT_EQ(rmw_init_options_init(&options, allocator), RMW_RET_OK);

  // Create context
  rmw_context_t context = rmw_get_zero_initialized_context();

  // Initialize without setting RMW_INTROSPECT_DELEGATE_TO
  ASSERT_EQ(rmw_init(&options, &context), RMW_RET_OK);

  // Should be in recording-only mode
  EXPECT_TRUE(rmw_introspect::internal::is_recording_only_mode());
  EXPECT_FALSE(rmw_introspect::internal::is_intermediate_mode());
  EXPECT_EQ(rmw_introspect::internal::g_real_rmw, nullptr);

  // Context should have no wrapper
  EXPECT_EQ(context.impl, nullptr);

  // Cleanup
  ASSERT_EQ(rmw_shutdown(&context), RMW_RET_OK);
  ASSERT_EQ(rmw_context_fini(&context), RMW_RET_OK);
  ASSERT_EQ(rmw_init_options_fini(&options), RMW_RET_OK);
}

TEST_F(TestInitIntermediate, IntermediateModeWithFastRTPS)
{
  // Set environment to use FastRTPS
  setenv("RMW_INTROSPECT_DELEGATE_TO", "rmw_fastrtps_cpp", 1);

  // Create init options
  rmw_init_options_t options = rmw_get_zero_initialized_init_options();
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  ASSERT_EQ(rmw_init_options_init(&options, allocator), RMW_RET_OK);

  // Create context
  rmw_context_t context = rmw_get_zero_initialized_context();

  // Initialize with RMW_INTROSPECT_DELEGATE_TO set
  rmw_ret_t ret = rmw_init(&options, &context);

  if (ret != RMW_RET_OK) {
    GTEST_SKIP() << "rmw_fastrtps_cpp not available, skipping test";
  }

  // Should be in intermediate mode
  EXPECT_FALSE(rmw_introspect::internal::is_recording_only_mode());
  EXPECT_TRUE(rmw_introspect::internal::is_intermediate_mode());
  EXPECT_NE(rmw_introspect::internal::g_real_rmw, nullptr);

  // Context should have a wrapper
  EXPECT_NE(context.impl, nullptr);

  auto* wrapper = static_cast<rmw_introspect::ContextWrapper*>(context.impl);
  EXPECT_NE(wrapper->real_context, nullptr);
  EXPECT_EQ(wrapper->real_rmw, rmw_introspect::internal::g_real_rmw);
  EXPECT_EQ(wrapper->real_rmw_name, "rmw_fastrtps_cpp");

  // Cleanup
  ASSERT_EQ(rmw_shutdown(&context), RMW_RET_OK);
  ASSERT_EQ(rmw_context_fini(&context), RMW_RET_OK);
  ASSERT_EQ(rmw_init_options_fini(&options), RMW_RET_OK);

  // Real RMW should be unloaded
  EXPECT_EQ(rmw_introspect::internal::g_real_rmw, nullptr);
  EXPECT_TRUE(rmw_introspect::internal::is_recording_only_mode());
}

TEST_F(TestInitIntermediate, IntermediateModeWithCycloneDDS)
{
  // Set environment to use CycloneDDS
  setenv("RMW_INTROSPECT_DELEGATE_TO", "rmw_cyclonedds_cpp", 1);

  // Create init options
  rmw_init_options_t options = rmw_get_zero_initialized_init_options();
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  ASSERT_EQ(rmw_init_options_init(&options, allocator), RMW_RET_OK);

  // Create context
  rmw_context_t context = rmw_get_zero_initialized_context();

  // Initialize with RMW_INTROSPECT_DELEGATE_TO set
  rmw_ret_t ret = rmw_init(&options, &context);

  if (ret != RMW_RET_OK) {
    GTEST_SKIP() << "rmw_cyclonedds_cpp not available, skipping test";
  }

  // Should be in intermediate mode
  EXPECT_TRUE(rmw_introspect::internal::is_intermediate_mode());
  EXPECT_NE(rmw_introspect::internal::g_real_rmw, nullptr);

  // Cleanup
  ASSERT_EQ(rmw_shutdown(&context), RMW_RET_OK);
  ASSERT_EQ(rmw_context_fini(&context), RMW_RET_OK);
  ASSERT_EQ(rmw_init_options_fini(&options), RMW_RET_OK);

  // Real RMW should be unloaded
  EXPECT_EQ(rmw_introspect::internal::g_real_rmw, nullptr);
}

TEST_F(TestInitIntermediate, InvalidDelegate)
{
  // Set environment to invalid RMW
  setenv("RMW_INTROSPECT_DELEGATE_TO", "rmw_nonexistent", 1);

  // Create init options
  rmw_init_options_t options = rmw_get_zero_initialized_init_options();
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  ASSERT_EQ(rmw_init_options_init(&options, allocator), RMW_RET_OK);

  // Create context
  rmw_context_t context = rmw_get_zero_initialized_context();

  // Initialize should fail
  EXPECT_EQ(rmw_init(&options, &context), RMW_RET_ERROR);

  // Should not be in intermediate mode
  EXPECT_TRUE(rmw_introspect::internal::is_recording_only_mode());
  EXPECT_EQ(rmw_introspect::internal::g_real_rmw, nullptr);

  // Cleanup options
  ASSERT_EQ(rmw_init_options_fini(&options), RMW_RET_OK);
}

TEST_F(TestInitIntermediate, MultipleContexts)
{
  setenv("RMW_INTROSPECT_DELEGATE_TO", "rmw_fastrtps_cpp", 1);

  // Create two contexts
  rmw_init_options_t options1 = rmw_get_zero_initialized_init_options();
  rmw_init_options_t options2 = rmw_get_zero_initialized_init_options();
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  ASSERT_EQ(rmw_init_options_init(&options1, allocator), RMW_RET_OK);
  ASSERT_EQ(rmw_init_options_init(&options2, allocator), RMW_RET_OK);

  rmw_context_t context1 = rmw_get_zero_initialized_context();
  rmw_context_t context2 = rmw_get_zero_initialized_context();

  // Initialize first context
  rmw_ret_t ret1 = rmw_init(&options1, &context1);
  if (ret1 != RMW_RET_OK) {
    GTEST_SKIP() << "rmw_fastrtps_cpp not available, skipping test";
  }

  EXPECT_TRUE(rmw_introspect::internal::is_intermediate_mode());
  EXPECT_NE(rmw_introspect::internal::g_real_rmw, nullptr);

  // Initialize second context - should reuse existing real RMW
  ASSERT_EQ(rmw_init(&options2, &context2), RMW_RET_OK);
  EXPECT_TRUE(rmw_introspect::internal::is_intermediate_mode());
  EXPECT_NE(rmw_introspect::internal::g_real_rmw, nullptr);

  // Cleanup first context - real RMW should still be loaded
  ASSERT_EQ(rmw_shutdown(&context1), RMW_RET_OK);
  ASSERT_EQ(rmw_context_fini(&context1), RMW_RET_OK);
  EXPECT_TRUE(rmw_introspect::internal::is_intermediate_mode());
  EXPECT_NE(rmw_introspect::internal::g_real_rmw, nullptr);

  // Cleanup second context - real RMW should now be unloaded
  ASSERT_EQ(rmw_shutdown(&context2), RMW_RET_OK);
  ASSERT_EQ(rmw_context_fini(&context2), RMW_RET_OK);
  EXPECT_TRUE(rmw_introspect::internal::is_recording_only_mode());
  EXPECT_EQ(rmw_introspect::internal::g_real_rmw, nullptr);

  // Cleanup options
  ASSERT_EQ(rmw_init_options_fini(&options1), RMW_RET_OK);
  ASSERT_EQ(rmw_init_options_fini(&options2), RMW_RET_OK);
}

int main(int argc, char ** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
