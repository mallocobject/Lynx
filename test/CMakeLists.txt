add_executable(Lynx_loggger event_loop_test.cpp)

target_include_directories(Lynx_loggger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(Lynx_loggger PRIVATE lynx_lib)

