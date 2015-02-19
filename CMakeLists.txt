cmake_minimum_required(VERSION 2.8.4)

set(CMAKE_MODULE_PATH
    ${CMAKE_SOURCE_DIR}/cmake
    ${CMAKE_MODULE_PATH})

project(Heimdall)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(libpit)
add_subdirectory(heimdall)
add_subdirectory(heimdall-frontend)

add_dependencies(heimdall-frontend heimdall)
