#ifndef __STRM_SOURCE_H__
#define __STRM_SOURCE_H__
#include "strmconn_zmq.h"
typedef enum MP4SRC_EVENT_T
{
	MP4SRC_EVENT_EOS
};

typedef void (*fnEventCallback_t) (int nEventId, void *pEventData);
class CStrmSourceIf
{
public:
	virtual ~CStrmSourceIf(){}
	virtual void Stop() = 0;
	virtual void Run(long long llTime) = 0;
	virtual int Load(const char *pszFileName) = 0;
	virtual void SetIpcParam(ConnCtxT *pAudConn, ConnCtxT *pVidConn) = 0;
	virtual int GetDuration() = 0;
	virtual void SetStartPts(long long llPtsMs) = 0;
	virtual int SetEventCallback(fnEventCallback_t pCallback) = 0;
};

CStrmSourceIf *CreateMp4Source();
void DeleteMp4Source(CStrmSourceIf *);

#endif  //__STRM_SOURCE_H__