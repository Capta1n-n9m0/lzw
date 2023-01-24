cmake_minimum_required(VERSION 3.24)
project(lzw C)

set(CMAKE_C_STANDARD 11)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
endif ()

add_executable(lzw main.c)
