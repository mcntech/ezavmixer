#include "jUdpPlayerEvents.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <android/log.h>
#include "JdDbg.h"

JavaVM* g_vm;
jobject g_jniGlobalSelf = NULL;
static int  modDbgLevel = CJdDbg::LVL_TRACE;

CUdpPlayerEvents::CUdpPlayerEvents(JNIEnv* env,jobject javaReceiver)
{
	env->GetJavaVM(&g_vm);
	g_jniGlobalSelf = env->NewGlobalRef(javaReceiver);

    // Test
    //jclass onyxApi = env->GetObjectClass(g_jniGlobalSelf);
    //jmethodID callback = env->GetStaticMethodID(onyxApi, "onPsiChange", "(Ljava/lang/String;Ljava/lang/String;)V");

	pthread_mutex_init(&m_eventMutex, NULL);
}

int CUdpPlayerEvents::printlog(const char *tag, const char *msg, va_list args)
{
	__android_log_vprint(ANDROID_LOG_INFO  , "[app]", msg, args);
	return 0;
}


bool CUdpPlayerEvents::onServerStatistics(const char *szPublishId, UDP_SERVER_STATS *pStat)
{
	return true;
}

bool CUdpPlayerEvents::onPsiPatChange(const char *url, const char *pPsiData)
{
	JNIEnv* env;
	safeAttach(&env);
	jclass onyxApi = env->GetObjectClass(g_jniGlobalSelf);
	if(onyxApi != NULL) {
		jmethodID callback = env->GetStaticMethodID(onyxApi, "onPsiPatChange", "(Ljava/lang/String;Ljava/lang/String;)V");
		if(callback != NULL) {
			jstring jurl = env->NewStringUTF(url);
            jstring jpsi = env->NewStringUTF(pPsiData);
			env->CallStaticVoidMethod(onyxApi, callback, jurl, jpsi);
			env->DeleteLocalRef(jurl);
            env->DeleteLocalRef(jpsi);
		}else {
			JDBG_LOG(CJdDbg::LVL_ERR, ("Failed to find onDiscoverRemoteNode on com/mcntech/udpplayer/UdpPlayerApi"));
		}
	} else {
		JDBG_LOG(CJdDbg::LVL_ERR, ("Failed to find com/mcntech/udpplayer/UdpPlayerApi"));
	}

	safeDetach();

	return true;
}

bool CUdpPlayerEvents::onPsiPmtChange(const char *url, int strmId, const char *pPsiData)
{
	JNIEnv* env;
	safeAttach(&env);
	jclass onyxApi = env->GetObjectClass(g_jniGlobalSelf);
	if(onyxApi != NULL) {
		jmethodID callback = env->GetStaticMethodID(onyxApi, "onPsiPmtChange", "(Ljava/lang/String;ILjava/lang/String;)V");
		if(callback != NULL) {
			jstring jurl = env->NewStringUTF(url);
			jstring jpsi = env->NewStringUTF(pPsiData);
			jint jStrmId = strmId;
			env->CallStaticVoidMethod(onyxApi, callback, jurl, jStrmId, jpsi);
			env->DeleteLocalRef(jurl);
			env->DeleteLocalRef(jpsi);
		}else {
			JDBG_LOG(CJdDbg::LVL_ERR, ("Failed to find onDiscoverRemoteNode on com/mcntech/udpplayer/UdpPlayerApi"));
		}
	} else {
		JDBG_LOG(CJdDbg::LVL_ERR, ("Failed to find com/mcntech/udpplayer/UdpPlayerApi"));
	}

	safeDetach();

	return true;
}

bool CUdpPlayerEvents::onFormatChange(const char *url, int strmId, const char *pFormat)
{
	JNIEnv* env;
	safeAttach(&env);
	jclass onyxApi = env->GetObjectClass(g_jniGlobalSelf);
	if(onyxApi != NULL) {
		jmethodID callback = env->GetStaticMethodID(onyxApi, "onFormatChange", "(Ljava/lang/String;ILjava/lang/String;)V");
		if(callback != NULL) {
			jstring jurl = env->NewStringUTF(url);
			jstring jmsg = env->NewStringUTF(pFormat);
			jint jStrmId = strmId;
			env->CallStaticVoidMethod(onyxApi, callback, jurl, jStrmId, jmsg);
			env->DeleteLocalRef(jurl);
			env->DeleteLocalRef(jmsg);
		}else {
			JDBG_LOG(CJdDbg::LVL_ERR, ("Failed to find onDiscoverRemoteNode on com/mcntech/udpplayer/UdpPlayerApi"));
		}
	} else {
		JDBG_LOG(CJdDbg::LVL_ERR, ("Failed to find com/mcntech/udpplayer/UdpPlayerApi"));
	}

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
