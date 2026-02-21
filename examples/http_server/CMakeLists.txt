cmake_minimum_required(VERSION 3.25)
project(MyProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Lynx_WebServer main.cpp)

target_include_directories(Lynx_WebServer PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    /usr/local/include
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(Lynx_WebServer PRIVATE /usr/local/lib/liblynx_lib_d.a)
else()
    target_link_libraries(Lynx_WebServer PRIVATE /usr/local/lib/liblynx_lib.a)
endif()

target_compile_definitions(Lynx_WebServer PRIVATE 
    LYNX_WEB_SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
