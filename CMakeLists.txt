cmake_minimum_required(VERSION 2.8.11)

set(Qt5Quick_DIR "/usr/local/Cellar/qt5/5.4.0/lib/cmake/Qt5Quick" CACHE FILEPATH "Qt5Quick path" FORCE)

set(CMAKE_MODULE_PATH
    ${CMAKE_SOURCE_DIR}/cmake
    ${CMAKE_MODULE_PATH})

project(Heimdall)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(libpit)
add_subdirectory(heimdall)
add_subdirectory(heimdall-frontend)

add_dependencies(heimdall-frontend heimdall)
