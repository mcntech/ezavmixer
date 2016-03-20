#include <string.h>
#include <jni.h>
#include <android/log.h>
#include <RtspPlayer.h>
#include "jEventHandler.h"


CRtspMultiPlayer	*g_pRtspPlayer = NULL;
CjOnyxPlayerEvents	*g_pEventHandler = NULL;
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

extern "C" {


jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	MIDSLOG("", "JNI_OnLoad ");
	return JNI_VERSION_1_6;
}

jlong Java_com_mcntech_rtspplayer_DeviceController_init(JNIEnv *env, jobject self)
{
	pthread_mutex_lock(&g_mutex);
	CjOnyxPlayerEvents *pEventHandler;
	CPlayerBase *pPlayer = NULL;

	COnyxPlayerEvents *pEventCallback = new COnyxPlayerEvents(env, self);
	g_pEventHandler  = new CjOnyxPlayerEvents(env,self);

	MIDSLOG("", "initializing Onyx Player");
	pPlayer =  CRtspMultiPlayer::openInstance(pEventCallback);
	pthread_mutex_unlock(&g_mutex);
	return (jlong)g_pRtspPlayer;
}

jboolean Java_com_mcntech_rtspplayer_DeviceController_deinit(JNIEnv *env, jobject self, jlong ctx)
{
	pthread_mutex_lock(&g_mutex);
	CPlayerBase* pb = (CPlayerBase*)ctx;
	pb->exit();
	delete pb;
	g_pRtspPlayer = NULL;
	pthread_mutex_unlock(&g_mutex);
	return true;
}

int Java_com_mcntech_rtspplayer_DeviceController_addServer(JNIEnv *env, jobject self, jlong ctx, jstring jurl)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	pPlayer->addServer(url);
	env->ReleaseStringUTFChars(jurl, szUrl);
}

int Java_com_mcntech_rtspplayer_DeviceController_removeServer(JNIEnv *env, jobject self, jlong ctx, jstring jurl)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	pPlayer->removeServer(url);
	env->ReleaseStringUTFChars(jurl, szUrl);
}

int Java_com_mcntech_rtspplayer_DeviceController_getStatus(JNIEnv *env, jobject self, jlong ctx, jstring jurl, std::string &status)
{

	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;

	env->ReleaseStringUTFChars(jurl, szUrl);
}

jint Java_com_mcntech_rtspplayer_DeviceController_getVideoFrame(JNIEnv *env, jobject self, jlong ctx, jstring jurl, jobject buf, jint numBytes, jint nTimeoutMs)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;

	//pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	int result = 0;

	uint8_t* rawjBytes = static_cast<uint8_t*>(env->GetDirectBufferAddress(buf));
	result =  pPlayer->getVideoData(url, (char *)rawjBytes,numBytes);
	//pthread_mutex_unlock(&g_mutex);
	return result;
}

jlong Java_com_mcntech_rtspplayer_DeviceController_getVideoPts(JNIEnv *env, jobject self, jlong ctx, jstring jurl)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;

	//pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	jlong pts = 0;

	pts = pPlayer->getVideoPts(url);
	env->ReleaseStringUTFChars(jurl, szUrl);
	return pts;
}

jlong Java_com_mcntech_rtspplayer_DeviceController_getClockUs(JNIEnv *env, jobject self, jlong ctx,jstring ur)
{
	//pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	jlong clock = 0;

	clock = pPlayer->getClkUs(url);
	env->ReleaseStringUTFChars(jurl, szUrl);
	return clock;
}

jint Java_com_mcntech_rtspplayer_DeviceController_getVidCodecType(JNIEnv *env, jobject self, jlong ctx, jstring jurl)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;

	//pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	jint nCodecType = 1;

	nCodecType = pPlayer->getVideoCodecType(url);

	env->ReleaseStringUTFChars(jurl, szUrl);
	return nCodecType;
}

jint Java_com_mcntech_rtspplayer_DeviceController_getAudCodecType(JNIEnv *env, jobject self, jlong ctx, jstring jurl)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;

	//pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	jint nCodecType = 1;

	nCodecType = pPlayer->getAudioCodecType(url);

	env->ReleaseStringUTFChars(jurl, szUrl);
	return nCodecType;
}

jint Java_com_mcntech_rtspplayer_DeviceController_getAudioFrame(JNIEnv *env, jobject self, jlong ctx, jstring jurl, jobject buf, jint numBytes, jint nTimeoutMs)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;

	//pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	int result = 0;

	uint8_t* rawjBytes = static_cast<uint8_t*>(env->GetDirectBufferAddress(buf));
	result =  pPlayer->getAudioData(url, (char *)rawjBytes,numBytes);

	env->ReleaseStringUTFChars(jurl, szUrl);
	//pthread_mutex_unlock(&g_mutex);
	return result;
}

jlong Java_com_mcntech_rtspplayer_DeviceController_getAudioPts(JNIEnv *env, jobject self, jlong ctx, jstring jurl)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;

	//pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	jlong pts = 0;
	pts = pPlayer->getAudioPts(url);

	env->ReleaseStringUTFChars(jurl, szUrl);
	return pts;
}
}//end extern C
