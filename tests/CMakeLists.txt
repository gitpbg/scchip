cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(tests)

set (SRC src/ramtest.cpp)
add_executable(ramtest ${SRC})
target_link_libraries(ramtest PUBLIC modules)

add_test(NAME ramtest COMMAND ramtest)
