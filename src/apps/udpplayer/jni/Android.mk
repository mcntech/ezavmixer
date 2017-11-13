APP_LOCAL_PATH := $(call my-dir)

SRC_FOLDER := ../..
include $(SRC_FOLDER)/plugins/jdudp/jni/Android.mk
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
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/jdudp/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/jdnet/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/parseutil/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/tsdemux/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/tinyxml/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/osal/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/configdb/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/osal/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/streamif/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../core/include

LOCAL_CFLAGS += -DPLATFORM_ANDROID
LOCAL_MODULE := OnyxPlayerApi

LOCAL_SRC_FILES := 	UdpMultiPlayer.cpp
LOCAL_SRC_FILES +=	ServerNodeBase.cpp
LOCAL_SRC_FILES +=	ServerNodeUdp.cpp

LOCAL_SRC_FILES +=	jUdpPlayerApi.cpp 
LOCAL_SRC_FILES +=	jUdpPlayerEvents.cpp

LOCAL_SHARED_LIBRARIES := liblog libcutils
LOCAL_LDLIBS := -llog
LOCAL_STATIC_LIBRARIES := onyxcore jdudp jdnet parseutil streamif tinyxml configdb jdosal
include $(BUILD_SHARED_LIBRARY)
