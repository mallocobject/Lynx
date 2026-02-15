cmake_minimum_required(VERSION 3.25)
project(MyProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Lynx_EchoServer main.cpp)

target_include_directories(Lynx_EchoServer PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    /usr/local/include
)

link_directories(/usr/local/lib)

target_link_libraries(Lynx_EchoServer PRIVATE lynx_lib)

target_compile_definitions(Lynx_EchoServer PRIVATE 
    LYNX_WEB_SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
