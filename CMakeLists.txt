cmake_minimum_required(VERSION 3.1)
project("webchat-server")

set(CMAKE_CXX_STANDARD 11)
set(BUILD_SHARED_LIBS on)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules")
add_definitions(-DASIO_STANDALONE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}") # probably redundant
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

find_package(SQLITE3 REQUIRED)
include_directories()
#add_subdirectory("external/websocketpp")

#add_subdirectory("external")
#include_directories("${CMAKE_SOURCE_DIR}/external/uWebSockets/src")
include_directories("${CMAKE_SOURCE_DIR}/src"
	"${CMAKE_SOURCE_DIR}/external/asio/include"
	"${CMAKE_SOURCE_DIR}/external/websocketpp")
set(SOURCES
	src/main.cpp
	src/optparse/optparse.c
	src/database.cpp
	)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
	${SQLITE3_LIBRARY}
	)
