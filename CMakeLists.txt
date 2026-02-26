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
# add_subdirectory(test)

# 安装库文件到 /usr/local/lib
install(TARGETS lynx_lib
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

# 安装公开头文件到 /usr/local/include/lynx
set(LYNX_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lynx)
set(LYNX_SUBDIRS base http json logger sql tcp time)

foreach(subdir ${LYNX_SUBDIRS})
    file(GLOB_RECURSE SUBDIR_HPP_FILES "${LYNX_INCLUDE_DIR}/${subdir}/*.hpp")
    list(APPEND ALL_HPP_FILES ${SUBDIR_HPP_FILES})
endforeach()

set(GENERATED_HEADER ${CMAKE_CURRENT_BINARY_DIR}/include/lynx/lynx.hpp)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/lynx)

# 写入文件内容：为每个收集到的 .hpp 生成 #include 语句
file(WRITE ${GENERATED_HEADER} "// Auto-generated lynx.hpp - includes all public headers\n\n")
foreach(hpp ${ALL_HPP_FILES})
    # 计算相对于 include/lynx 的路径（例如 base/alloc.hpp）
    file(RELATIVE_PATH rel_path ${LYNX_INCLUDE_DIR} ${hpp})
    file(APPEND ${GENERATED_HEADER} "#include <lynx/${rel_path}>\n")
endforeach()

install(
    DIRECTORY ${LYNX_INCLUDE_DIR}/
    DESTINATION include/lynx
    FILES_MATCHING PATTERN "*.hpp"
)

install(
    FILES ${GENERATED_HEADER}
    DESTINATION include/lynx
)