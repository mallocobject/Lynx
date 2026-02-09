add_executable(test logger_test.cpp)

target_include_directories(test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../alpha)

target_link_libraries(test PRIVATE lynx_lib)

