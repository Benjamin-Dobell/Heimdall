cmake_minimum_required(VERSION 3.11)
project(libpit)

set(LIBPIT_SOURCE_FILES
    source/libpit.cpp)

add_library(pit STATIC ${LIBPIT_SOURCE_FILES})

target_compile_features(pit PRIVATE cxx_std_11)
