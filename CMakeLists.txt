cmake_minimum_required(VERSION 3.25)
project(Lynx)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()


set(LOG_LEVEL "INFO" CACHE STRING "Logger level")
set_property(CACHE LOG_LEVEL PROPERTY STRINGS 
    TRACE
    DEBUG
    INFO
    WARN
    ERROR
    FATAL
)

message(STATUS "Current logger level: ${LOG_LEVEL}")
add_definitions(-DLOGGER_LEVEL_SETTING=LYNX_${LOG_LEVEL})
message(STATUS "cmake build mode: ${CMAKE_BUILD_TYPE}")

add_subdirectory(lynx)
add_subdirectory(test)

# 安装库文件到 /usr/local/lib
install(TARGETS lynx_lib
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

# 安装公开头文件到 /usr/local/include/lynx
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/lynx/
    DESTINATION include/lynx
)