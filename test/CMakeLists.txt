set(LIB_SUB_DIRS logger)
set(LIB_SOURCES)

foreach(dir ${LIB_SUB_DIRS})
    file(GLOB dir_sources CMAKE_CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/../lynx/${dir}/*.cpp")
    list(APPEND LIB_SOURCES ${dir_sources})
endforeach()

add_library(lynx_lib STATIC 
    ${LIB_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/../lynx/base/time_stamp.cpp
)

target_include_directories(lynx_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(Lynx_loggger file_appender_test.cpp)

target_link_libraries(Lynx_loggger PRIVATE lynx_lib)

