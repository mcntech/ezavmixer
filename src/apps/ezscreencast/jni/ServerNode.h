
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
	virtual void start(){}
	virtual void stop(){}
};

typedef std::map<std::string, CServerNode *> ServerNodeMap;

class CRtspServerNode : public CServerNode
{
public:
	CRtspServerNode(std::string url, std::string appName, int localRtpPort, int remoteRtpPort, int serverPort)
	:m_Config(url.c_str(), appName.c_str(), localRtpPort, remoteRtpPort, serverPort){
	}
	void start();
	void stop();

public:
	CRtspPublishConfig m_Config;
	CRtspPublishBridge *m_pRtspPublishBridge;
};

class CS3PublishNode : public CServerNode
{
public:
	CS3PublishNode(	std::string szHost, std::string szAccesId, std::string szSecKey,
			std::string szBucket, std::string szFolder, std::string szFilePefix)
	{
		m_AwsContext.defaultHostM = szHost;
		m_AwsContext.idM = szAccesId;
		m_AwsContext.secretKeyM = szSecKey;
		m_szBucket = szBucket;
		m_szFolder = szFolder;
		m_szFilePefix = szFilePefix;
	}
	void start(COutputStream *pOutputStream){}
	void stop(){}

public:
	CJdAwsContext m_AwsContext;

	std::string m_szBucket;
	std::string m_szFolder;
	std::string m_szFilePefix;
};

#endif
