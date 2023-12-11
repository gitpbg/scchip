cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(modules)

set (SRC src/ram.cpp)

add_library(modules ${SRC})
target_link_libraries(modules PUBLIC common)
target_include_directories(modules PUBLIC ./src)
