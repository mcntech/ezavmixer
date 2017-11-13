#include <string.h>
#include <jni.h>
#include <android/log.h>
#include "UdpMultiPlayer.h"
#include "jUdpPlayerEvents.h"
#include "JdDbg.h"

static int  modDbgLevel = CJdDbg::LVL_TRACE;

CUdpPlayerEvents	*g_pEventHandler = NULL;
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

extern "C" {


jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("", "JNI_OnLoad "));
	return JNI_VERSION_1_6;
}

jlong Java_com_mcntech_udpplayer_UdpPlayerApi_init(JNIEnv *env, jobject self)
{
	pthread_mutex_lock(&g_mutex);
	CUdpPlayerEvents *pEventHandler;
	CPlayerBase *pPlayer = NULL;

	CUdpPlayerEvents *pEventCallback = new CUdpPlayerEvents(env, self);
	g_pEventHandler  = pEventCallback;

	JDBG_LOG(CJdDbg::LVL_TRACE, ("initializing Onyx Player"));
	pPlayer =  CUdpMultiPlayer::openInstance(pEventCallback);
	pthread_mutex_unlock(&g_mutex);
	return (jlong)pPlayer;
}

jboolean Java_com_mcntech_udpplayer_UdpPlayerApi_deinit(JNIEnv *env, jobject self, jlong ctx)
{
	pthread_mutex_lock(&g_mutex);
	CPlayerBase* pb = (CPlayerBase*)ctx;
	delete pb;
	g_pEventHandler = NULL;
	pthread_mutex_unlock(&g_mutex);
	return true;
}

int Java_com_mcntech_udpplayer_UdpPlayerApi_addServer(JNIEnv *env, jobject self, jlong ctx, jstring jurl)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	pPlayer->addServer(url);
	env->ReleaseStringUTFChars(jurl, szUrl);
}

int Java_com_mcntech_udpplayer_UdpPlayerApi_removeServer(JNIEnv *env, jobject self, jlong ctx, jstring jurl)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	pPlayer->removeServer(url);
	env->ReleaseStringUTFChars(jurl, szUrl);
}

int Java_com_mcntech_udpplayer_UdpPlayerApi_getStatus(JNIEnv *env, jobject self, jlong ctx, jstring jurl, std::string &status)
{

	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;

	env->ReleaseStringUTFChars(jurl, szUrl);
}

int Java_com_mcntech_udpplayer_UdpPlayerApi_startServer(JNIEnv *env, jobject self, jlong ctx, jstring jurl)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	pPlayer->startServer(url);
	env->ReleaseStringUTFChars(jurl, szUrl);
}

int Java_com_mcntech_udpplayer_UdpPlayerApi_stopServer(JNIEnv *env, jobject self, jlong ctx, jstring jurl)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	pPlayer->stopServer(url);
	env->ReleaseStringUTFChars(jurl, szUrl);
}

jint Java_com_mcntech_udpplayer_UdpPlayerApi_getVideoFrame(JNIEnv *env, jobject self, jlong ctx, jstring jurl, jobject buf, jint numBytes, jint nTimeoutMs)
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

jlong Java_com_mcntech_udpplayer_UdpPlayerApi_getVideoPts(JNIEnv *env, jobject self, jlong ctx, jstring jurl)
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

jlong Java_com_mcntech_udpplayer_UdpPlayerApi_getClockUs(JNIEnv *env, jobject self, jlong ctx,jstring jurl)
{
	//pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	jlong clock = 0;
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;


	clock = pPlayer->getClkUs(url);
	env->ReleaseStringUTFChars(jurl, szUrl);
	return clock;
}

jint Java_com_mcntech_udpplayer_UdpPlayerApi_getVidCodecType(JNIEnv *env, jobject self, jlong ctx, jstring jurl)
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

jint Java_com_mcntech_udpplayer_UdpPlayerApi_getAudCodecType(JNIEnv *env, jobject self, jlong ctx, jstring jurl)
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

jint Java_com_mcntech_udpplayer_UdpPlayerApi_getAudioFrame(JNIEnv *env, jobject self, jlong ctx, jstring jurl, jobject buf, jint numBytes, jint nTimeoutMs)
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

jlong Java_com_mcntech_udpplayer_UdpPlayerApi_getAudioPts(JNIEnv *env, jobject self, jlong ctx, jstring jurl)
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

jlong Java_com_mcntech_udpplayer_UdpPlayerApi_getNumAvailVideoFrames(JNIEnv *env, jobject self, jlong ctx, jstring jurl)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;

	//pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	jlong pts = 0;
	pts = pPlayer->getNumAvailVideoFrames(url);

	env->ReleaseStringUTFChars(jurl, szUrl);
	return pts;
}
jlong Java_com_mcntech_udpplayer_UdpPlayerApi_getNumAvailAudioFrames(JNIEnv *env, jobject self, jlong ctx, jstring jurl)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;

	//pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	jlong pts = 0;
	pts = pPlayer->getNumAvailAudioFrames(url);

	env->ReleaseStringUTFChars(jurl, szUrl);
	return pts;
}


}//end extern C
