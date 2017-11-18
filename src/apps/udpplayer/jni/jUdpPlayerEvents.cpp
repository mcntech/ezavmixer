#include "jUdpPlayerEvents.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <android/log.h>
#include "JdDbg.h"

JavaVM* g_vm;
jobject g_jniGlobalSelf = NULL;
jclass g_deviceClass = NULL;
static int  modDbgLevel = CJdDbg::LVL_TRACE;

CUdpPlayerEvents::CUdpPlayerEvents(JNIEnv* env,jobject javaReceiver)
{
	env->GetJavaVM(&g_vm);
	g_jniGlobalSelf = env->NewGlobalRef(javaReceiver);
	g_deviceClass = env->FindClass("com/mcntech/rtspplayer/OnyxPlayerApi");
	g_deviceClass = (jclass)env->NewGlobalRef(g_deviceClass);
	pthread_mutex_init(&m_eventMutex, NULL);
}

int CUdpPlayerEvents::printlog(const char *tag, const char *msg, va_list args)
{
	__android_log_vprint(ANDROID_LOG_INFO  , "[app]", msg, args);
	return 0;
}


bool CUdpPlayerEvents::onNativeMessage(char *szTitle, char *szMsg)
{
	JNIEnv* env;
	safeAttach(&env);//must always call safeDetach() before returning
	jclass onyxApi = env->GetObjectClass(g_jniGlobalSelf);
	jmethodID callback = env->GetStaticMethodID(onyxApi, "onNativeMessage", "(Ljava/lang/Object;Ljava/lang/Object;)V");
	jstring jtitle = env->NewStringUTF(szTitle);
	jstring jmessage = env->NewStringUTF(szMsg);
	env->CallStaticVoidMethod(onyxApi, callback,jtitle,jmessage);
	env->DeleteLocalRef(jtitle);
	env->DeleteLocalRef(jmessage);
}

bool CUdpPlayerEvents::onRemoteNodeError(char *url, char *szErr)
{
	JNIEnv* env;
	safeAttach(&env);//must always call safeDetach() before returning
	jclass onyxApi = env->GetObjectClass(g_jniGlobalSelf);
	jstring jurl = env->NewStringUTF(url);
	jstring jmsg = env->NewStringUTF(szErr);

	jmethodID callback = env->GetStaticMethodID(onyxApi, "onRemoteNodeError", "(Ljava/lang/Object;Ljava/lang/Object;)V");
	env->CallStaticVoidMethod(onyxApi, callback,jurl,jmsg);
	env->DeleteLocalRef(jurl);
	env->DeleteLocalRef(jmsg);

	safeDetach();
	return true;
}

bool CUdpPlayerEvents::onServerStatus(const char *szUrl, int nState, int nStrmInTime, int nStrmOutTime, int nLostBufferTime)
{
	JNIEnv* env;
	safeAttach(&env);
	jclass onyxApi = env->GetObjectClass(g_jniGlobalSelf);
	if(onyxApi != NULL) {
		jmethodID callback = env->GetStaticMethodID(onyxApi, "onMpdPublishStatus", "(Ljava/lang/String;IIII)V");
		if(callback != NULL) {
			jstring jPublishId = env->NewStringUTF(szUrl);

			env->CallStaticVoidMethod(onyxApi, callback, jPublishId, nState, nStrmInTime, nStrmOutTime,  nLostBufferTime);
			env->DeleteLocalRef(jPublishId);
		} else {
			JDBG_LOG(CJdDbg::LVL_ERR, ("Failed to find onMpdPublishStatus on com/mcntech/rtspplayer/OnyxPlayerApi"));
		}

	} else {
		JDBG_LOG(CJdDbg::LVL_ERR, ("Failed to find com/mcntech/rtspplayer/OnyxPlayerApi"));
	}
	safeDetach();
	return true;
}

bool CUdpPlayerEvents::onServerStatistics(const char *szPublishId, UDP_SERVER_STATS *pStat)
{
	return true;
}

bool CUdpPlayerEvents::onDiscoverRemoteNode(char *url)
{
	JNIEnv* env;
	safeAttach(&env);//must always call safeDetach() before returning
	jclass onyxApi = env->GetObjectClass(g_jniGlobalSelf);
	if(onyxApi != NULL) {
		jmethodID callback = env->GetStaticMethodID(onyxApi, "onDiscoverRemoteNode", "(Ljava/lang/String;)V");
		if(callback != NULL) {
			jstring jurl = env->NewStringUTF(url);
			env->CallStaticVoidMethod(onyxApi, callback, jurl);
			env->DeleteLocalRef(jurl);
		}else {
			JDBG_LOG(CJdDbg::LVL_ERR, ("Failed to find onDiscoverRemoteNode on com/mcntech/rtspplayer/OnyxPlayerApi"));
		}
	} else {
		JDBG_LOG(CJdDbg::LVL_ERR, ("Failed to find com/mcntech/rtspplayer/OnyxPlayerApi"));
	}

	safeDetach();
	return true;
}

bool CUdpPlayerEvents::onConnectRemoteNode(char *url)
{
	JNIEnv* env;
	safeAttach(&env);//must always call safeDetach() before returning
	jclass onyxApi = env->GetObjectClass(g_jniGlobalSelf);
	jstring jurl = env->NewStringUTF(url);

	jmethodID callback = env->GetStaticMethodID(onyxApi, "onConnectRemoteNode", "(Ljava/lang/Object;)V");
	env->CallStaticVoidMethod(onyxApi, callback, jurl);
	env->DeleteLocalRef(jurl);


	safeDetach();
	return true;
}

bool CUdpPlayerEvents::onDisconnectRemoteNode(char *url)
{
	JNIEnv* env;
	safeAttach(&env);//must always call safeDetach() before returning
	jclass onyxApi = env->GetObjectClass(g_jniGlobalSelf);
	jstring jurl = env->NewStringUTF(url);

	jmethodID callback = env->GetStaticMethodID(onyxApi, "onDisconnectRemoteNode", "(Ljava/lang/Object;)V");
	env->CallStaticVoidMethod(onyxApi, callback,jurl);
	env->DeleteLocalRef(jurl);

	safeDetach();
	return true;
}

bool CUdpPlayerEvents::onStatusRemoteNode(char *url, char *szErr)
{
	JNIEnv* env;
	safeAttach(&env);//must always call safeDetach() before returning
	jclass onyxApi = env->GetObjectClass(g_jniGlobalSelf);
	jstring jurl = env->NewStringUTF(url);
	jstring jmsg = env->NewStringUTF(szErr);

	jmethodID callback = env->GetStaticMethodID(onyxApi, "onStatusRemoteNode", "(Ljava/lang/Object;Ljava/lang/Object;)V");
	env->CallStaticVoidMethod(onyxApi, callback,jurl,jmsg);
	env->DeleteLocalRef(jurl);
	env->DeleteLocalRef(jmsg);

	safeDetach();
	return true;
}

bool CUdpPlayerEvents::attachThread(JNIEnv** env){
  bool changed = false;
  switch (g_vm->GetEnv((void**)env, JNI_VERSION_1_6))
  {
  	case JNI_OK:
    	break;
  	case JNI_EDETACHED:
	    if (g_vm->AttachCurrentThread(env, NULL)!=0)
	    {
	    	(*env)->ExceptionDescribe(); // write exception data to the console 
	      	(*env)->ExceptionClear();    // clear the exception that was pending 
	      	break;
	   	}
    	changed = true;
    	break;
	  case JNI_EVERSION:
    	break;
	  default:
		break;
  }
  return m_jniThreadChanged= changed;	
}

bool CUdpPlayerEvents::safeAttach(JNIEnv** env){
	if(*env == NULL){
		JNIEnv envalloc;
		*env = &envalloc;
	}
	pthread_mutex_lock(&m_eventMutex);
	//m_eventStarted = true;
	//m_lastEventThread = pthread_self();
	return attachThread(env);
}

void CUdpPlayerEvents::safeDetach()
{
	if(m_jniThreadChanged)
		g_vm->DetachCurrentThread();
	//m_eventStarted = false;
	//m_waitForEvent = false;
	pthread_mutex_unlock(&m_eventMutex);
}

/*
long myMethod (int n, String s, int[] arr);
is seen from JNI with the signature
(ILJAVA/LANG/STRING;[I)J


Type     Chararacter
boolean      Z
byte         B
char         C
double       D
float        F
int          I
long         J
object       L
short        S
void         V
array        [
Note that to specify an object, the "L" is followed by the object's class name and ends with a semi-colon, ';' .
Ljava/lang/String;

*/