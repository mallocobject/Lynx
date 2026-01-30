cmake_minimum_required(VERSION 3.25)
project(Lynx)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DNODEBUG)
endif()


file(GLOB_RECURSE LYNX_SOURCES "lynx/*/*.cpp")

add_library(lynx_lib STATIC ${LYNX_SOURCES})

target_include_directories(lynx_lib PUBLIC ${CMAKE_SOURCE_DIR})

add_subdirectory("test")