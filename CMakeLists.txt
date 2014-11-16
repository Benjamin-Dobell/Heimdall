cmake_minimum_required(VERSION 2.8.4)

set(CMAKE_MODULE_PATH
    ${CMAKE_SOURCE_DIR}/cmake
    ${CMAKE_MODULE_PATH})

project(Heimdall)

add_subdirectory(libpit)
add_subdirectory(heimdall)
