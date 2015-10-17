#include "jOnyxApiHandler.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <android/log.h>

JavaVM* g_vm;
jobject g_jniGlobalSelf = NULL;
jclass g_deviceClass = NULL;


COnyxEventHandler::COnyxEventHandler(JNIEnv* env,jobject javaReceiver){
	env->GetJavaVM(&g_vm);
	g_jniGlobalSelf = env->NewGlobalRef(javaReceiver);
	g_deviceClass = env->FindClass("com/mcntech/ezscreencast/OnyxApi");
	g_deviceClass = (jclass)env->NewGlobalRef(g_deviceClass);
	
	//TODO Clogger::redirect(&printlog);
}

int COnyxEventHandler::printlog(const char *tag, const char *msg, va_list args)
{
	__android_log_vprint(ANDROID_LOG_INFO  , "[app]", msg, args);
	return 0;
}


bool COnyxEventHandler::onNativeMessage(char *szTitle, char *szMsg)
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

bool COnyxEventHandler::onRemoteNodeError(char *url, char *szErr)
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

	bool COnyxEventHandler::onConnectRemoteNode(char *url)
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

	bool COnyxEventHandler::onDisconnectRemoteNode(char *url)
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

	bool COnyxEventHandler::onStatusRemoteNode(char *url, char *szErr)
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

bool COnyxEventHandler::attachThread(JNIEnv** env){
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

bool COnyxEventHandler::safeAttach(JNIEnv** env){
	if(*env == NULL){
		JNIEnv envalloc;
		*env = &envalloc;
	}
	pthread_mutex_lock(&m_eventMutex);
	//m_eventStarted = true;
	//m_lastEventThread = pthread_self();
	return attachThread(env);
}

void COnyxEventHandler::safeDetach()
{
	if(m_jniThreadChanged)
		g_vm->DetachCurrentThread();
	//m_eventStarted = false;
	//m_waitForEvent = false;
	pthread_mutex_unlock(&m_eventMutex);
}
