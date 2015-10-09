
#ifndef _MULTI_PUBLISH_CLNT_H_
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
#include "uimsg.h"
#include "ServerNode.h"

class CMultiPublish
{
public:
	CConfigBase *m_pConfig;
	static 	CMultiPublish *getInstance();
	static 	void closeInstancce(CMultiPublish *pInst);

	int AddPublishServer(std::string url, std::string appName, int localRtpPort, int remoteRtpPort, int serverPort);
	int RemovePublishServer(std::string url);

	int sendAudioData(const char *pData, int numBytes, long Pts, int Flags);
	int sendVideoData(const char *pData, int numBytes, long Pts, int Flags);

	int start();
	int stop();

public:
	CMultiPublish();
	CMultiPublish(CConfigBase *pConfig);
	CMediaSwitch       *m_pPublishSwitch;
	COutputStream      *m_pOutputStream;
	ServerNodeMap      m_PublishServerList;
};

#endif
