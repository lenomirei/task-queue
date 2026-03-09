cmake_minimum_required(VERSION 3.17)

project(TaskQueueExample)

file(GLOB SRCS
    "${CMAKE_CURRENT_LIST_DIR}/*.cc"
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/examples/example.cpp")

add_executable(${PROJECT_NAME} ${SRCS})