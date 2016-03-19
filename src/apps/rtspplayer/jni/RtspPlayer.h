
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

#include "ConfigBase.h"
#include "uimsg.h"
#include "ServerNode.h"
#include "PlayerBase.h"
#include "PlayerEventBase.h"

class CRtspPlayer : public CPlayerBase
{
public:
	CConfigBase *m_pConfig;
	static CPublishClntBase *openInstance(CPublishEventBase *pEventBase);
	//void closeInstancce(CPublishClntBase *pInst);

	int AddServer(std::string url);
	int RemoveServer(std::string url);

	int getAudioData(std::string url, const char *pData, int numBytes, long Pts, int Flags);
	int getVideoData(std::string url, const char *pData, int numBytes, long Pts, int Flags);

	int start(std::string url);
	int stop(std::string url);

public:
	CRtspPlayer(CPublishEventBase *pEventBase);
	CRtspMultiPublishClnt(CConfigBase *pConfig);
	CMediaSwitch       *m_pPublishSwitch;
	COutputStream      *m_pOutputStream;
	ServerNodeMap      m_ServerList;
	ConnCtxT           *m_pAudConnSrc;
	ConnCtxT           *m_pVidConnSrc;
	CPublishEventBase  *m_EventCallback;
private:
	CServerNode *getServerNode(std::string url);
};

#endif
