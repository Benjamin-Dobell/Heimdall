cmake_minimum_required(VERSION 2.8.4)
project(libpit)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Dnullptr=NULL")

set(LIBPIT_SOURCE_FILES
    source/libpit.cpp)

add_library(pit STATIC ${LIBPIT_SOURCE_FILES})
