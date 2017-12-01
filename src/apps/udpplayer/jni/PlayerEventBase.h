#ifndef _PLAYER_EVENT_BASE_H_
#define _PLAYER_EVENT_BASE_H_

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "[app]", __VA_ARGS__)

#include <pthread.h>
#include <jni.h>

class CPlayerEventBase
{
public:	
  	virtual bool onPsiPatChange(const char *szPublishId, const char *pPsiData) = 0;
	virtual bool onPsiPmtChange(const char *szPublishId, int strmId, const char *pPsiData) = 0;
};
#endif
