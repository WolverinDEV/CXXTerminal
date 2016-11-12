cmake_minimum_required(VERSION 3.6)
project(CXXTerminal)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp include/Terminal.h src/Terminal.cpp)
add_executable(CXXTerminal ${SOURCE_FILES})