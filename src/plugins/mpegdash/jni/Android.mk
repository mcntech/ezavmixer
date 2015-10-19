LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := mpegdash
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/.. 
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../include 
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../tinyxml/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../osal/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../jdhttp/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../httplive/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../misc/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../mp4mux/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../tsdemux/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../tsmux/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../jdaws/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../logutil/include

MY_SRCS := ../src

LOCAL_CFLAGS += -DPLATFORM_ANDROID

LOCAL_SRC_FILES := $(MY_SRCS)/Mpd.cpp
LOCAL_SRC_FILES += $(MY_SRCS)/MpegDashSgmt.cpp

include $(BUILD_STATIC_LIBRARY)
