LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := tinyxml
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/.. 
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../logutil/include
MY_SRCS := ../src

LOCAL_CFLAGS += -DPLATFORM_ANDROID

LOCAL_SRC_FILES := $(MY_SRCS)/tinystr.cpp
LOCAL_SRC_FILES += $(MY_SRCS)/tinyxml.cpp
LOCAL_SRC_FILES := $(MY_SRCS)/tinyxmlerror.cpp
LOCAL_SRC_FILES := $(MY_SRCS)/tinyxmlparser.cpp

include $(BUILD_STATIC_LIBRARY)
