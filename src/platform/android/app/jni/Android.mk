APP_LOCAL_PATH := $(call my-dir)
include /opt/mcnt/ezavmixer/src/onyxcore/jni/Android.mk
LOCAL_PATH := $(APP_LOCAL_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE := onyxjni
LOCAL_SRC_FILES := onyxjni.cpp
LOCAL_SHARED_LIBRARIES := liblog libcutils
LOCAL_STATIC_LIBRARIES := onyxcore
include $(BUILD_SHARED_LIBRARY)
 
