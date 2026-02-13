add_executable(Lynx_EchoServer main.cpp)

target_link_libraries(Lynx_EchoServer PRIVATE lynx_lib)

target_compile_definitions(Lynx_EchoServer PRIVATE 
    LYNX_WEB_SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)