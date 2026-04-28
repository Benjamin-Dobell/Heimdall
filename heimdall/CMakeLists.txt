cmake_minimum_required(VERSION 3.10)

project(heimdall)

if((NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux") AND (NOT DEFINED libusb_USE_STATIC_LIBS))
    set(libusb_USE_STATIC_LIBS YES)
endif((NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux") AND (NOT DEFINED libusb_USE_STATIC_LIBS))

find_package(libusb REQUIRED)

set(LIBPIT_INCLUDE_DIRS
    ../libpit/source)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
endif(MINGW)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_definitions(-DOS_LINUX)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

include_directories(SYSTEM ${LIBUSB_INCLUDE_DIRS})

include_directories(${LIBPIT_INCLUDE_DIRS})

set(HEIMDALL_SOURCE_FILES
    source/Arguments.cpp
    source/BridgeManager.cpp
    source/ClosePcScreenAction.cpp
    source/DetectAction.cpp
    source/DownloadPitAction.cpp
    source/FlashAction.cpp
    source/HelpAction.cpp
    source/InfoAction.cpp
    source/Interface.cpp
    source/main.cpp
    source/PrintPitAction.cpp
    source/Utility.cpp
    source/VersionAction.cpp)

include(LargeFiles)
use_large_files(heimdall YES)
add_executable(heimdall ${HEIMDALL_SOURCE_FILES})

target_link_libraries(heimdall PRIVATE pit)
target_link_libraries(heimdall PRIVATE ${LIBUSB_LIBRARY})

if(APPLE)
find_library(CORE_FOUNDATION CoreFoundation REQUIRED)
find_library(IOKIT IOKit REQUIRED)
find_library(SECURITY Security REQUIRED)
target_link_libraries(heimdall PRIVATE ${CORE_FOUNDATION} ${IOKIT} ${SECURITY})
endif(APPLE)

install (TARGETS heimdall
		RUNTIME	DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
		LIBRARY	DESTINATION ${CMAKE_INSTALL_LIBDIR})

