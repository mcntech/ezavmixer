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

jlong Java_com_mcntech_udpplayer_UdpPlayerApi_addServer(JNIEnv *env, jobject self, jlong ctx, jstring jurl)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	pPlayer->addServer(url);
	env->ReleaseStringUTFChars(jurl, szUrl);
    return 0;
}

jlong Java_com_mcntech_udpplayer_UdpPlayerApi_removeServer(JNIEnv *env, jobject self, jlong ctx, jstring jurl)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	pPlayer->removeServer(url);
	env->ReleaseStringUTFChars(jurl, szUrl);
    return 0;
}

jint Java_com_mcntech_udpplayer_UdpPlayerApi_getStatus(JNIEnv *env, jobject self, jlong ctx, jstring jurl, std::string &status)
{

	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;

	env->ReleaseStringUTFChars(jurl, szUrl);
    return 0;
}

jint Java_com_mcntech_udpplayer_UdpPlayerApi_startServer(JNIEnv *env, jobject self, jlong ctx, jstring jurl)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	pPlayer->startServer(url);
	env->ReleaseStringUTFChars(jurl, szUrl);
    return 0;
}

jint Java_com_mcntech_udpplayer_UdpPlayerApi_stopServer(JNIEnv *env, jobject self, jlong ctx, jstring jurl)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	pPlayer->stopServer(url);
	env->ReleaseStringUTFChars(jurl, szUrl);
    return 0;
}

jint Java_com_mcntech_udpplayer_UdpPlayerApi_subscribeStream(JNIEnv *env, jobject self, jlong ctx, jstring jurl, jint strmid)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;

	//pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	int result = 0;

	result =  pPlayer->subscribeStream(url, strmid);
    env->ReleaseStringUTFChars(jurl, szUrl);
	//pthread_mutex_unlock(&g_mutex);
	return result;
}

jint Java_com_mcntech_udpplayer_UdpPlayerApi_unsubscribeStream(JNIEnv *env, jobject self, jlong ctx, jstring jurl, jint strmid)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;

	//pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	int result = 0;

	result =  pPlayer->unsubscribeStream(url, strmid);
    env->ReleaseStringUTFChars(jurl, szUrl);
	//pthread_mutex_unlock(&g_mutex);
	return result;
}

jint Java_com_mcntech_udpplayer_UdpPlayerApi_subscribeProgram(JNIEnv *env, jobject self, jlong ctx, jstring jurl, jint strmid)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;

	//pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	int result = 0;

	result =  pPlayer->subscribeProgram(url, strmid);
	env->ReleaseStringUTFChars(jurl, szUrl);
	//pthread_mutex_unlock(&g_mutex);
	return result;
}

jint Java_com_mcntech_udpplayer_UdpPlayerApi_unsubscribeProgram(JNIEnv *env, jobject self, jlong ctx, jstring jurl, jint strmid)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;

	//pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	int result = 0;

	result =  pPlayer->unsubscribeProgram(url, strmid);
	env->ReleaseStringUTFChars(jurl, szUrl);
	//pthread_mutex_unlock(&g_mutex);
	return result;
}

jint Java_com_mcntech_udpplayer_UdpPlayerApi_getFrame(JNIEnv *env, jobject self, jlong ctx, jstring jurl, jint strmid, jobject buf, jint numBytes, jint nTimeoutMs)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;

	//pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	int result = 0;

	uint8_t* rawjBytes = static_cast<uint8_t*>(env->GetDirectBufferAddress(buf));
	result =  pPlayer->getData(url, strmid, (char *)rawjBytes,numBytes);
    env->ReleaseStringUTFChars(jurl, szUrl);
	//pthread_mutex_unlock(&g_mutex);
	return result;
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

jint Java_com_mcntech_udpplayer_UdpPlayerApi_getCodecType(JNIEnv *env, jobject self, jlong ctx, jstring jurl, jint strmid)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;

	//pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	jint nCodecType = 1;

	nCodecType = pPlayer->getCodecType(url, strmid);

	env->ReleaseStringUTFChars(jurl, szUrl);
	return nCodecType;
}


jlong Java_com_mcntech_udpplayer_UdpPlayerApi_getPts(JNIEnv *env, jobject self, jlong ctx, jstring jurl, jint strmid)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;

	//pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	jlong pts = 0;
	pts = pPlayer->getPts(url, strmid);

	env->ReleaseStringUTFChars(jurl, szUrl);
	return pts;
}

jlong Java_com_mcntech_udpplayer_UdpPlayerApi_getNumAvailFrames(JNIEnv *env, jobject self, jlong ctx, jstring jurl, jint strmid)
{
	const char *szUrl = env->GetStringUTFChars(jurl, 0);
	std::string url = szUrl;

	//pthread_mutex_lock(&g_mutex);
	CPlayerBase* pPlayer = (CPlayerBase*)ctx;
	jlong pts = 0;
	pts = pPlayer->getNumAvailFrames(url, strmid);

	env->ReleaseStringUTFChars(jurl, szUrl);
	return pts;
}


}//end extern C
