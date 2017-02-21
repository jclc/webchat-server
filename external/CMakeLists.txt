### This part of this file is modified from uWebSockets' CMakeLists.txt, because
# it doesn't let us to not build the examples 

#cmake_minimum_required(VERSION 3.1)
#set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_CXX_STANDARD 11)
#project (µWebSockets)

#option(BUILD_SHARED_LIBS "Build shared libraries." ON)

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

if(NOT LIBUV_INCLUDE_DIR)
    find_path(LIBUV_INCLUDE_DIR uv.h)
endif()
if(NOT LIBUV_LIBRARY)
    find_library(LIBUV_LIBRARY NAMES uv uv1)
endif()

add_library(uWS uWebSockets/src/Extensions.cpp uWebSockets/src/Group.cpp uWebSockets/src/WebSocketImpl.cpp uWebSockets/src/Networking.cpp uWebSockets/src/Hub.cpp uWebSockets/src/Node.cpp uWebSockets/src/WebSocket.cpp uWebSockets/src/HTTPSocket.cpp uWebSockets/src/Socket.cpp)
target_include_directories(uWS PUBLIC uWebSockets/src)

target_include_directories(uWS PUBLIC ${LIBUV_INCLUDE_DIR})
target_include_directories(uWS PUBLIC ${ZLIB_INCLUDE_DIRS})
target_include_directories(uWS PUBLIC ${OPENSSL_INCLUDE_DIR})
target_link_libraries (uWS LINK_PUBLIC ${LIBUV_LIBRARY})
target_link_libraries (uWS LINK_PUBLIC ${OPENSSL_SSL_LIBRARY})
target_link_libraries (uWS LINK_PUBLIC ${OPENSSL_CRYPTO_LIBRARY})
target_link_libraries (uWS LINK_PUBLIC ${ZLIB_LIBRARY})

if (UNIX)
target_link_libraries (uWS LINK_PUBLIC pthread)
install (TARGETS uWS DESTINATION /usr/lib64)
install (FILES uWebSockets/src/Extensions.h uWebSockets/src/WebSocketProtocol.h uWebSockets/src/Networking.h uWebSockets/src/WebSocket.h uWebSockets/src/Hub.h uWebSockets/src/Group.h uWebSockets/src/Node.h uWebSockets/src/Socket.h uWebSockets/src/HTTPSocket.h uWebSockets/src/uWS.h DESTINATION /usr/include/uWS)
endif (UNIX)

###
