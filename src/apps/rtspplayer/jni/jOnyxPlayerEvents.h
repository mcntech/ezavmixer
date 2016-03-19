#ifndef _ONYX_PLAYER_API_H_
#define _ONYX_PLAYER_API_H_

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "[app]", __VA_ARGS__)

#include <pthread.h>
#include <PlayerEventBase.h>
#include <jni.h>

class CjOnyxPlayerEvents : public CPlayerEventsBase {
public:	
	CjOnyxPlayerEvents(){};
	CjOnyxPlayerEvents(JNIEnv* env,jobject javaReceiver);
	~CjOnyxPlayerEvents(){
	}
	bool attachThread(JNIEnv** env);
	bool safeAttach(JNIEnv** env);
	void safeDetach();
	jobject createJDevice(JNIEnv* env, const TDEVICE_INFO& info);
 	jclass m_deviceClass;

 	void onStartPlay();
 	void onStopPlay();
private:
	bool m_jniThreadChanged;
	pthread_mutex_t m_eventMutex;
	static int printlog(const char* tag, const char* msg, va_list args);
};
#endif
