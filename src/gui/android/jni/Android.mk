APP_LOCAL_PATH := $(call my-dir)

include ../../core/jni/Android.mk

LOCAL_PATH := $(APP_LOCAL_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE := onyxjni
LOCAL_SRC_FILES := onyxjni.cpp
LOCAL_SHARED_LIBRARIES := liblog libcutils
LOCAL_STATIC_LIBRARIES := core
include $(BUILD_SHARED_LIBRARY)
 
