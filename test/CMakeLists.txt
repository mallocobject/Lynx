# test/CMakeLists.txt
cmake_minimum_required(VERSION 3.25)
project(LynxTest)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SRC_FILES "../src/*.cpp")

add_executable(test1 tcp_server_test.cpp ${SRC_FILES})
add_executable(test2 tcp_client_test.cpp ${SRC_FILES})

# 包含路径
target_include_directories(test1 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_include_directories(test2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)