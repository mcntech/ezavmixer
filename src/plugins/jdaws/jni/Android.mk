LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := jdaws
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)
JD_PLUGIN_FOLDER := $(LOCAL_PATH)/../../../plugins

LOCAL_C_INCLUDES := $(LOCAL_PATH)/.. 
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../logutil/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../extlibs/openssl-1.0.2/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/streamif/include

MY_SRCS := ../src

LOCAL_CFLAGS += -DPLATFORM_ANDROID

LOCAL_SRC_FILES := $(MY_SRCS)/JdAwsConfig.cpp
LOCAL_SRC_FILES += $(MY_SRCS)/JdAwsContext.cpp
LOCAL_SRC_FILES += $(MY_SRCS)/SegmentWriteS3.cpp
LOCAL_SRC_FILES += $(MY_SRCS)/JdAwsRest.cpp
LOCAL_SRC_FILES += $(MY_SRCS)/awsv4.cpp

include $(BUILD_STATIC_LIBRARY)
