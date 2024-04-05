cmake_minimum_required(VERSION 3.10)
project(Muduo_C11 VERSION 1.0)

include_directories(${PROJECT_SOURCE_DIR}/include/base)
include_directories(${PROJECT_SOURCE_DIR}/include/net)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")

ADD_SUBDIRECTORY(src/base)
ADD_SUBDIRECTORY(src/net)