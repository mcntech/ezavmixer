#include <string.h>
#include <jni.h>
#include <android/log.h>
//#include "jEventHandler.h"
#include "RtspMultiPublishClnt.h"
#include "DashMultiPublishClnt.h"
#include "PublishClntBase.h"
#include "jOnyxEvents.h"
#define DBGLOG

pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
CPublishClntBase *g_pMultiPublish;

extern "C" {

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	DBGLOG("", "JNI_OnLoad ");
	return JNI_VERSION_1_6;
}

jlong Java_com_mcntech_ezscreencast_OnyxApi_init(JNIEnv *env, jobject self,jint protocol)
{
	pthread_mutex_lock(&g_mutex);
	if(g_pMultiPublish == NULL) {
		COnyxEvents *pEventCallback = new COnyxEvents(env, self);
		if(protocol == 0)
			g_pMultiPublish =  CRtspMultiPublishClnt::openInstance(pEventCallback);
		if(protocol == 1)
			g_pMultiPublish =  CDashMultiPublishClnt::openInstance(pEventCallback);
	}
	DBGLOG("", "initializing screencat");
	pthread_mutex_unlock(&g_mutex);
	return (jlong)g_pMultiPublish;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_deinit(JNIEnv *env, jobject self, jlong publisher)
{
	DBGLOG("", "MediaController_deinit:Start");
	pthread_mutex_lock(&g_mutex);
	CPublishClntBase* _publisher = (CPublishClntBase*)publisher;
	//_publisher->closeInstancce((CPublishClntBase *)g_pMultiPublish);
	delete g_pMultiPublish;
	g_pMultiPublish = NULL;
	pthread_mutex_unlock(&g_mutex);
	DBGLOG("", "MediaController_deinit:End");
	return true;
}

// play from start of track every time
jboolean Java_com_mcntech_ezscreencast_OnyxApi_start(JNIEnv *env, jobject self, jlong publisher, jstring path, jstring title, jstring artist)
{
	//todo if(!CPublishClntBase::safeLock(&g_mutex))//pthread_mutex_lock(&g_mutex);
	//	return false;

	CPublishClntBase* _publisher = (CPublishClntBase*)publisher;
	_publisher->stop();
	DBGLOG("", "jni playfile: waiting for session to end");
	jboolean result = true;
	_publisher->start();

	//pthread_mutex_unlock(&g_mutex);
	return result;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_stop(JNIEnv *env, jobject self, jlong publisher)
{
	//TODO if(!CPublishClntBase::safeLock(&g_mutex))
	//	return false;
	CPublishClntBase* _publisher = (CPublishClntBase*)publisher;
	if(_publisher != NULL)
		_publisher->stop();
	DBGLOG("", "jni stopPlaying: stopped playback1");
	return true;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_pause(JNIEnv *env, jobject self, jlong publisher)
{
	return true;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_resume(JNIEnv *env, jobject self, jlong publisher, jstring path, jstring title, jstring artist)
{
	return true;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_addRemoteNode(JNIEnv *env, jobject self, jlong handle, jstring url, jstring jappName)
{
	int result = 0;
	CPublishClntBase* _publisher = (CPublishClntBase*)handle;
	const char * szUrl = env->GetStringUTFChars(url, 0);
	std::string tmpUrl = szUrl;
	const char *szAppName = env->GetStringUTFChars(jappName, 0);
	std::string tmpAppName = szAppName;
	_publisher->AddPublishServer(tmpUrl, tmpAppName);
	env->ReleaseStringUTFChars(url, szUrl);
	env->ReleaseStringUTFChars(jappName, szAppName);
	return JNI_TRUE;
}

jint Java_com_mcntech_ezscreencast_OnyxApi_sendAudioData(JNIEnv *env, jobject self, jlong publisher,jbyteArray pcmBytes,jint numBytes, long Pts, int Flags)
{
	int result = 0;
	//pthread_mutex_lock(&g_mutex);
	CPublishClntBase* _publisher = (CPublishClntBase*)publisher;
	
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
	CPublishClntBase* _publisher = (CPublishClntBase*)publisher;
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
	CPublishClntBase* _publisher = (CPublishClntBase*)publisher;
	bool result =  JNI_FALSE;
	//TODO
	pthread_mutex_unlock(&g_mutex);
	return result;
}


jboolean Java_com_mcntech_ezscreencast_OnyxApi_endSession(JNIEnv *env, jobject self, jlong publisher)
{
	DBGLOG("", "MediaController_endSession:Begin");
	pthread_mutex_lock(&g_mutex);
	CPublishClntBase* _publisher = (CPublishClntBase*)publisher;
	//TODO
	pthread_mutex_unlock(&g_mutex);
	return JNI_TRUE;
}

jstring Java_com_mcntech_ezscreencast_OnyxApi_getVersion(JNIEnv *env, jobject self, jlong publisher)
{
	pthread_mutex_lock(&g_mutex);
	CPublishClntBase* _publisher = (CPublishClntBase*)publisher;
	jstring result =  env->NewStringUTF("Test 1.0"/*TODO_publisher->getVersionString()*/);
	pthread_mutex_unlock(&g_mutex);
	return result;
}

jlong Java_com_mcntech_ezscreencast_OnyxApi_getClockUs(JNIEnv *env, jobject self, jlong publisher)
{
	//pthread_mutex_lock(&g_mutex);
	CPublishClntBase* _publisher = (CPublishClntBase*)publisher;
	jlong clock = 0;
	uint64_t freq;
	//TODO
	return clock;
}
} // extern "C"
