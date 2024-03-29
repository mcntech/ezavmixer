#ifndef _PLAYER_EVENT_BASE_H_
#define _PLAYER_EVENT_BASE_H_

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "[app]", __VA_ARGS__)

#include <pthread.h>
#include <jni.h>

class CPlayerEventBase
{
public:	
  	virtual bool onNativeMessage(char *szTitle, char *szMsg) = 0;
  	virtual bool onRemoteNodeError(char *url, char *szErr)  = 0;
  	virtual bool onConnectRemoteNode(char *url)  = 0;
  	virtual bool onDisconnectRemoteNode(char *url)  = 0;
  	virtual bool onStatusRemoteNode(char *url, char *szMsg)  = 0;
};
#endif
