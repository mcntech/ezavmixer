
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

#include "UdpClntBridge.h"
#include "JdDbg.h"
#include "strmconn.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <signal.h>
#include "ServerNodeBase.h"
#include <map>

typedef std::map<std::string, CServerNodeBase *> ServerNodeMap;

class CUdpServerNode : public CServerNodeBase
{
public:
	CUdpServerNode(CUdpClntBridge *pUdpClntBridge);
	void start();
	void stop();
	int subscribeStream(int nStrmId);
	int unsubscribeStream(int nStrmId);
	int getData(int nStrmId, char *pData, int numBytes);

	long long getPts(int nStrmId);
	long long getClkUs();
	int getCodecType(int nStrmId);
	int getNumAvailFrames(int nStrmId);

	int getStatus(std::string &status);

public:
	CUdpClntBridge *m_pClntBridge;
	long long       m_llAudPts;
	unsigned long   m_ulAudFlags;
	long long       m_llVidPts;
	unsigned long   m_ulVidFlags;
};


#endif
