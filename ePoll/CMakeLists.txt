cmake_minimum_required(VERSION 3.0)

project(epoll)

file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/tcpServerEpoll.cpp)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(epoll_IO ${SRC})