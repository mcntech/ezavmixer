
#ifndef __RTSP_MULTI_PLAYER_H_
#define __RTSP_MULTI_PLAYER_H_

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


#include "JdRtspClnt.h"
#include "JdDbg.h"
#include "strmconn.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <signal.h>

#include "ServerNode.h"
#include "PlayerBase.h"
#include "PlayerEventBase.h"

class CRtspMultiPlayer : public CPlayerBase
{
public:
	static CPlayerBase *openInstance(CPlayerEventBase *pEventBase);
	int getStatus(std::string url, std::string &status);
	int addServer(std::string url);
	int removeServer(std::string url);

	int getAudioData(std::string url, char *pData, int numBytes);
	int getVideoData(std::string url, char *pData, int numBytes);

	long long getVideoPts(std::string url);
	long long getAudioPts(std::string url);

	virtual int  getVideoCodecType(std::string url);
	virtual int  getAudioCodecType(std::string url);

	int start(std::string url);
	int stop(std::string url);

public:
	CRtspMultiPlayer(CPlayerEventBase *pEventBase);
	ServerNodeMap      m_ServerList;
	CPlayerEventBase  *m_EventCallback;

private:
	CServerNode *getServerNode(std::string url);
};

#endif
