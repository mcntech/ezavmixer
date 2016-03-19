#include "jOnyxPlayerEvents.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <android/log.h>

//global context must be maintained for these variable to be valid when scope changes
static JavaVM* g_vm;
static jobject g_jniGlobalSelf = NULL;
static jclass g_deviceClass = NULL;

CjOnyxPlayerEvents::CjOnyxPlayerEvents(JNIEnv* env, jobject javaReceiver){
	//do not share global jvm objects from JNI_OnLoad. get/share local jvm from env instead
	env->GetJavaVM(&g_vm);
	g_jniGlobalSelf = env->NewGlobalRef(javaReceiver);
	g_deviceClass = env->FindClass("com/mcntech/rtspplayer/Device");
	g_deviceClass = (jclass)env->NewGlobalRef(g_deviceClass);

	pthread_mutex_init( &m_eventMutex, NULL);

	Clogger::redirect(&printlog);//enable MIDSLOG
}

int CjOnyxPlayerEvents::printlog(const char *tag, const char *msg, va_list args)
{
	__android_log_vprint(ANDROID_LOG_DEBUG  , "[mcn_rtsp]", msg, args);
	return 0;
}

jobject CjOnyxPlayerEvents::createJDevice(JNIEnv* env, const TDEVICE_INFO& info){
	jthrowable exc = 0;

	if(g_deviceClass == 0){//assigned when app was created to keep thread context
		MIDSLOG("", "CjniEventHandler::createJDevice 1");		
		g_deviceClass = env->FindClass("com/mcntech/rtspplayer/Device"");
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
	return jnode;
}


bool CjOnyxPlayerEvents::attachThread(JNIEnv** env){
  bool changed = false;
  switch (g_vm->GetEnv((void**)env, JNI_VERSION_1_6))
  {
  	case JNI_OK:
  		//MIDSLOG("", "attachThread: already attached");
    	break;
  	case JNI_EDETACHED:
	    if (g_vm->AttachCurrentThread(env, NULL)!=0)
	    {
	    	(*env)->ExceptionDescribe(); // write exception data to the console 
	      	(*env)->ExceptionClear();    // clear the exception that was pending 
	      	break;
	   	}
   		//MIDSLOG("", "attachThread: Attached to current thread");
    	changed = true;
    	break;
	  case JNI_EVERSION:
	    MIDSLOG("", "attachThread: Invalid java version");
    	break;
	  default:
		break;
  }
  return m_jniThreadChanged= changed;	
}

bool CjOnyxPlayerEvents::safeAttach(JNIEnv** env){
	if(*env == NULL){
		JNIEnv envalloc;
		*env = &envalloc;
	}

	m_eventStarted = true;
	m_lastEventThread = pthread_self();

	return attachThread(env);
}

void CjOnyxPlayerEvents::safeDetach()
{
	if(m_jniThreadChanged)
		g_vm->DetachCurrentThread();
	m_eventStarted = false;	
	m_waitForEvent = false;
	//pthread_mutex_unlock(&m_eventMutex);
}

void CjOnyxPlayerEvents::onStartPlay()
{
	JNIEnv* env;
	safeAttach(&env);
	jclass cls = env->GetObjectClass(g_jniGlobalSelf);

	jmethodID midOnStartPlay = env->GetStaticMethodID(cls, "onStartPlay", "()V");
	if(midOnStartPlay)
		env->CallStaticVoidMethod(cls, midOnStartPlay);

	safeDetach();
	//pthread_mutex_unlock(&g_mutex);
}

void  CjOnyxPlayerEvents::onStopPlay()
{
	JNIEnv* env;
	safeAttach(&env);
	jclass cls = env->GetObjectClass(g_jniGlobalSelf);

	jmethodID midOnStopPlay = env->GetStaticMethodID(cls, "onStopPlay", "()V");
	if(midOnStopPlay)
		env->CallStaticVoidMethod(cls, midOnStopPlay);

	safeDetach();
}
