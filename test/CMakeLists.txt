cmake_minimum_required(VERSION 3.25)
project(MyProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Lynx_test test_sql.cpp)

target_include_directories(Lynx_test PRIVATE 
    /usr/local/include
)

# link_directories(/usr/local/lib)

find_library(MYSQL_CONNECTOR_CPP NAMES mysqlcppconn)

target_link_libraries(Lynx_test PRIVATE /usr/local/lib/liblynx_lib_d.a ${MYSQL_CONNECTOR_CPP})