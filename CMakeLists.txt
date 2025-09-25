cmake_minimum_required(VERSION 3.16)
project(memprof LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Incluye TODOS los .cpp que haya en src/memproflib/
file(GLOB MEMPROF_SRCS CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/src/memproflib/*.cpp")

add_library(memproflib STATIC ${MEMPROF_SRCS}
        src/memproflib/SocketClienteTest.cpp
        src/memproflib/SocketClienteTest.h)
target_include_directories(memproflib PUBLIC ${CMAKE_SOURCE_DIR}/src)

# Winsock en Windows
if (WIN32)
    target_link_libraries(memproflib ws2_32)
endif()

# Ejecutables de ejemplo
add_executable(demo_ok   ${CMAKE_SOURCE_DIR}/src/main_demo_ok.cpp
        src/memproflib/SocketClienteTest.cpp
        src/memproflib/SocketClienteTest.h)
target_link_libraries(demo_ok memproflib)

add_executable(demo_leak ${CMAKE_SOURCE_DIR}/src/main_demo_leak.cpp
        src/memproflib/SocketClienteTest.cpp
        src/memproflib/SocketClienteTest.h)
target_link_libraries(demo_leak memproflib)

