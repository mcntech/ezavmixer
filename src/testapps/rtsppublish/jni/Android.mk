APP_LOCAL_PATH := $(call my-dir)

include ../../plugins/jdrtsp/jni/Android.mk
include ../../plugins/osal/jni/Android.mk

include $(CLEAR_VARS)
LOCAL_PATH := $(APP_LOCAL_PATH)

LOCAL_MODULE := TestRtspPublish
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)
JD_PLUGIN_FOLDER := $(LOCAL_PATH)/../../../plugins

LOCAL_C_INCLUDES := $(LOCAL_PATH)/.. $(LOCAL_PATH)/../include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../logutil/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/jdrtsp/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/osal/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/configdb/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/osal/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/streamif/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/tsmux/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../core/include

LOCAL_CFLAGS += -DPLATFORM_ANDROID

LOCAL_SRC_FILES := TestRtspPublish.cpp
LOCAL_SHARED_LIBRARIES := liblog libcutils
LOCAL_STATIC_LIBRARIES := jdrtsp jdosal
include $(BUILD_SHARED_LIBRARY)
