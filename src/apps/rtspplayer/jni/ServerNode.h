
#ifndef _SERVER_NODE__H_
#define _SERVER_NODE__H_
#ifdef WIN32
#include <winsock2.h>
#else // Linux
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef PLATFORM_ANDROID
#include <execinfo.h>
#endif

#endif
#include <assert.h>
#include "minini.h"

#include "JdRtspSrv.h"
#include "JdRtspClntRec.h"
#include "RtspConfigure.h"
#include "JdDbg.h"
#include "strmconn.h"
#include "strmconn_ipc.h"
#include "strmconn_zmq.h"
#include "sock_ipc.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <signal.h>

#include "MediaSwitch.h"
#include "RtspPublishBridge.h"
#include "ConfigBase.h"
#include "JdAwsContext.h"

class CServerNode
{
public:
	CServerNode(){}
	virtual void start() = 0;
	virtual void stop() = 0;
};

typedef std::map<std::string, CServerNode *> ServerNodeMap;

class CRtspServerNode : public CServerNode
{
public:
	CRtspServerNode(CRtspClntBridge *pRtspClntBridge);
	void start();
	void stop();
	int getVideoData(char *pData, int numBytes);
	int getAudioData(char *pData, int numBytes);

	long long getVideoPts();
	long long getAudioPts();
	int getAudioCodecType();
	int getAudioCodecType();
	int getStatus(std::string &status);

public:
	CRtspClntBridge *m_pRtspClntBridge;
	long long       m_llAudPts;
	unsigned long   m_ulAudFlags;
	long long       m_llVidPts;
	unsigned long   m_ulVidFlags;
};


#endif
