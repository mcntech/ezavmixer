LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := jdnet
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/.. $(LOCAL_PATH)/../include
MY_SRCS := $(LOCAL_PATH)/../src

LOCAL_CFLAGS += -DPLATFORM_ANDROID

LOCAL_SRC_FILES := $(MY_SRCS)/JdOsal.cpp
LOCAL_SRC_FILES += $(MY_SRCS)/JdUdp.cpp
LOCAL_SRC_FILES += $(MY_SRCS)/JdRtp.cpp
LOCAL_SRC_FILES += $(MY_SRCS)/JdRtpRcv.cpp
LOCAL_SRC_FILES += $(MY_SRCS)/JdRtpSnd.cpp
LOCAL_SRC_FILES += $(MY_SRCS)/JdRtspClnt.cpp
LOCAL_SRC_FILES += $(MY_SRCS)/JdRtspClntRec.cpp

include $(BUILD_STATIC_LIBRARY)
