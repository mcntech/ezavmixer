APP_LOCAL_PATH := $(call my-dir)

SRC_FOLDER := ../..
include $(SRC_FOLDER)/plugins/jdrtsp/jni/Android.mk
include $(SRC_FOLDER)/plugins/jdnet/jni/Android.mk
include $(SRC_FOLDER)/plugins/parseutil/jni/Android.mk
include $(SRC_FOLDER)/plugins/osal/jni/Android.mk
include $(SRC_FOLDER)/core/jni/Android.mk
include $(SRC_FOLDER)/plugins/streamif/jni/Android.mk
include $(SRC_FOLDER)/plugins/onvifdiscvr/jni/Android.mk
include $(SRC_FOLDER)/plugins/tinyxml/jni/Android.mk
include $(SRC_FOLDER)/plugins/configdb/jni/Android.mk



include $(CLEAR_VARS)

LOCAL_PATH := $(APP_LOCAL_PATH)
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)
JD_PLUGIN_FOLDER := $(LOCAL_PATH)/../../../plugins

LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/.. $(LOCAL_PATH)/../include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../logutil/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/jdrtsp/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/jdnet/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/parseutil/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/tsdemux/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/tinyxml/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/osal/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/configdb/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/osal/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/streamif/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/onvifdiscvr/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../core/include

LOCAL_CFLAGS += -DPLATFORM_ANDROID
LOCAL_MODULE := OnyxPlayerApi

LOCAL_SRC_FILES := 	RtspMultiPlayer.cpp
LOCAL_SRC_FILES +=	ServerNode.cpp

LOCAL_SRC_FILES +=	jOnyxPlayerApi.cpp 
LOCAL_SRC_FILES +=	jOnyxPlayerEvents.cpp

LOCAL_SHARED_LIBRARIES := liblog libcutils
LOCAL_LDLIBS := -llog
LOCAL_STATIC_LIBRARIES := onyxcore jdrtsp jdnet parseutil streamif tinyxml configdb jdosal onvifdiscvr
include $(BUILD_SHARED_LIBRARY)
