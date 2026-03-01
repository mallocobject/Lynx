cmake_minimum_required(VERSION 3.25)
project(MyProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Lynx_SignServer main.cpp email.cpp)

find_library(MYSQL_CONNECTOR_CPP NAMES mysqlcppconn)

target_include_directories(Lynx_SignServer PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    /usr/local/include
)

find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
# find_package(mysql-concpp REQUIRED)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(Lynx_SignServer PRIVATE /usr/local/lib/liblynx_lib_d.a)
else()
    target_link_libraries(Lynx_SignServer PRIVATE /usr/local/lib/liblynx_lib.a)
endif()

target_link_libraries(Lynx_SignServer PRIVATE
    ${MYSQL_CONNECTOR_CPP} 
    OpenSSL::SSL
    OpenSSL::Crypto
    CURL::libcurl
    # mysql::concpp
)

target_compile_definitions(Lynx_SignServer PRIVATE 
    LYNX_WEB_SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
