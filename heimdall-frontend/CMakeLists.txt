cmake_minimum_required(VERSION 2.8.11)

project(heimdall-frontend)

set(LIBPIT_INCLUDE_DIRS
    ../libpit/source)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON) # moc files are generated in build (current) directory

find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(ZLIB REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

if(MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
endif(MINGW)

if(ENABLE_QML_DEBUGGING)
    add_definitions(-DENABLE_QML_DEBUGGING)
endif(ENABLE_QML_DEBUGGING)

include_directories(${LIBPIT_INCLUDE_DIRS})

set(HEIMDALL_FRONTEND_SOURCE_FILES
    source/aboutform.cpp
    source/Alerts.cpp
    source/FirmwareInfo.cpp
    source/GZipFile.cpp
    source/main.cpp
    source/mainwindow.cpp
    source/PackageData.cpp
    source/Packaging.cpp)

set(HEIMDALL_FRONTEND_QML_FILES
    source/qml/DropFiles.qml
    source/qml/DropFilesForm.qml
    source/qml/FileUtils.js
    source/qml/main.qml
    source/qml/Root.qml
    source/qml/RootForm.qml)

qt5_wrap_ui(HEIMDALL_FRONTEND_FORMS
    mainwindow.ui
    aboutform.ui)

qt5_add_resources(HEIMDALL_FRONTEND_RESOURCES
    source/qml/qml.qrc
    source/qml/resources.qrc)

add_executable(heimdall-frontend WIN32
    MACOSX_BUNDLE
    ${HEIMDALL_FRONTEND_SOURCE_FILES}
    ${HEIMDALL_FRONTEND_FORMS} # So IDEs know about forms
    ${HEIMDALL_FRONTEND_QML_FILES} # So IDEs know about QML
    ${HEIMDALL_FRONTEND_RESOURCES})

include(LargeFiles)
use_large_files(heimdall-frontend YES)

set_property(TARGET heimdall-frontend
    APPEND PROPERTY COMPILE_DEFINITIONS "QT_LARGEFILE_SUPPORT")

target_link_libraries(heimdall-frontend
    pit
    Qt5::Qml
    Qt5::Quick
    Qt5::Widgets
    z)
