
#ifndef _SERVER_NODE__H_
#define _SERVER_NODE__H_
#ifdef WIN32
#include <winsock2.h>
#else // Linux
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef PLATFORM_ANDROID
//#include <execinfo.h>
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

class CStrmCtx
{
public:
    CStrmCtx() {m_ulFlags = 0; m_llPts = 0;}
    unsigned long   m_ulFlags;
    long long       m_llPts;
};

class CUdpServerNode : public CServerNodeBase
{
public:
	CUdpServerNode(CUdpClntBridge *pUdpClntBridge);
	void start();
	void stop();
	int subscribeStream(int nStrmId);
	int unsubscribeStream(int nStrmId);
	int subscribeProgram(int nStrmId);
	int unsubscribeProgram(int nStrmId);

	int getData(int nStrmId, char *pData, int numBytes);

	long long getPts(int nStrmId);
	long long getClkUs(int nStrmId);
	int getCodecType(int nStrmId);
	int getNumAvailFrames(int nStrmId);

	int getStatus(std::string &status);

public:
	CUdpClntBridge *m_pClntBridge;

	std::map<int, ConnCtxT *> m_Connections;
    std::map<int, CStrmCtx *> m_StrmCtxs;
};


#endif
