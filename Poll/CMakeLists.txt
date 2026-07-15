cmake_minimum_required(VERSION 3.2)

project(poll)

file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(poll_IO ${SRC})