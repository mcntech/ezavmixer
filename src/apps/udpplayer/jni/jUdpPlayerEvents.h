#ifndef _UDP_PLAYER_API_H_
#define _UDP_PLAYER_API_H_

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "[app]", __VA_ARGS__)

#include <pthread.h>
#include "PlayerEventBase.h"
#include <jni.h>
#include "UdpCallback.h"

class CUdpPlayerEvents : public CPlayerEventBase
{
public:	
	CUdpPlayerEvents(JNIEnv* env,jobject javaReceiver);

	bool attachThread(JNIEnv** env);
	bool safeAttach(JNIEnv** env);
	void safeDetach();

  	bool onServerStatistics(const char *szPublishId, UDP_SERVER_STATS *pStat);
  	bool onPsiPatChange(const char *szPublishId, const char *pPsiData);
	bool onPsiPmtChange(const char *szPublishId, int strmId, const char *pPsiData);
	bool onFormatChange(const char *url, int strmId, const char *pFormat);

  	jclass m_deviceClass;

private:
	bool m_jniThreadChanged;
	pthread_mutex_t m_eventMutex;
	static int printlog(const char* tag, const char* msg, va_list args);
};
#endif
