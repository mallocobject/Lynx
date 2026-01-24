# test/CMakeLists.txt
cmake_minimum_required(VERSION 3.25)
project(LynxTest)

set(CMAKE_CXX_STANDARD 17)

# 明确指定需要的源文件，排除根目录的 main.cpp
set(BUFFER_SRC "../src/buffer.cpp") 

add_executable(buffer_test buffer_test.cpp ${BUFFER_SRC})

# 包含路径
target_include_directories(buffer_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)