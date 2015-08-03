LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := onyxcore
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/.. $(LOCAL_PATH)/../tsdemux $(LOCAL_PATH)/../tsmux
LOCAL_CFLAGS += -DPLATFORM_ANDROID

LOCAL_SRC_FILES := ../onyxcore.cpp ../udprdr.c ../UdpSrvBridge.cpp

include $(BUILD_STATIC_LIBRARY)
