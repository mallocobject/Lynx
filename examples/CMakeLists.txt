add_executable(Lynx_WebServer main.cpp)

target_link_libraries(Lynx_WebServer PRIVATE lynx_lib)

target_compile_definitions(Lynx_WebServer PRIVATE 
    LYNX_WEB_SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)