#ifndef _UDP_PLAYER_API_H_
#define _UDP_PLAYER_API_H_

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "[app]", __VA_ARGS__)

#include <pthread.h>
#include <PlayerEventBase.h>
#include <jni.h>
#include "UdpCallback.h"

class CUdpPlayerEvents : public CPlayerEventBase
{
public:	
	CUdpPlayerEvents(JNIEnv* env,jobject javaReceiver);

	bool attachThread(JNIEnv** env);
	bool safeAttach(JNIEnv** env);
	void safeDetach();

 	bool onDiscoverRemoteNode(char *host);

  	bool onNativeMessage(char *szTitle, char *szMsg);
  	bool onRemoteNodeError(char *url, char *szErr);
  	bool onConnectRemoteNode(char *url);
  	bool onDisconnectRemoteNode(char *url);
  	bool onStatusRemoteNode(char *url, char *szMsg);
  	bool onServerStatus(const char *szPublishId, int nState, int nStrmInTime, int nStrmOutTime, int nLostBufferTime);
  	bool onServerStatistics(const char *szPublishId, UDP_SERVER_STATS *pStat);

  	jclass m_deviceClass;

private:
	bool m_jniThreadChanged;
	pthread_mutex_t m_eventMutex;
	static int printlog(const char* tag, const char* msg, va_list args);
};
#endif
