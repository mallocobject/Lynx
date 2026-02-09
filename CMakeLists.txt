cmake_minimum_required(VERSION 3.25)
project(Lynx)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()


# set(LOG_LEVEL "lynx::OFF")
# message(STATUS "Current logger level: ${LOG_LEVEL}")
# add_definitions(-DLOGGER_LEVEL_SETTING=${LOG_LEVEL})

message(STATUS "cmake build mode: ${CMAKE_BUILD_TYPE}")

add_subdirectory(test)
add_subdirectory(alpha/lynx)
# add_subdirectory(examples)