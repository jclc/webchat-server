cmake_minimum_required(VERSION 2.8)
project("webchat-server")

add_subdirectory("external/uWebSockets")

include_directories("exteral/uWebSockets/src")
add_executable(${PROJECT_NAME} "main.cpp")
target_link_libraries(${PROJECT_NAME} uWS)
