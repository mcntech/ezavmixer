LOCAL_PATH := $(call my-dir)

SRC_FOLDER := ../..
include $(SRC_FOLDER)/plugins/jdrtsp/jni/Android.mk
include $(SRC_FOLDER)/plugins/mpegdash/jni/Android.mk
include $(SRC_FOLDER)/plugins/jdhttp/jni/Android.mk
include $(SRC_FOLDER)/plugins/jdnet/jni/Android.mk
include $(SRC_FOLDER)/plugins/tsmux/jni/Android.mk
include $(SRC_FOLDER)/plugins/mp4mux/jni/Android.mk
include $(SRC_FOLDER)/plugins/parseutil/jni/Android.mk
include $(SRC_FOLDER)/plugins/osal/jni/Android.mk
include $(SRC_FOLDER)/core/jni/Android.mk
include $(SRC_FOLDER)/plugins/streamif/jni/Android.mk
include $(SRC_FOLDER)/plugins/tinyxml/jni/Android.mk
include $(SRC_FOLDER)/plugins/configdb/jni/Android.mk
include $(SRC_FOLDER)/plugins/jdaws/jni/Android.mk
include $(SRC_FOLDER)/extlibs/openssl-1.0.2/Android.mk


include $(CLEAR_VARS)

LOCAL_PATH := $(APP_LOCAL_PATH)
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)
JD_PLUGIN_FOLDER := $(LOCAL_PATH)/../../../plugins

LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/.. $(LOCAL_PATH)/../include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../logutil/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/jdrtsp/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/jdhttp/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/httplive/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/jdnet/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/parseutil/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/tsdemux/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/tinyxml/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/mp4mux/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/mpegdash/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/osal/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/configdb/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/osal/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/streamif/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/tsmux/include
LOCAL_C_INCLUDES += $(JD_PLUGIN_FOLDER)/jdaws/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../core/include

LOCAL_CFLAGS += -DPLATFORM_ANDROID
LOCAL_MODULE := OnyxPlayerApi

LOCAL_SRC_FILES := 	RtspPlayer.cpp

LOCAL_SRC_FILES +=	jOnyxPlayerApi.cpp 
LOCAL_SRC_FILES +=	jOnyxPlayerEvents.cpp

LOCAL_SHARED_LIBRARIES := liblog libcutils
LOCAL_LDLIBS := -llog
LOCAL_STATIC_LIBRARIES := onyxcore jdrtsp  mpegdash jdaws opencrypto_static jdhttp jdnet tsmux mp4mux parseutil streamif tinyxml configdb jdosal
include $(BUILD_SHARED_LIBRARY)