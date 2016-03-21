#ifndef _ONYX_PLAYER_API_H_
#define _ONYX_PLAYER_API_H_

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "[app]", __VA_ARGS__)

#include <pthread.h>
#include <PlayerEventBase.h>
#include <jni.h>

class COnyxPlayerEvents : public CPlayerEventBase
{
public:	
	COnyxPlayerEvents(JNIEnv* env,jobject javaReceiver);

	bool attachThread(JNIEnv** env);
	bool safeAttach(JNIEnv** env);
	void safeDetach();

  	bool onNativeMessage(char *szTitle, char *szMsg);
  	bool onRemoteNodeError(char *url, char *szErr);
  	bool onConnectRemoteNode(char *url);
  	bool onDisconnectRemoteNode(char *url);
  	bool onStatusRemoteNode(char *url, char *szMsg);
  	bool onRtspServerStatus(const char *szPublishId, int nState, int nStrmInTime, int nStrmOutTime, int nLostBufferTime);

  	jclass m_deviceClass;

private:
	bool m_jniThreadChanged;
	pthread_mutex_t m_eventMutex;
	static int printlog(const char* tag, const char* msg, va_list args);
};
#endif
