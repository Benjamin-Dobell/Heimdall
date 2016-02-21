cmake_minimum_required(VERSION 2.8.4)

set(CMAKE_MODULE_PATH
    ${CMAKE_SOURCE_DIR}/cmake
    ${CMAKE_MODULE_PATH})

project(Heimdall)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(DISABLE_FRONTEND "Disable GUI frontend" OFF)

add_subdirectory(libpit)
add_subdirectory(heimdall)
if(NOT DISABLE_FRONTEND)
    add_subdirectory(heimdall-frontend)
    add_dependencies(heimdall-frontend heimdall)
endif()
