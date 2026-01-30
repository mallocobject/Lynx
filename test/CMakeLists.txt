cmake_minimum_required(VERSION 3.25)
project(LynxTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(Lynx http_context_test.cpp)
target_link_libraries(Lynx PUBLIC lynx_lib)

target_include_directories(Lynx PUBLIC ${CMAKE_SOURCE_DIR/..})