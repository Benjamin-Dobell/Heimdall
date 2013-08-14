LOCAL_PATH := $(call my-dir)

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
  HelpAction.cpp \
  Interface.cpp \
  Utility.cpp \
  main.cpp \
  ../../libpit/Source/libpit.cpp
  
LOCAL_C_INCLUDES := \
  external/heimdall/libpit/Source \
  external/libusbx/libusb

LOCAL_STATIC_LIBRARIES := libusbx

ifeq ($(HOST_OS),darwin)
LOCAL_LDFLAGS := -framework CoreFoundation -framework IOKit
endif

ifeq ($(HOST_OS),linux)
LOCAL_LDFLAGS := -lpthread -lrt
endif

LOCAL_MODULE := heimdall
include $(BUILD_HOST_EXECUTABLE)

$(call dist-for-goals,dist_files sdk,$(LOCAL_BUILT_MODULE))
