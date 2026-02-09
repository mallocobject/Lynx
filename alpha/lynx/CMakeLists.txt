set(LIB_SUB_DIRS base tcp logger)
set(LIB_SOURCES)

foreach(dir ${LIB_SUB_DIRS})
    file(GLOB dir_sources CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.cpp")
    list(APPEND LIB_SOURCES ${dir_sources})
endforeach()

add_library(lynx_lib STATIC ${LIB_SOURCES})

target_include_directories(lynx_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)