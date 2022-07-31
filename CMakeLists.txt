cmake_minimum_required(VERSION 3.22)
project(server C)

set(CMAKE_C_STANDARD 11)

add_subdirectory(ecglib)

include_directories(.)
include_directories(ecglib/include)

add_executable(server
        httpd.c
        main.c)

target_link_libraries(server ecglib)