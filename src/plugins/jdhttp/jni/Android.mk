LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := jdhttp
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/.. 
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../osal/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../jdnet/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../logutil/include

MY_SRCS := ../src

LOCAL_CFLAGS += -DPLATFORM_ANDROID

LOCAL_SRC_FILES := $(MY_SRCS)/JdHttpClnt.cpp
LOCAL_SRC_FILES += $(MY_SRCS)/JdHttpSrv.cpp


include $(BUILD_STATIC_LIBRARY)
