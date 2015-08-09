LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := mpegdash
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/.. $(LOCAL_PATH)/../include $(LOCAL_PATH)/../../tinyxml/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../osal/include
MY_SRCS := $(LOCAL_PATH)/../src

LOCAL_CFLAGS += -DPLATFORM_ANDROID

LOCAL_SRC_FILES := $(MY_SRCS)/Mpd.cpp
LOCAL_SRC_FILES += $(MY_SRCS)/MpegDashSgmt.cpp

include $(BUILD_STATIC_LIBRARY)
