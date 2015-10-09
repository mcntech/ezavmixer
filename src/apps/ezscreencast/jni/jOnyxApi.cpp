#include <string.h>
#include <jni.h>
#include <android/log.h>
//#include "jEventHandler.h"
#include "MultiPublishClnt.h"

#define DBGLOG

pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
CMultiPublish *g_pMultiPublish;

extern "C" {

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	DBGLOG("", "JNI_OnLoad ");
	return JNI_VERSION_1_6;
}

jlong Java_com_mcntech_ezscreencast_OnyxApi_init(JNIEnv *env, jobject self,jint deviceIp)
{
	pthread_mutex_lock(&g_mutex);
	if(g_pMultiPublish == NULL)
		g_pMultiPublish =  CMultiPublish::getInstance();
	DBGLOG("", "initializing screencat");
	pthread_mutex_unlock(&g_mutex);
	return (jlong)g_pMultiPublish;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_deinit(JNIEnv *env, jobject self, jlong publisher)
{
	DBGLOG("", "MediaController_deinit:Start");
	pthread_mutex_lock(&g_mutex);

	CMultiPublish::closeInstancce(g_pMultiPublish);
	g_pMultiPublish = NULL;
	pthread_mutex_unlock(&g_mutex);
	DBGLOG("", "MediaController_deinit:End");
	return true;
}


jboolean Java_com_mcntech_ezscreencast_OnyxApi_stop(JNIEnv *env, jobject self, jlong publisher)
{
	//TODO if(!CMultiPublish::safeLock(&g_mutex))
	//	return false;
	CMultiPublish* _publisher = (CMultiPublish*)publisher;
	if(_publisher != NULL)
		_publisher->stop();
	DBGLOG("", "jni stopPlaying: stopped playback1");
	return true;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_pause(JNIEnv *env, jobject self, jlong publisher)
{
	return true;
}
// play from start of track every time
jboolean Java_com_mcntech_ezscreencast_OnyxApi_start(JNIEnv *env, jobject self, jlong publisher, jstring path, jstring title, jstring artist)
{
	//todo if(!CMultiPublish::safeLock(&g_mutex))//pthread_mutex_lock(&g_mutex);
	//	return false;

	CMultiPublish* _publisher = (CMultiPublish*)publisher;
	_publisher->stop();
	DBGLOG("", "jni playfile: waiting for session to end");
	jboolean result = true;
	_publisher->start();

	//pthread_mutex_unlock(&g_mutex);
	return result;
}

//resumes playback if same track selected
jboolean Java_com_mcntech_ezscreencast_OnyxApi_resume(JNIEnv *env, jobject self, jlong publisher, jstring path, jstring title, jstring artist)
{
	return true;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_isDeviceActive(JNIEnv *env, jobject self, jlong publisher, jlong deviceid)
{
	jboolean isDeviceActive = JNI_FALSE;
	// TODO
	return isDeviceActive;
}

jint Java_com_mcntech_ezscreencast_OnyxApi_sendAudioData(JNIEnv *env, jobject self, jlong publisher,jbyteArray pcmBytes,jint numBytes, long Pts, int Flags)
{
	int result = 0;
	//pthread_mutex_lock(&g_mutex);
	CMultiPublish* _publisher = (CMultiPublish*)publisher;
	
	jboolean isCopy;
	int len = env->GetArrayLength (pcmBytes);
	jbyte* rawjBytes = env->GetByteArrayElements(pcmBytes, &isCopy);
	result = _publisher->sendAudioData((const char *)rawjBytes, len, Pts, Flags);
	env->ReleaseByteArrayElements(pcmBytes,rawjBytes,0);
	//pthread_mutex_unlock(&g_mutex);
	return result;
}

jint Java_com_mcntech_ezscreencast_OnyxApi_sendVideoData(JNIEnv *env, jobject self, jlong publisher,jbyteArray pcmBytes,jint numBytes, long Pts, int Flags)
{
	int result = 0;
	//pthread_mutex_lock(&g_mutex);
	CMultiPublish* _publisher = (CMultiPublish*)publisher;
	jboolean isCopy;
	int len = env->GetArrayLength (pcmBytes);
	jbyte* rawjBytes = env->GetByteArrayElements(pcmBytes, &isCopy);
	result = _publisher->sendVideoData((const char *)rawjBytes, len, Pts, Flags);
	env->ReleaseByteArrayElements(pcmBytes,rawjBytes,0);
	return result;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_startSession(JNIEnv *env, jobject self, jlong publisher, jboolean fEnableAudio, jboolean fEnableVideo)
{
	pthread_mutex_lock(&g_mutex);
	CMultiPublish* _publisher = (CMultiPublish*)publisher;
	bool result =  JNI_FALSE;
	//TODO
	pthread_mutex_unlock(&g_mutex);
	return result;
}


jboolean Java_com_mcntech_ezscreencast_OnyxApi_endSession(JNIEnv *env, jobject self, jlong publisher)
{
	DBGLOG("", "MediaController_endSession:Begin");
	pthread_mutex_lock(&g_mutex);
	CMultiPublish* _publisher = (CMultiPublish*)publisher;
	//TODO
	pthread_mutex_unlock(&g_mutex);
	return JNI_TRUE;
}


///////////////////////////////Misc state information////////////////////////////////////

jboolean Java_com_mcntech_ezscreencast_OnyxApi_isPlaying(JNIEnv *env, jobject self, jlong publisher)
{
	pthread_mutex_lock(&g_mutex);
	CMultiPublish* _publisher = (CMultiPublish*)publisher;
	jboolean result = JNI_TRUE; //TODO _publisher->isPlaying();
	pthread_mutex_unlock(&g_mutex);
	return result;
}

jstring Java_com_mcntech_ezscreencast_OnyxApi_getVersion(JNIEnv *env, jobject self, jlong publisher)
{
	pthread_mutex_lock(&g_mutex);
	CMultiPublish* _publisher = (CMultiPublish*)publisher;
	jstring result =  env->NewStringUTF("Test 1.0"/*TODO_publisher->getVersionString()*/);
	pthread_mutex_unlock(&g_mutex);
	return result;
}

jlong Java_com_mcntech_ezscreencast_OnyxApi_getClockUs(JNIEnv *env, jobject self, jlong publisher)
{
	//pthread_mutex_lock(&g_mutex);
	CMultiPublish* _publisher = (CMultiPublish*)publisher;
	jlong clock = 0;
	uint64_t freq;
	//TODO
	return clock;
}
} // extern "C"
