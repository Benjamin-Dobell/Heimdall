cmake_minimum_required(VERSION 2.8.4)

project(heimdall-frontend)

set(LIBPIT_INCLUDE_DIRS
    ../libpit/source)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON) # moc files are generated in build (current) directory

find_package(Qt5Widgets REQUIRED)
find_package(ZLIB REQUIRED)

if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lobjc -framework IOKit -framework CoreFoundation")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
endif()

if(MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
endif(MINGW)

include_directories(${LIBPIT_INCLUDE_DIRS})

set(HEIMDALL_FRONTEND_SOURCE_FILES
    source/aboutform.cpp
    source/Alerts.cpp
    source/FirmwareInfo.cpp
    source/main.cpp
    source/mainwindow.cpp
    source/PackageData.cpp
    source/Packaging.cpp)

qt5_wrap_ui(HEIMDALL_FRONTEND_FORMS
    mainwindow.ui
    aboutform.ui)

qt5_add_resources(HEIMDALL_FRONTEND_RESOURCES
    mainwindow.qrc)

add_executable(heimdall-frontend WIN32
    MACOSX_BUNDLE
    ${HEIMDALL_FRONTEND_SOURCE_FILES}
    ${HEIMDALL_FRONTEND_FORMS}
    ${HEIMDALL_FRONTEND_RESOURCES})

include(LargeFiles)
use_large_files(heimdall-frontend YES)

set_property(TARGET heimdall-frontend
    APPEND PROPERTY COMPILE_DEFINITIONS "QT_LARGEFILE_SUPPORT")

target_link_libraries(heimdall-frontend pit)
target_link_libraries(heimdall-frontend Qt5::Widgets)
target_link_libraries(heimdall-frontend z)
install (TARGETS heimdall-frontend
		RUNTIME	DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
		BUNDLE	DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
		LIBRARY	DESTINATION ${CMAKE_INSTALL_LIBDIR}
		BUNDLE DESTINATION /Applications)

