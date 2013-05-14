LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
  core.c \
  descriptor.c \
  io.c \
  sync.c

ifeq ($(HOST_OS),darwin)
LOCAL_SRC_FILES += os/darwin_usb.c
endif
ifeq ($(HOST_OS),linux)
LOCAL_SRC_FILES += os/linux_usbfs.c
endif
  
LOCAL_SRC_FILES := $(addprefix ../../../libusb/libusb/,$(LOCAL_SRC_FILES))
LOCAL_MODULE := libusb

LOCAL_C_INCLUDES := \
  external/libusb \
  external/libusb/libusb

include $(BUILD_HOST_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
  Arguments.cpp \
  ClosePcScreenAction.cpp \
  DownloadPitAction.cpp \
  FlashAction.cpp \
  InfoAction.cpp \
  PrintPitAction.cpp \
  VersionAction.cpp \
  BridgeManager.cpp \
  DetectAction.cpp \
  DumpAction.cpp \
  HelpAction.cpp \
  Interface.cpp \
  Utility.cpp \
  main.cpp \
  ../../libpit/Source/libpit.cpp
  
LOCAL_SRC_FILES += $(LIBUSB_SRC_FILES)

LOCAL_C_INCLUDES := \
  external/heimdall/libpit/Source \
  external/libusb/libusb

LOCAL_STATIC_LIBRARIES := libusb

ifeq ($(HOST_OS),darwin)
LOCAL_LDFLAGS := -framework CoreFoundation -framework IOKit
endif

ifeq ($(HOST_OS),linux)
LOCAL_LDFLAGS := -lpthread -lrt
endif

LOCAL_MODULE := heimdall
include $(BUILD_HOST_EXECUTABLE)
