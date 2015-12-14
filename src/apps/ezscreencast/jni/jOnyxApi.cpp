#include <string.h>
#include <jni.h>
#include <android/log.h>
#include <InprocStrmConn.h>
//#include "jEventHandler.h"
#include "RtspMultiPublishClnt.h"
#include "DashMultiPublishClnt.h"
#include "PublishClntBase.h"
#include "jOnyxEvents.h"
#include "JdDbg.h"

#define PUBLISH_TYPE_NONE  0
#define PUBLISH_TYPE_RTSP  1
#define PUBLISH_TYPE_MPD   2
#define PUBLISH_TYPE_HLS   3
#define PUBLISH_TYPE_RTMP  4

pthread_mutex_t   g_mutex = PTHREAD_MUTEX_INITIALIZER;

extern "C" {

static int  modDbgLevel = CJdDbg::LVL_ERR;
jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("OnyxAPI"));
	return JNI_VERSION_1_6;
}

jlong Java_com_mcntech_ezscreencast_OnyxApi_init(JNIEnv *env, jobject self,jint protocol)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	pthread_mutex_lock(&g_mutex);
	CPublishClntBase *pMultiPublish = NULL;
	/* Initialize publish client */
	COnyxEvents *pEventCallback = new COnyxEvents(env, self);
	if(protocol == PUBLISH_TYPE_RTSP)
		pMultiPublish =  CRtspMultiPublishClnt::openInstance(pEventCallback);
	if(protocol == PUBLISH_TYPE_MPD)
		pMultiPublish =  CDashMultiPublishClnt::openInstance(pEventCallback);

	pthread_mutex_unlock(&g_mutex);
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return (jlong)pMultiPublish;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_deinit(JNIEnv *env, jobject self, jlong publisher)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	pthread_mutex_lock(&g_mutex);
	CPublishClntBase* _publisher = (CPublishClntBase*)publisher;
	pthread_mutex_unlock(&g_mutex);
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return JNI_TRUE;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_addRtspPublishNode(JNIEnv *env, jobject self, jlong handle, jstring url, jstring jappName)
{
	int result = 0;
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));

	CPublishClntBase* _publisher = (CPublishClntBase*)handle;
	const char * szUrl = env->GetStringUTFChars(url, 0);
	std::string tmpUrl = szUrl;
	const char *szAppName = env->GetStringUTFChars(jappName, 0);
	std::string tmpAppName = szAppName;
	result = _publisher->AddPublishServer(tmpUrl, tmpAppName);
	env->ReleaseStringUTFChars(url, szUrl);
	env->ReleaseStringUTFChars(jappName, szAppName);
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return result == 0 ? JNI_TRUE : JNI_FALSE;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_addS3PublishNode(JNIEnv *env, jobject self, jlong handle, jstring jid,
		jstring jhost, jstring jaccessId, jstring jsecKey,
		jstring jbucket, jstring jfolder, jstring jfilePerfix)
{
	int result = 0;

	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));

	CPublishClntBase* _publisher = (CPublishClntBase*)handle;

	const char * szId = env->GetStringUTFChars(jid, 0);
	std::string tmpId = szId;

	const char * szhost = env->GetStringUTFChars(jhost, 0);
	std::string tmphost = szhost;
	const char *szAppName = env->GetStringUTFChars(jaccessId, 0);
	std::string tmpaccessId = szAppName;
	const char *szsecKey = env->GetStringUTFChars(jsecKey, 0);
	std::string tmpsecKey = szsecKey;

	const char *szbucket = env->GetStringUTFChars(jbucket, 0);
	std::string tmpbucket = szbucket;
	const char *szfolder = env->GetStringUTFChars(jfolder, 0);
	std::string tmpfolder = szfolder;
	const char *szfilePerfix = env->GetStringUTFChars(jfilePerfix, 0);
	std::string tmpfilePerfix = szfilePerfix;

	result = _publisher->AddS3PublishNode(tmpId, tmphost, tmpaccessId, tmpsecKey, tmpbucket, tmpfolder, tmpfilePerfix);

	env->ReleaseStringUTFChars(jid, szId);
	env->ReleaseStringUTFChars(jhost, szhost);
	env->ReleaseStringUTFChars(jaccessId, szAppName);
	env->ReleaseStringUTFChars(jsecKey, szsecKey);

	env->ReleaseStringUTFChars(jbucket, szbucket);
	env->ReleaseStringUTFChars(jfolder, szfolder);
	env->ReleaseStringUTFChars(jfilePerfix, szfilePerfix);

	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));

	return result == 0 ? JNI_TRUE : JNI_FALSE;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_CreateMpd(JNIEnv *env, jobject self, jlong handle, jstring jid, jboolean islive, jint durationMs)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	int result = 0;

	CDashMultiPublishClnt *pDash = (CDashMultiPublishClnt *)handle;
	const char * szId = env->GetStringUTFChars(jid, 0);
	std::string tmpId = szId;

	result = pDash->CreateMpd(tmpId, islive, durationMs);

	env->ReleaseStringUTFChars(jid, szId);
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return result == 0 ? JNI_TRUE : JNI_FALSE;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_CreatePeriod(JNIEnv *env, jobject self, jlong handle, jstring jmpdId, jstring jperiodId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	int result = 0;
	CDashMultiPublishClnt *pDash = (CDashMultiPublishClnt *)handle;

	const char * szmpdId = env->GetStringUTFChars(jmpdId, 0);
	std::string tmpmpdId = szmpdId;
	const char * szperiodId = env->GetStringUTFChars(jperiodId, 0);
	std::string tmpperiodId = szperiodId;

	result = pDash->CreatePeriod(tmpmpdId, tmpperiodId);

	env->ReleaseStringUTFChars(jmpdId, szmpdId);
	env->ReleaseStringUTFChars(jperiodId, szperiodId);

	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return result == 0 ? JNI_TRUE : JNI_FALSE;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_CreateAdaptationSet(JNIEnv *env, jobject self, jlong handle, jstring jmpdId, jstring jperiodId, jstring jadaptId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	int result = 0;

	CDashMultiPublishClnt *pDash = (CDashMultiPublishClnt *)handle;

	const char * szmpdId = env->GetStringUTFChars(jmpdId, 0);
	std::string tmpmpdId = szmpdId;
	const char * szperiodId = env->GetStringUTFChars(jperiodId, 0);
	std::string tmpperiodId = szperiodId;

	const char * szadaptId = env->GetStringUTFChars(jadaptId, 0);
	std::string tmpadaptId = szadaptId;

	result = pDash->CreateAdaptationSet(tmpmpdId, tmpperiodId, tmpadaptId);

	env->ReleaseStringUTFChars(jadaptId, szadaptId);
	env->ReleaseStringUTFChars(jmpdId, szmpdId);
	env->ReleaseStringUTFChars(jperiodId, szperiodId);
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return result == 0 ? JNI_TRUE : JNI_FALSE;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_CreateRepresentation(JNIEnv *env, jobject self, jlong handle,  jstring jmpdId, jstring jperiodId, jstring jadaptId, jstring jrepId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));

	int result = 0;
	CDashMultiPublishClnt *pDash = (CDashMultiPublishClnt *)handle;

	const char * szmpdId = env->GetStringUTFChars(jmpdId, 0);
	std::string tmpmpdId = szmpdId;
	const char * szperiodId = env->GetStringUTFChars(jperiodId, 0);
	std::string tmpperiodId = szperiodId;

	const char * szadaptId = env->GetStringUTFChars(jadaptId, 0);
	std::string tmpadaptId = szadaptId;

	const char * szrepId = env->GetStringUTFChars(jrepId, 0);
	std::string tmprepId = szrepId;

	result = pDash->CreateRepresentation(tmpmpdId, tmpperiodId, tmpadaptId, tmprepId);

	env->ReleaseStringUTFChars(jadaptId, szadaptId);
	env->ReleaseStringUTFChars(jmpdId, szmpdId);
	env->ReleaseStringUTFChars(jperiodId, szperiodId);

	env->ReleaseStringUTFChars(jrepId, szrepId);

	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));

	return result == 0 ? JNI_TRUE : JNI_FALSE;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_CreateMpdPublishStream(
		JNIEnv *env, jobject self, jlong handle,
		jstring jId,
		jstring jmpdId,
		jstring jperiodId,
		jstring jadaptId,
		jstring jrepId,
		jstring jswitchId,
		jstring jserverNode)
{
	int result = 0;
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	CDashMultiPublishClnt *pDash = (CDashMultiPublishClnt *)handle;

	const char * szId = env->GetStringUTFChars(jId, 0);
	std::string strId = szId;

	const char * szmpdId = env->GetStringUTFChars(jmpdId, 0);
	std::string strmpdId = szmpdId;

	const char * szperiodId = env->GetStringUTFChars(jperiodId, 0);
	std::string strperiodId = szperiodId;

	const char * szadaptId = env->GetStringUTFChars(jadaptId, 0);
	std::string stradaptId = szadaptId;

	const char * szrepId = env->GetStringUTFChars(jrepId, 0);
	std::string strrepId = szrepId;

	const char * szSwitchId = env->GetStringUTFChars(jswitchId, 0);
	std::string strSwitchId = szSwitchId;

	const char * szServerNode = env->GetStringUTFChars(jserverNode, 0);
	std::string strServerNode= szServerNode;


	result = pDash->CreateMpdPublishStream(strId, strmpdId, strperiodId,
			stradaptId, strrepId, strSwitchId, strServerNode);

	env->ReleaseStringUTFChars(jId, szId);
	env->ReleaseStringUTFChars(jmpdId, szmpdId);
	env->ReleaseStringUTFChars(jperiodId, szperiodId);
	env->ReleaseStringUTFChars(jadaptId, szadaptId);
	env->ReleaseStringUTFChars(jrepId, szrepId);
	env->ReleaseStringUTFChars(jswitchId, szSwitchId);
	env->ReleaseStringUTFChars(jserverNode, szServerNode);
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return result == 0 ? JNI_TRUE : JNI_FALSE;
}



jboolean Java_com_mcntech_ezscreencast_OnyxApi_StartMpdPublishStream(JNIEnv *env, jobject self, jlong handle,  jstring jId)
{
	int result = 0;
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	CDashMultiPublishClnt *pDash = (CDashMultiPublishClnt *)handle;

	const char * szId = env->GetStringUTFChars(jId, 0);
	std::string tmpId = szId;

	result = pDash->SatrtMpdPublishStream(tmpId);

	env->ReleaseStringUTFChars(jId, szId);
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return result == 0 ? JNI_TRUE : JNI_FALSE;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_UpdateMpdPublishStatus(JNIEnv *env, jobject self, jlong handle,  jstring jId)
{
	int result = 0;
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	CDashMultiPublishClnt *pDash = (CDashMultiPublishClnt *)handle;

	const char * szId = env->GetStringUTFChars(jId, 0);
	std::string tmpId = szId;
	pDash->UpdateMpdPublishStatus(tmpId);

	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return JNI_TRUE;

}
jboolean Java_com_mcntech_ezscreencast_OnyxApi_CreateInputStream(JNIEnv *env, jobject self, jlong handle, jstring jid, jstring jInputType, jstring jUrl)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	int result = 0;
	CPublishClntBase* _publisher = (CPublishClntBase*)handle;
	const char * szId = env->GetStringUTFChars(jid, 0);
	const char * szInputType = env->GetStringUTFChars(jInputType, 0);
	const char * szUrl = env->GetStringUTFChars(jUrl, 0);
	result = _publisher->CreateInputStrm(szId, szInputType, szUrl);

	env->ReleaseStringUTFChars(jid, szId);
	env->ReleaseStringUTFChars(jInputType, szId);
	env->ReleaseStringUTFChars(jUrl, szUrl);
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return result == 0 ? JNI_TRUE : JNI_FALSE;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_CreateSwitch(JNIEnv *env, jobject self, jlong handle, jstring jid)
{
	int result = 0;
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	CPublishClntBase* _publisher = (CPublishClntBase*)handle;
	const char * szId = env->GetStringUTFChars(jid, 0);
	result = _publisher->CreateSwitch(szId);

	env->ReleaseStringUTFChars(jid, szId);
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	return result == 0 ? JNI_TRUE : JNI_FALSE;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_StartSwitch(JNIEnv *env, jobject self, jlong handle, jstring jid)
{
	int result = 0;
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	CPublishClntBase* _publisher = (CPublishClntBase*)handle;
	const char * szId = env->GetStringUTFChars(jid, 0);

	result = _publisher->startSwitch(szId);

	env->ReleaseStringUTFChars(jid, szId);
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return result == 0 ? JNI_TRUE : JNI_FALSE;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_ConnectSwitchInput(JNIEnv *env, jobject self, jlong handle, jstring jSwitchId, jstring jInputId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	int result = 0;
	CPublishClntBase* _publisher = (CPublishClntBase*)handle;
	CDashMultiPublishClnt *pDash = (CDashMultiPublishClnt *)_publisher;

	const char * szSwitchId = env->GetStringUTFChars(jSwitchId, 0);
	const char * szInputId = env->GetStringUTFChars(jInputId, 0);

	result = _publisher->ConnectSwitchInput(szSwitchId, szInputId);

	env->ReleaseStringUTFChars(jSwitchId, szSwitchId);
	env->ReleaseStringUTFChars(jInputId, szInputId);
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return result == 0 ? JNI_TRUE : JNI_FALSE;
}

jint Java_com_mcntech_ezscreencast_OnyxApi_sendAudioData(JNIEnv *env, jobject self, jlong publisher, jstring jInputId, jbyteArray pcmBytes,jint numBytes, long Pts, int Flags)
{
	int result = 0;
	//pthread_mutex_lock(&g_mutex);
	JDBG_LOG(CJdDbg::LVL_STRM, ("%s:Ener", __FUNCTION__));
	CPublishClntBase* _publisher = (CPublishClntBase*)publisher;
	const char * szInputId = env->GetStringUTFChars(jInputId, 0);
	std::string strInputId = szInputId;
	ConnCtxT *pConn = _publisher->GetInputStrmConn(szInputId, 0/*aud*/);
	if(pConn) {
		jboolean isCopy;
		int len = env->GetArrayLength (pcmBytes);

		jbyte* rawjBytes = env->GetByteArrayElements(pcmBytes, &isCopy);
		pConn->Write(pConn, (char *)rawjBytes, len, Pts, Flags);
		env->ReleaseByteArrayElements(pcmBytes,rawjBytes, 0);
	} else{
		JDBG_LOG(CJdDbg::LVL_ERR,("%s:%d:aud %s pConn==NULL", __FILE__, __LINE__,szInputId ));
	}
	env->ReleaseStringUTFChars(jInputId, szInputId);
	JDBG_LOG(CJdDbg::LVL_STRM, ("%s:Leave", __FUNCTION__));
	//pthread_mutex_unlock(&g_mutex);
	return result;
}

jint Java_com_mcntech_ezscreencast_OnyxApi_sendVideoData(JNIEnv *env, jobject self, jlong publisher, jstring jInputId, jbyteArray pcmBytes,jint numBytes, long Pts, int Flags)
{
	int result = 0;
	//pthread_mutex_lock(&g_mutex);
	JDBG_LOG(CJdDbg::LVL_STRM, ("%s:Ener", __FUNCTION__));
	CPublishClntBase* _publisher = (CPublishClntBase*)publisher;
	JDBG_LOG(CJdDbg::LVL_STRM,("sendVideoData Enter"));
	const char * szInputId = env->GetStringUTFChars(jInputId, 0);
	std::string strInputId = szInputId;
	ConnCtxT *pConn = _publisher->GetInputStrmConn(szInputId, 1/*vid*/);

	if(pConn) {
		jboolean isCopy;
		int len = env->GetArrayLength (pcmBytes);

		jbyte* rawjBytes = env->GetByteArrayElements(pcmBytes, &isCopy);
		pConn->Write(pConn, (char *)rawjBytes, len, Pts, Flags);
		env->ReleaseByteArrayElements(pcmBytes,rawjBytes, 0);
	} else {
		JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%d:vid %s pConn==NULL", __FILE__, __LINE__,szInputId ));
	}
	env->ReleaseStringUTFChars(jInputId, szInputId);
	JDBG_LOG(CJdDbg::LVL_STRM, ("%s:Leave", __FUNCTION__));
	return result;
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
	JDBG_LOG(CJdDbg::LVL_STRM, ("%s:Ener", __FUNCTION__));
	CPublishClntBase* _publisher = (CPublishClntBase*)publisher;
	jlong clock = 0;
	uint64_t freq;
	//TODO
	JDBG_LOG(CJdDbg::LVL_STRM, ("%s:Leave", __FUNCTION__));
	return clock;
}
} // extern "C"
