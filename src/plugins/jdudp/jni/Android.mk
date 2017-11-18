LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS += -DPLATFORM_ANDROID

LOCAL_MODULE := jdudp
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/.. 
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../logutil/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../streamif/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../osal/include
MY_SRCS := ../src

LOCAL_SRC_FILES := $(MY_SRCS)/filesrc.cpp
LOCAL_SRC_FILES += $(MY_SRCS)/udprx.cpp
LOCAL_SRC_FILES += $(MY_SRCS)/xport.cpp
LOCAL_SRC_FILES += $(MY_SRCS)/strmclock.cpp

include $(BUILD_STATIC_LIBRARY)
