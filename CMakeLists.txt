cmake_minimum_required(VERSION 3.25)
project(Lynx)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DNODEBUG)
    message(STATUS "Release mode: NODEBUG defined")
endif()

# add_subdirectory(test)
add_subdirectory(lynx)
add_subdirectory(examples)