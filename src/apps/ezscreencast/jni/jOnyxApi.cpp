#include <string.h>
#include <jni.h>
#include <android/log.h>
#include <InprocStrmConn.h>
//#include "jEventHandler.h"
#include "RtspMultiPublishClnt.h"
#include "DashMultiPublishClnt.h"
#include "PublishClntBase.h"
#include "jOnyxEvents.h"

#define PUBLISH_TYPE_NONE  0
#define PUBLISH_TYPE_RTSP  1
#define PUBLISH_TYPE_MPD   2
#define PUBLISH_TYPE_HLS   3
#define PUBLISH_TYPE_RTMP  4

#define DBGLOG(...) ((void) __android_log_print(ANDROID_LOG_DEBUG  ,"ezscreencast",  __VA_ARGS__))

pthread_mutex_t   g_mutex = PTHREAD_MUTEX_INITIALIZER;
CPublishClntBase  *g_pMultiPublish;
CInprocStrmConn   *gpStrmInputInproc0 = NULL;

extern "C" {

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	DBGLOG("JNI_OnLoad ");
	return JNI_VERSION_1_6;
}

jlong Java_com_mcntech_ezscreencast_OnyxApi_init(JNIEnv *env, jobject self,jint protocol)
{
	pthread_mutex_lock(&g_mutex);

	/* Initialize publish client */
	if(g_pMultiPublish == NULL) {
		COnyxEvents *pEventCallback = new COnyxEvents(env, self);
		if(protocol == PUBLISH_TYPE_RTSP)
			g_pMultiPublish =  CRtspMultiPublishClnt::openInstance(pEventCallback);
		if(protocol == PUBLISH_TYPE_MPD)
			g_pMultiPublish =  CDashMultiPublishClnt::openInstance(pEventCallback);
	}

	/* Get strmconn interfacce */
	CInprocStrmConnRegistry *pRegistry = CInprocStrmConnRegistry::getRegistry();
	const char *pszInputUri = "Input0";
	gpStrmInputInproc0 = pRegistry->getEntry(pszInputUri);

	DBGLOG("initializing screencat");
	pthread_mutex_unlock(&g_mutex);
	return (jlong)g_pMultiPublish;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_deinit(JNIEnv *env, jobject self, jlong publisher)
{
	DBGLOG("MediaController_deinit:Start");
	pthread_mutex_lock(&g_mutex);
	CPublishClntBase* _publisher = (CPublishClntBase*)publisher;
	//_publisher->closeInstancce((CPublishClntBase *)g_pMultiPublish);
	delete g_pMultiPublish;
	g_pMultiPublish = NULL;
	pthread_mutex_unlock(&g_mutex);
	DBGLOG("MediaController_deinit:End");
	return true;
}

// play from start of track every time
jboolean Java_com_mcntech_ezscreencast_OnyxApi_start(JNIEnv *env, jobject self, jlong publisher, jstring path, jstring title, jstring artist)
{
	//todo if(!CPublishClntBase::safeLock(&g_mutex))//pthread_mutex_lock(&g_mutex);
	//	return false;

	CPublishClntBase* _publisher = (CPublishClntBase*)publisher;
	_publisher->stop();
	DBGLOG("jni playfile: waiting for session to end");
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
	DBGLOG("jni stopPlaying: stopped playback1");
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

jboolean Java_com_mcntech_ezscreencast_OnyxApi_addRtspPublishNode(JNIEnv *env, jobject self, jlong handle, jstring url, jstring jappName)
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

jboolean Java_com_mcntech_ezscreencast_OnyxApi_addS3PublishNode(JNIEnv *env, jobject self, jlong handle, jstring jid,
		jstring jhost, jstring jaccessId, jstring jsecKey,
		jstring jbucket, jstring jfolder, jstring jfilePerfix)
{
	int result = 0;
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

	_publisher->AddS3PublishNode(tmpId, tmphost, tmpaccessId, tmpsecKey, tmpbucket, tmpfolder, tmpfilePerfix);

	env->ReleaseStringUTFChars(jid, szId);
	env->ReleaseStringUTFChars(jhost, szhost);
	env->ReleaseStringUTFChars(jaccessId, szAppName);
	env->ReleaseStringUTFChars(jsecKey, szsecKey);

	env->ReleaseStringUTFChars(jbucket, szbucket);
	env->ReleaseStringUTFChars(jfolder, szfolder);
	env->ReleaseStringUTFChars(jfilePerfix, szfilePerfix);
	return JNI_TRUE;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_CreateMpd(JNIEnv *env, jobject self, jlong handle, jstring jid)
{
	DBGLOG("%s:%d", __FILE__, __LINE__);
	int result = 0;

	CDashMultiPublishClnt *pDash = (CDashMultiPublishClnt *)handle;
	const char * szId = env->GetStringUTFChars(jid, 0);
	std::string tmpId = szId;
	DBGLOG("%s:%d", __FILE__, __LINE__);
	pDash->CreateMpd(tmpId);

	env->ReleaseStringUTFChars(jid, szId);
	DBGLOG("%s:%d", __FILE__, __LINE__);
	return JNI_TRUE;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_CreatePeriod(JNIEnv *env, jobject self, jlong handle, jstring jmpdId, jstring jperiodId)
{
	DBGLOG("%s:%d", __FILE__, __LINE__);
	int result = 0;
	CDashMultiPublishClnt *pDash = (CDashMultiPublishClnt *)handle;

	const char * szmpdId = env->GetStringUTFChars(jmpdId, 0);
	std::string tmpmpdId = szmpdId;
	const char * szperiodId = env->GetStringUTFChars(jperiodId, 0);
	std::string tmpperiodId = szperiodId;

	pDash->CreatePeriod(tmpmpdId, tmpperiodId);

	env->ReleaseStringUTFChars(jmpdId, szmpdId);
	env->ReleaseStringUTFChars(jperiodId, szperiodId);

	DBGLOG("%s:%d", __FILE__, __LINE__);

	return JNI_TRUE;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_CreateAdaptationSet(JNIEnv *env, jobject self, jlong handle, jstring jmpdId, jstring jperiodId, jstring jadaptId)
{
	DBGLOG("%s:%d", __FILE__, __LINE__);
	int result = 0;

	CDashMultiPublishClnt *pDash = (CDashMultiPublishClnt *)handle;

	const char * szmpdId = env->GetStringUTFChars(jmpdId, 0);
	std::string tmpmpdId = szmpdId;
	const char * szperiodId = env->GetStringUTFChars(jperiodId, 0);
	std::string tmpperiodId = szperiodId;

	const char * szadaptId = env->GetStringUTFChars(jadaptId, 0);
	std::string tmpadaptId = szadaptId;

	pDash->CreateAdaptationSet(tmpmpdId, tmpperiodId, tmpadaptId);

	env->ReleaseStringUTFChars(jadaptId, szadaptId);
	env->ReleaseStringUTFChars(jmpdId, szmpdId);
	env->ReleaseStringUTFChars(jperiodId, szperiodId);
	DBGLOG("%s:%d", __FILE__, __LINE__);
	return JNI_TRUE;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_CreateRepresentation(JNIEnv *env, jobject self, jlong handle,  jstring jmpdId, jstring jperiodId, jstring jadaptId, jstring jrepId)
{
	DBGLOG("%s:%d", __FILE__, __LINE__);

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

	pDash->CreateRepresentation(tmpmpdId, tmpperiodId, tmpadaptId, tmprepId);

	env->ReleaseStringUTFChars(jadaptId, szadaptId);
	env->ReleaseStringUTFChars(jmpdId, szmpdId);
	env->ReleaseStringUTFChars(jperiodId, szperiodId);

	env->ReleaseStringUTFChars(jrepId, szrepId);

	DBGLOG("%s:%d", __FILE__, __LINE__);

	return JNI_TRUE;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_CreateMpdPublishStream(JNIEnv *env, jobject self, jlong handle,  jstring jmpdId, jstring jperiodId, jstring jadaptId, jstring jrepId, jstring jswitchId, jstring jserverNode)
{
	int result = 0;
	DBGLOG("%s:%d", __FILE__, __LINE__);
	CDashMultiPublishClnt *pDash = (CDashMultiPublishClnt *)handle;

	const char * szmpdId = env->GetStringUTFChars(jmpdId, 0);
	std::string tmpmpdId = szmpdId;

	const char * szperiodId = env->GetStringUTFChars(jperiodId, 0);
	std::string tmpperiodId = szperiodId;

	const char * szadaptId = env->GetStringUTFChars(jadaptId, 0);
	std::string tmpadaptId = szadaptId;

	const char * szrepId = env->GetStringUTFChars(jrepId, 0);
	std::string tmprepId = szrepId;

	const char * szSwitchId = env->GetStringUTFChars(jswitchId, 0);
	std::string strSwitchId = szSwitchId;

	const char * szServerNode = env->GetStringUTFChars(jserverNode, 0);
	std::string strServerNode= szServerNode;

	DBGLOG("%s:%d", __FILE__, __LINE__);
	pDash->CreateMpdPublishStream(tmpmpdId, tmpperiodId, tmpadaptId, tmprepId, strSwitchId, strServerNode);
	DBGLOG("%s:%d", __FILE__, __LINE__);

	env->ReleaseStringUTFChars(jadaptId, szadaptId);
	env->ReleaseStringUTFChars(jmpdId, szmpdId);
	env->ReleaseStringUTFChars(jperiodId, szperiodId);
	env->ReleaseStringUTFChars(jperiodId, szperiodId);
	env->ReleaseStringUTFChars(jrepId, szrepId);
	env->ReleaseStringUTFChars(jswitchId, szSwitchId);
	env->ReleaseStringUTFChars(jserverNode, szServerNode);
	DBGLOG("%s:%d", __FILE__, __LINE__);
	return JNI_TRUE;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_CreateInputStream(JNIEnv *env, jobject self, jlong handle, jstring jid, jstring jInputType, jstring jUrl)
{
	DBGLOG("%s:%d", __FILE__, __LINE__);
	int result = 0;
	CPublishClntBase* _publisher = (CPublishClntBase*)handle;
	const char * szId = env->GetStringUTFChars(jid, 0);
	const char * szInputType = env->GetStringUTFChars(jInputType, 0);
	const char * szUrl = env->GetStringUTFChars(jUrl, 0);
	_publisher->CreateInputStrm(szId, szInputType, szUrl);

	env->ReleaseStringUTFChars(jid, szId);
	env->ReleaseStringUTFChars(jInputType, szId);
	env->ReleaseStringUTFChars(jUrl, szUrl);
	DBGLOG("%s:%d", __FILE__, __LINE__);
	return JNI_TRUE;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_CreateSwitch(JNIEnv *env, jobject self, jlong handle, jstring jid)
{
	int result = 0;
	DBGLOG("%s:%d", __FILE__, __LINE__);
	CPublishClntBase* _publisher = (CPublishClntBase*)handle;
	const char * szId = env->GetStringUTFChars(jid, 0);
	_publisher->CreateSwitch(szId);

	env->ReleaseStringUTFChars(jid, szId);
	DBGLOG("%s:%d", __FILE__, __LINE__);
	return JNI_TRUE;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_ConnectSwitchInput(JNIEnv *env, jobject self, jlong handle, jstring jSwitchId, jstring jInputId)
{
	DBGLOG("%s:%d", __FILE__, __LINE__);
	int result = 0;
	CPublishClntBase* _publisher = (CPublishClntBase*)handle;
	CDashMultiPublishClnt *pDash = (CDashMultiPublishClnt *)_publisher;

	const char * szSwitchId = env->GetStringUTFChars(jSwitchId, 0);
	const char * szInputId = env->GetStringUTFChars(jInputId, 0);

	_publisher->ConnectSwitchInput(szSwitchId, szInputId);

	env->ReleaseStringUTFChars(jSwitchId, szSwitchId);
	env->ReleaseStringUTFChars(jInputId, szInputId);
	DBGLOG("%s:%d", __FILE__, __LINE__);
	return JNI_TRUE;
}

jint Java_com_mcntech_ezscreencast_OnyxApi_sendAudioData(JNIEnv *env, jobject self, jlong publisher, jstring jinputId, jbyteArray pcmBytes,jint numBytes, long Pts, int Flags)
{
	int result = 0;
	//pthread_mutex_lock(&g_mutex);
	CPublishClntBase* _publisher = (CPublishClntBase*)publisher;
	
	jboolean isCopy;
	int len = env->GetArrayLength (pcmBytes);
	jbyte* rawjBytes = env->GetByteArrayElements(pcmBytes, &isCopy);
	//result = _publisher->sendAudioData((const char *)rawjBytes, len, Pts, Flags);
	if(gpStrmInputInproc0 && gpStrmInputInproc0->m_pAudCon){
		ConnCtxT *pStrm = gpStrmInputInproc0->m_pAudCon;
		pStrm->Write(pStrm, (char *)rawjBytes, len, Pts, Flags);
	}

	env->ReleaseByteArrayElements(pcmBytes,rawjBytes,0);
	//pthread_mutex_unlock(&g_mutex);
	return result;
}

jint Java_com_mcntech_ezscreencast_OnyxApi_sendVideoData(JNIEnv *env, jobject self, jlong publisher, jstring jinputId, jbyteArray pcmBytes,jint numBytes, long Pts, int Flags)
{
	int result = 0;
	//pthread_mutex_lock(&g_mutex);
	CPublishClntBase* _publisher = (CPublishClntBase*)publisher;
	jboolean isCopy;
	int len = env->GetArrayLength (pcmBytes);
	jbyte* rawjBytes = env->GetByteArrayElements(pcmBytes, &isCopy);
	//result = _publisher->sendVideoData((const char *)rawjBytes, len, Pts, Flags);
	if(gpStrmInputInproc0 && gpStrmInputInproc0->m_pVidCon){
		ConnCtxT *pStrm = gpStrmInputInproc0->m_pVidCon;
		pStrm->Write(pStrm, (char *)rawjBytes, len, Pts, Flags);
	}
	env->ReleaseByteArrayElements(pcmBytes,rawjBytes,0);
	return result;
}

jboolean Java_com_mcntech_ezscreencast_OnyxApi_startSession(JNIEnv *env, jobject self, jlong publisher, jboolean fEnableAudio, jboolean fEnableVideo)
{
	DBGLOG("%s:%d", __FILE__, __LINE__);
	pthread_mutex_lock(&g_mutex);
	CPublishClntBase* _publisher = (CPublishClntBase*)publisher;
	bool result =  JNI_FALSE;
	//TODO
	pthread_mutex_unlock(&g_mutex);
	DBGLOG("%s:%d", __FILE__, __LINE__);
	return result;
}


jboolean Java_com_mcntech_ezscreencast_OnyxApi_endSession(JNIEnv *env, jobject self, jlong publisher)
{
	DBGLOG("MediaController_endSession:Begin");
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
