cmake_minimum_required(VERSION 2.8.4)

project(heimdall)

if(WIN32)
    set(LIBUSB_1_INCLUDE_DIRS
        ../libusb-1.0/include)
else(WIN32)
    find_package(libusb-1.0 REQUIRED)
endif(WIN32)

set(LIBPIT_INCLUDE_DIRS
    ../libpit/Source)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

include_directories(SYSTEM ${LIBUSB_1_INCLUDE_DIRS})

include_directories(${LIBPIT_INCLUDE_DIRS})

set(HEIMDALL_SOURCE_FILES
    source/Arguments.cpp
    source/Arguments.h
    source/BeginDumpPacket.h
    source/BeginSessionPacket.h
    source/BridgeManager.cpp
    source/BridgeManager.h
    source/ClosePcScreenAction.cpp
    source/ClosePcScreenAction.h
    source/ControlPacket.h
    source/DetectAction.cpp
    source/DetectAction.h
    source/DeviceTypePacket.h
    source/DownloadPitAction.cpp
    source/DownloadPitAction.h
    source/DumpPartFileTransferPacket.h
    source/DumpPartPitFilePacket.h
    source/DumpResponse.h
    source/EndFileTransferPacket.h
    source/EndModemFileTransferPacket.h
    source/EndPhoneFileTransferPacket.h
    source/EndPitFileTransferPacket.h
    source/EndSessionPacket.h
    source/FilePartSizePacket.h
    source/FileTransferPacket.h
    source/FlashAction.cpp
    source/FlashAction.h
    source/FlashPartFileTransferPacket.h
    source/FlashPartPitFilePacket.h
    source/Heimdall.h
    source/HelpAction.cpp
    source/HelpAction.h
    source/InboundPacket.h
    source/InfoAction.cpp
    source/InfoAction.h
    source/Interface.cpp
    source/Interface.h
    source/main.cpp
    source/OutboundPacket.h
    source/Packet.h
    source/PitFilePacket.h
    source/PitFileResponse.h
    source/PrintPitAction.cpp
    source/PrintPitAction.h
    source/ReceiveFilePartPacket.h
    source/ResponsePacket.h
    source/SendFilePartPacket.h
    source/SendFilePartResponse.h
    source/SessionSetupPacket.h
    source/SessionSetupResponse.h
    source/SetupSessionPacket.h
    source/SetupSessionResponse.h
    source/TotalBytesPacket.h
    source/Utility.cpp
    source/Utility.h
    source/VersionAction.cpp
    source/VersionAction.h)

add_executable(heimdall ${HEIMDALL_SOURCE_FILES})

target_link_libraries(heimdall PRIVATE pit)

LINK_DIRECTORIES(../libusb-1.0/lib/mingw)
if(WIN32)
    target_link_libraries(heimdall PRIVATE ${CMAKE_SOURCE_DIR}/libusb-1.0/lib/mingw/libusb-1.0.a)
else(WIN32)
    target_link_libraries(heimdalll PRIVATE usb-1.0)
endif(WIN32)
