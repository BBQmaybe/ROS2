cmake_minimum_required(VERSION 3.5)

project(robot_app)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)

add_executable(go_round_the_circle src/circle_movement.cpp)
ament_target_dependencies(go_round_the_circle rclcpp geometry_msgs)

install (TARGETS go_round_the_circle DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()