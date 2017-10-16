LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := onvifdiscvr
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)

OSAL_FOLDER := $(LOCAL_PATH)/../../osal

LOCAL_C_INCLUDES := $(LOCAL_PATH)/.. 
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../include
LOCAL_C_INCLUDES += $(OSAL_FOLDER)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../logutil/include
MY_SRCS := ../src

LOCAL_CFLAGS += -DPLATFORM_ANDROID

LOCAL_SRC_FILES := $(MY_SRCS)/Onvif.cpp

include $(BUILD_STATIC_LIBRARY)
