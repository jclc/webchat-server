cmake_minimum_required(VERSION 3.1)
project("webchat-server")

set(CMAKE_CXX_STANDARD 11)
set(BUILD_SHARED_LIBS on)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}") # probably redundant
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

find_package(SQLITE3 REQUIRED)

add_subdirectory("external")
add_subdirectory("src/database")
include_directories("${CMAKE_SOURCE_DIR}/external/uWebSockets/src")
include_directories("${CMAKE_SOURCE_DIR}/src")


add_executable(${PROJECT_NAME}
		src/main.cpp
		src/optparse/optparse.c
		)

target_link_libraries(${PROJECT_NAME}
		uWS
		database
		)

#link_libraries(${PROJECT_NAME}
#		SQLITE3
#		)
