cmake_minimum_required(VERSION 4.0)
project(memprof)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_library(memproflib memproflib.cpp memproflib.h)

add_executable(memprof main.cpp)

target_link_libraries(memprof memproflib)