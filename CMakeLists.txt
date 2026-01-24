cmake_minimum_required(VERSION 3.25)
project(Lynx)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB SRC_FILES "src/*.cpp")

add_executable(lynx main.cpp ${SRC_FILES})
add_executable(client client.cpp ${SRC_FILES})

target_include_directories(lynx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)