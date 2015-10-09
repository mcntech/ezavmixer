#ifndef _EVENT_HANDLER_H_
#define _EVENT_HANDLER_H_

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "[app]", __VA_ARGS__)

#include <pthread.h>
#include <jni.h>
#include "MultiPublishClnt.h"

class COnyxEventHandler : public CMultiPublish {
public:	
	COnyxEventHandler(){};
	COnyxEventHandler(JNIEnv* env,jobject javaReceiver);
	~COnyxEventHandler(){
	}
	bool attachThread(JNIEnv** env);
	bool safeAttach(JNIEnv** env);
	void safeDetach();

	jobject createJDevice(JNIEnv* env);
  	bool onNativeMessage(char *szTitle, char *szMsg);
  	bool onRemoteNodeError(char *url, char *szErr);
  	bool onRemoteNodePlayStarted();

  	jclass m_deviceClass;
		
private:
	bool m_jniThreadChanged;
	pthread_mutex_t m_eventMutex;
	static int printlog(const char* tag, const char* msg, va_list args);
};
#endif // _EVENT_HANDLER_H_
