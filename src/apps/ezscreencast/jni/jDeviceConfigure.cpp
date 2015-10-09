#include <string.h>
#include <jni.h>
#include <android/log.h>
#include "jEventHandler.h"

extern pthread_mutex_t g_mutex;
extern jclass g_deviceClass;

extern "C" {


jboolean Java_com_mcntech_ezscreencast_setDeviceSettings(JNIEnv *env, jobject self, jlong onyxapi, jobject node){

	COnyxApi* pOnyx = (COnyxApi*)onyxapi;
	if(pOnyx == NULL)
		return JNI_FALSE;
	if(g_deviceClass == NULL)
		return JNI_FALSE;
	jclass destClass = g_deviceClass;
	jfieldID keyField = env->GetFieldID(destClass, "key","Ljava/lang/String;");
	jfieldID portld = env->GetFieldID(destClass, "port","I");

	int nPort = env->GetIntField(node, portld);
	char *key =  (char *)env->GetStringUTFChars(jkey, NULL);
	
	return jstatus;
}

}//end extern C
