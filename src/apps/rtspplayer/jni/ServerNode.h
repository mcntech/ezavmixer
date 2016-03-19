
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
	virtual void start(COutputStream *pOutputStream) = 0;
	virtual void stop() = 0;
};

typedef std::map<std::string, CServerNode *> ServerNodeMap;

class CRtspServerNode : public CServerNode
{
public:
	CRtspServerNode(std::string url, std::string appName)
	//:m_Config(url.c_str(), appName.c_str(), localRtpPort, remoteRtpPort, serverPort)
{
		//m_pRtspCommonCfg = new CRtspCommonConfig(/*TODO*/NULL, NULL,1,1,1);
		m_pRtspClntBridge = new CRtspClntBridge(pInputStream->pszInputUri, pInputStream->fEnableAud, pInputStream->fEnableVid, &nResult);
	}
	void start(COutputStream *pOutputStream);
	void stop();

public:
	//CRtspPublishConfig m_Config;
	CRtspClntBridge *m_pRtspClntBridge;
	//CRtspCommonConfig  *m_pRtspCommonCfg;
};


#endif