cmake_minimum_required(VERSION 3.10)
project(Network_Library VERSION 1.0)

include_directories(${PROJECT_SOURCE_DIR}/include/base)
include_directories(${PROJECT_SOURCE_DIR}/include/net)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++20 -fPIC")

ADD_SUBDIRECTORY(src/base)
ADD_SUBDIRECTORY(src/net)