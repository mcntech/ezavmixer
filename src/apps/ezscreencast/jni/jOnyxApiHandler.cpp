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

jobject COnyxEventHandler::createJDevice(JNIEnv* env){
	jthrowable exc = 0;
	if(g_deviceClass == 0){//assigned when app was created to keep thread context
		g_deviceClass = env->FindClass("com/bfrx/fcvsrc/Device");
		exc = env->ExceptionOccurred();
		if(exc){
			env->ExceptionDescribe(); // write exception data to the console 
	    	env->ExceptionClear();    // clear the exception that was pending 
	    	return NULL;
	    }
		if(g_deviceClass == 0)
			return NULL;
		
		g_deviceClass = (jclass)env->NewGlobalRef(g_deviceClass);
		env->ExceptionOccurred();	
		if(exc){
			env->ExceptionDescribe(); 
    		env->ExceptionClear(); 
    		return NULL;
    	}     		
		//return NULL;
    }
    
	jfieldID labelID = env->GetFieldID(g_deviceClass, "label","Ljava/lang/String;");
	env->ExceptionOccurred();	
	if(exc){
		env->ExceptionDescribe(); 
		env->ExceptionClear(); 
		return NULL;
	}   
    if(labelID == NULL)
		return NULL;
	jfieldID roleField = env->GetFieldID(g_deviceClass, "role","I");
	jfieldID deviceInfoField = env->GetFieldID(g_deviceClass, "deviceInfo","Ljava/lang/String;");
	env->ExceptionOccurred();	
	if(exc){
		env->ExceptionDescribe(); 
		env->ExceptionClear(); 
		return NULL;
	}   
      
	jmethodID nodeConstructor = env->GetMethodID(g_deviceClass, "<init>", "()V");
	env->ExceptionOccurred();	
	if(exc){
		env->ExceptionDescribe(); 
		env->ExceptionClear(); 
		return NULL;
	}      
	if(nodeConstructor == NULL)
		return NULL;
	jobject jnode = env->NewObject(g_deviceClass,nodeConstructor);
	env->ExceptionOccurred();	
	if(exc){
		env->ExceptionDescribe(); 
		env->ExceptionClear(); 
		return NULL;
	}      
	if(jnode == NULL)
		return NULL;
	//make sure to match the method to the right jtype or values can get overwritten
	env->SetIntField(jnode, roleField,0);
	
	char deviceInfo[256];
	deviceInfo[0] = 0;
	// TODO info.getMfiInfo(deviceInfo,256);
	if(deviceInfo[0] != '\0' && strcmp(deviceInfo,"") != 0){
		env->SetObjectField(jnode, deviceInfoField, env->NewStringUTF(deviceInfo));
	}
	return jnode;
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

	jmethodID callback = env->GetStaticMethodID(onyxApi, "onRemoteNodeError", "(Ljava/lang/Object;)V");
	env->CallStaticVoidMethod(onyxApi, callback,szErr);
	env->DeleteLocalRef(jurl);
	env->DeleteLocalRef(jmsg);

	safeDetach();
	return true;
}

bool COnyxEventHandler::onRemoteNodePlayStarted()
{
	JNIEnv* env;
	safeAttach(&env);
    jclass onyxApi = env->GetObjectClass(g_jniGlobalSelf);
	jmethodID callback = env->GetStaticMethodID(onyxApi, "onRemoteNodePlayStarted", "(J)V");
	env->CallStaticVoidMethod(onyxApi, 0);
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
