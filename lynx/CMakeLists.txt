set(LIB_SUB_DIRS ${LYNX_SUBDIRS})
set(LIB_SOURCES)

message("-- LIB_SUB_DIRS = ${LIB_SUB_DIRS}")
foreach(dir ${LIB_SUB_DIRS})
    file(GLOB dir_sources CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.cpp")
    list(APPEND LIB_SOURCES ${dir_sources})
endforeach()

find_library(MYSQL_CONNECTOR_CPP NAMES mysqlcppconn)

if (NOT MYSQL_CONNECTOR_CPP)
    message(FATAL_ERROR "Could not find libmysqlcppconn. Please install libmysqlcppconn-dev")
endif()

add_library(lynx_lib STATIC ${LIB_SOURCES})

target_link_libraries(lynx_lib PUBLIC ${MYSQL_CONNECTOR_CPP})

target_include_directories(lynx_lib 
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..
    /usr/include
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(lynx_lib PROPERTIES DEBUG_POSTFIX "_d")
endif()
