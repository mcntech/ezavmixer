#ifndef _EVENT_HANDLER_H_
#define _EVENT_HANDLER_H_

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "[app]", __VA_ARGS__)

#include <pthread.h>
#include <jni.h>
#include "PublishEventBase.h"

class COnyxEvents : public CPublishEventBase {
public:	
	COnyxEvents(){};
	COnyxEvents(JNIEnv* env,jobject javaReceiver);
	~COnyxEvents(){
	}
	bool attachThread(JNIEnv** env);
	bool safeAttach(JNIEnv** env);
	void safeDetach();

  	bool onNativeMessage(char *szTitle, char *szMsg);
  	bool onRemoteNodeError(char *url, char *szErr);
  	bool onConnectRemoteNode(char *url);
  	bool onDisconnectRemoteNode(char *url);
  	bool onStatusRemoteNode(char *url, char *szMsg);

  	jclass m_deviceClass;
		
private:
	bool m_jniThreadChanged;
	pthread_mutex_t m_eventMutex;
	static int printlog(const char* tag, const char* msg, va_list args);
};
#endif // _EVENT_HANDLER_H_
