cmake_minimum_required(VERSION 3.28)

project(select)

file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/tcpServerSelect.cpp)

set(CMAKE_CXX_STANDARD 11)

add_executable(select_IO ${SRC})