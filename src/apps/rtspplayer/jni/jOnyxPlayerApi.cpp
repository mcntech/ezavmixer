#include <string.h>
#include <jni.h>
#include <android/log.h>
#include <RtspPlayer.h>
#include "jEventHandler.h"


CRtspPlayer	*g_pRtspPlayer = NULL;
CjOnyxPlayerEvents	*g_pEventHandler = NULL;
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

extern "C" {


jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	MIDSLOG("", "JNI_OnLoad ");
	return JNI_VERSION_1_6;
}

//////////////////////////Basic set of routines for initializing and playing///////////////////
jlong Java_com_mcntech_rtspplayer_DeviceController_init(JNIEnv *env, jobject self,jint deviceIp, jboolean retry, jint role, jint framesPerPeriod, jint numPeriods, int audDelayUs)
{
	pthread_mutex_lock(&g_mutex);
	CjOnyxPlayerEvents *pEventHandler;
	CPlayerBase *pPlayer = NULL;

	COnyxPlayerEvents *pEventCallback = new COnyxPlayerEvents(env, self);
	g_pEventHandler  = new CjOnyxPlayerEvents(env,self);

	MIDSLOG("", "initializing Onyx Player");
	pPlayer =  CRtspPlayer::openInstance(pEventCallback);
	if(pPlayer->initialize((int)deviceIp,retry,role, framesPerPeriod, numPeriods, audDelayUs))
		pPlayer->start();
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

jboolean Java_com_mcntech_rtspplayer_DeviceController_restart(JNIEnv *env, jobject self, jlong ctx, jint ip)
{
	jboolean status;
	pthread_mutex_lock(&g_mutex);
	CPlayerBase* pb = (CPlayerBase*)ctx;
	status = pb->restart(ip);
	pthread_mutex_unlock(&g_mutex);
	return status;
}


///////////////////////////////Misc state information////////////////////////////////////

jboolean Java_com_mcntech_rtspplayer_DeviceController_isPlaying(JNIEnv *env, jobject self, jlong ctx)
{
	pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	jboolean result = pPlayer->isPlaying();
	pthread_mutex_unlock(&g_mutex);
	return result;
}

jstring Java_com_mcntech_rtspplayer_DeviceController_getVersion(JNIEnv *env, jobject self, jlong ctx)
{
	pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	jstring result = env->NewStringUTF(pPlayer->getVersionString());
	pthread_mutex_unlock(&g_mutex);
	return result;
}


jint Java_com_mcntech_rtspplayer_DeviceController_getNumAvailVideoFrames(JNIEnv *env, jobject self, jlong ctx)
{
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	int result = 0;
	result =  pPlayer->GetNumAvailVideoFrames();
	return result;
}

jint Java_com_mcntech_rtspplayer_DeviceController_getVideoFrame(JNIEnv *env, jobject self, jlong ctx, jobject buf, jint numBytes, jint nTimeoutMs)
{
	//pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	int result = 0;

	uint8_t* rawjBytes = static_cast<uint8_t*>(env->GetDirectBufferAddress(buf));
	result =  pPlayer->GetVideoFrame((char *)rawjBytes,numBytes, &lPts, nTimeoutMs);
	//pthread_mutex_unlock(&g_mutex);
	return result;
}

jlong Java_com_mcntech_rtspplayer_DeviceController_getVideoPts(JNIEnv *env, jobject self, jlong ctx)
{
	//pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	jlong pts = 0;

	pts = pPlayer->getVideoPts();
	return pts;
}

jlong Java_com_mcntech_rtspplayer_DeviceController_getClockUs(JNIEnv *env, jobject self, jlong ctx)
{
	//pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	jlong clock = 0;

	clock = pPlayer->getClkUs();
	return clock;
}

jint Java_com_mcntech_rtspplayer_DeviceController_getVidCodecType(JNIEnv *env, jobject self, jlong ctx)
{
	//pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	jint nVidCodecType = 1;

	nVidCodecType = pPlayer->getVidCodecType();
	return nVidCodecType;
}

jint Java_com_mcntech_rtspplayer_DeviceController_getAudioFrame(JNIEnv *env, jobject self, jlong ctx, jobject buf, jint numBytes, jint nTimeoutMs)
{
	//pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	int result = 0;

	uint8_t* rawjBytes = static_cast<uint8_t*>(env->GetDirectBufferAddress(buf));
	result =  pPlayer->GetAudioFrame((char *)rawjBytes,numBytes, &lPts, nTimeoutMs);
	//pthread_mutex_unlock(&g_mutex);
	return result;
}

jlong Java_com_mcntech_rtspplayer_DeviceController_getAudioPts(JNIEnv *env, jobject self, jlong ctx)
{
	//pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	jlong pts = 0;
	pts = pPlayer->getAudioPts();
	return pts;
}
}//end extern C
