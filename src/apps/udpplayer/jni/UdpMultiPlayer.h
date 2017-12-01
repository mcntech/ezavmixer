
#ifndef __RTSP_MULTI_PLAYER_H_
#define __RTSP_MULTI_PLAYER_H_

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


#include "UdpCallback.h"
#include "strmconn.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <signal.h>

#include "ServerNodeUdp.h"
#include "PlayerBase.h"
#include "PlayerEventBase.h"

class CUdpMultiPlayer : public CPlayerBase, public CUdpServerCallback
{
public:
	static CPlayerBase *openInstance(CPlayerEventBase *pEventBase);
	int getStatus(std::string url, std::string &status);
	int addServer(std::string url);
	int removeServer(std::string url);

	int subscribeStream(std::string url, int substrmId);
	int unsubscribeStream(std::string url, int substrmId);
    int  subscribeProgram(std::string url,  int substrmId);
    int  unsubscribeProgram(std::string url,  int substrmId);


	int getData(std::string url, int substrmId, char *pData, int numBytes);

	long long getPts(std::string url, int substrmId);
	long long getClkUs(std::string url);

	virtual int  getCodecType(std::string url, int substrmId);

	int  getNumAvailFrames(std::string url, int substrmId);
	int startServer(std::string url);
	int stopServer(std::string url);

	void UpdateStats(const char *url, UDP_SERVER_STATS *);
	void NotifyPsiPatChange(const char *url, const char *pPsiData);
	void NotifyPsiPmtChange(const char *url, int strmId, const char *pPsiData);

public:
	CUdpMultiPlayer(CPlayerEventBase *pEventBase);
	ServerNodeMap      m_ServerList;
	CPlayerEventBase  *m_EventCallback;

private:
	CServerNodeBase *getServerNode(std::string url);
	void remServerNode(std::string url);
};

#endif
