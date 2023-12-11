cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(scchip)
list(APPEND CMAKE_MODULE_PATH /home/gharpure/.local/lib)

find_package(SystemCLanguage)
set (CMAKE_CXX_STANDARD ${SystemC_CXX_STANDARD})

add_subdirectory(common)
add_subdirectory(modules)
add_subdirectory(scchip)
include(CTest)
add_subdirectory(tests)