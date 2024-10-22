cmake_minimum_required(VERSION 3.5)
project(cpp_srvcli)

find_package(rosidl_default_generators REQUIRED)
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_srvs REQUIRED)
find_package(full_name_service REQUIRED)


add_executable(server_name src/server.cpp)
ament_target_dependencies(server_name rclcpp std_srvs full_name_service)

add_executable(client_name src/client.cpp)
ament_target_dependencies(client_name rclcpp std_srvs full_name_service)

install(TARGETS
  server_name
  client_name
  DESTINATION lib/${PROJECT_NAME})

ament_package()
