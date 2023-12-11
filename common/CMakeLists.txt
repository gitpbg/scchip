cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(common)

add_library(common INTERFACE)
target_include_directories(common INTERFACE ./src)
target_link_libraries(common INTERFACE SystemC::systemc)
