
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

	int AddPublishServer(std::string url, std::string appName, int localRtpPort=0, int remoteRtpPort=0, int serverPort=554);
	int RemovePublishServer(std::string url);

	int sendAudioData(const char *pData, int numBytes, long Pts, int Flags);
	int sendVideoData(const char *pData, int numBytes, long Pts, int Flags);

	int start();
	int stop();

public:
	CRtspPlayer(CPublishEventBase *pEventBase);
	CRtspMultiPublishClnt(CConfigBase *pConfig);
	CMediaSwitch       *m_pPublishSwitch;
	COutputStream      *m_pOutputStream;
	ServerNodeMap      m_PublishServerList;
	ConnCtxT           *m_pAudConnSrc;
	ConnCtxT           *m_pVidConnSrc;
	CPublishEventBase  *m_EventCallback;
};

#endif
