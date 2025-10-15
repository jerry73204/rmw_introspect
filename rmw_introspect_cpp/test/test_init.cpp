#include <gtest/gtest.h>
#include "rmw/rmw.h"
#include "rmw/init.h"
#include "rmw/init_options.h"
#include "rcutils/allocator.h"

// Test that implementation identifier is correct
TEST(TestInit, ImplementationIdentifier) {
  const char * identifier = rmw_get_implementation_identifier();
  ASSERT_NE(identifier, nullptr);
  EXPECT_STREQ(identifier, "rmw_introspect_cpp");
}

// Test init options initialization
TEST(TestInit, InitOptionsInit) {
  rmw_init_options_t options = rmw_get_zero_initialized_init_options();
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  rmw_ret_t ret = rmw_init_options_init(&options, allocator);
  ASSERT_EQ(ret, RMW_RET_OK);
  EXPECT_STREQ(options.implementation_identifier, "rmw_introspect_cpp");

  ret = rmw_init_options_fini(&options);
  ASSERT_EQ(ret, RMW_RET_OK);
}

// Test init options copy
TEST(TestInit, InitOptionsCopy) {
  rmw_init_options_t src = rmw_get_zero_initialized_init_options();
  rmw_init_options_t dst = rmw_get_zero_initialized_init_options();
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  rmw_ret_t ret = rmw_init_options_init(&src, allocator);
  ASSERT_EQ(ret, RMW_RET_OK);

  ret = rmw_init_options_copy(&src, &dst);
  ASSERT_EQ(ret, RMW_RET_OK);
  EXPECT_STREQ(dst.implementation_identifier, "rmw_introspect_cpp");

  ret = rmw_init_options_fini(&src);
  EXPECT_EQ(ret, RMW_RET_OK);
  ret = rmw_init_options_fini(&dst);
  EXPECT_EQ(ret, RMW_RET_OK);
}

// Test RMW initialization and shutdown
TEST(TestInit, RmwInitShutdown) {
  rmw_init_options_t options = rmw_get_zero_initialized_init_options();
  rmw_context_t context = rmw_get_zero_initialized_context();
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  rmw_ret_t ret = rmw_init_options_init(&options, allocator);
  ASSERT_EQ(ret, RMW_RET_OK);

  ret = rmw_init(&options, &context);
  ASSERT_EQ(ret, RMW_RET_OK);
  EXPECT_STREQ(context.implementation_identifier, "rmw_introspect_cpp");

  ret = rmw_shutdown(&context);
  ASSERT_EQ(ret, RMW_RET_OK);

  ret = rmw_context_fini(&context);
  ASSERT_EQ(ret, RMW_RET_OK);

  ret = rmw_init_options_fini(&options);
  EXPECT_EQ(ret, RMW_RET_OK);
}

int main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
