LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := onyxcore
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)
OSAL_FOLDER := $(LOCAL_PATH)/../../osal
JD_PLUGIN_FOLDER := $(LOCAL_PATH)/../../plugins

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/..
LOCAL_C_INCLUDES += $(OSAL_FOLDER)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../codecdm81xx/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../logutil/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/jdrtsp/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/tsmux/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/misc/include

LOCAL_CFLAGS += -DPLATFORM_ANDROID

LOCAL_SRC_FILES := ../src/MediaSwitch.cpp
LOCAL_SRC_FILES += ../src/RtspPublishBridge.cpp
LOCAL_SRC_FILES += ../src/StrmInBridgeBase.cpp
LOCAL_SRC_FILES += ../src/StrmOutBridgeBase.cpp
LOCAL_SRC_FILES += ../src/RtspConfigure.cpp

include $(BUILD_STATIC_LIBRARY)
