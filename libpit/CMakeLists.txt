cmake_minimum_required(VERSION 3.10)
project(libpit)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LIBPIT_SOURCE_FILES
    source/libpit.cpp)

add_library(pit STATIC ${LIBPIT_SOURCE_FILES})
