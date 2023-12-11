cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(scchip)

set (SRC src/main.cpp)

add_executable(scchip ${SRC})
target_link_libraries(scchip PUBLIC modules)
