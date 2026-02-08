file(GLOB LIB_SOURCES CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(logger_lib STATIC ${LIB_SOURCES})

target_include_directories(logger_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
