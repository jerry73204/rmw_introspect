cmake_minimum_required(VERSION 3.8)
project(rmw_introspect_cpp)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rcutils REQUIRED)
find_package(rmw REQUIRED)
find_package(rosidl_typesupport_introspection_cpp REQUIRED)
find_package(rosidl_typesupport_introspection_c REQUIRED)
find_package(rosidl_typesupport_cpp REQUIRED)
find_package(rmw_dds_common REQUIRED)

# RMW introspect library
add_library(${PROJECT_NAME} SHARED
  src/rmw_init.cpp
  src/rmw_node.cpp
  src/rmw_publisher.cpp
  src/rmw_subscription.cpp
  src/rmw_service.cpp
  src/rmw_client.cpp
  src/rmw_wait.cpp
  src/rmw_graph.cpp
  src/rmw_serialize.cpp
  src/rmw_event.cpp
  src/rmw_network_flow.cpp
  src/rmw_utils.cpp
  src/data.cpp
  src/type_support.cpp
  # Phase 4 stub implementations
  src/rmw_gid.cpp
  src/rmw_qos_compat.cpp
  src/rmw_info.cpp
  src/rmw_allocation.cpp
  src/rmw_callbacks.cpp
  src/rmw_features.cpp
  src/rmw_logging.cpp
  # Phase 1: Core infrastructure
  src/real_rmw.cpp
  src/wrappers.cpp
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(${PROJECT_NAME}
  rcutils
  rmw
  rosidl_typesupport_introspection_cpp
  rosidl_typesupport_introspection_c
  rosidl_typesupport_cpp
  rmw_dds_common
)

# Link dl library for dlopen/dlsym support
target_link_libraries(${PROJECT_NAME}
  ${CMAKE_DL_LIBS}
)

# Export library
ament_export_targets(${PROJECT_NAME}_targets HAS_LIBRARY_TARGET)
ament_export_dependencies(
  rcutils
  rmw
  rosidl_typesupport_introspection_cpp
  rosidl_typesupport_introspection_c
  rosidl_typesupport_cpp
  rmw_dds_common
)

# Install library
install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}_targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)

# Install headers
install(
  DIRECTORY include/
  DESTINATION include
)

# Testing
if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  find_package(std_msgs REQUIRED)
  find_package(std_srvs REQUIRED)

  ament_add_gtest(test_init test/test_init.cpp)
  target_link_libraries(test_init ${PROJECT_NAME})
  ament_target_dependencies(test_init rcutils rmw)

  ament_add_gtest(test_data test/test_data.cpp)
  target_link_libraries(test_data ${PROJECT_NAME})
  ament_target_dependencies(test_data rcutils)

  ament_add_gtest(test_phase1 test/test_phase1.cpp)
  target_link_libraries(test_phase1 ${PROJECT_NAME})
  ament_target_dependencies(test_phase1 rcutils rmw std_msgs rosidl_typesupport_cpp)

  ament_add_gtest(test_phase2 test/test_phase2.cpp)
  target_link_libraries(test_phase2 ${PROJECT_NAME})
  ament_target_dependencies(test_phase2 rcutils rmw std_srvs rosidl_typesupport_cpp)

  ament_add_gtest(test_phase3 test/test_phase3.cpp)
  target_link_libraries(test_phase3 ${PROJECT_NAME})
  ament_target_dependencies(test_phase3 rcutils rmw)

  ament_add_gtest(test_phase4_stubs test/test_phase4_stubs.cpp)
  target_link_libraries(test_phase4_stubs ${PROJECT_NAME})
  ament_target_dependencies(test_phase4_stubs rcutils rmw std_msgs std_srvs rosidl_typesupport_cpp)

  # Phase 1 tests: Core infrastructure
  ament_add_gtest(test_real_rmw test/test_real_rmw.cpp)
  target_link_libraries(test_real_rmw ${PROJECT_NAME})
  ament_target_dependencies(test_real_rmw rcutils)

  ament_add_gtest(test_init_intermediate test/test_init_intermediate.cpp)
  target_link_libraries(test_init_intermediate ${PROJECT_NAME})
  ament_target_dependencies(test_init_intermediate rcutils rmw)
endif()

ament_package()
