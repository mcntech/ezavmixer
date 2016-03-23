
#ifndef _RTSP_MULTI_PUBLISH_CLNT_H_
#define _RTSP_MULTI_PUBLISH_CLNT_H_

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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <signal.h>

#include "MediaSwitch.h"

#include "RtspPublishBridge.h"
#include "ConfigBase.h"
#include "uimsg.h"
#include "ServerNode.h"
#include "PublishClntBase.h"
#include "PublishEventBase.h"
class CPublishEventBase;

class CRtspMultiPublishClnt : public CPublishClntBase
{
public:

	static CPublishClntBase *openInstance(CPublishEventBase *pEventBase);
	//void closeInstancce(CPublishClntBase *pInst);
	int CreateRtspPublishStream(const char *szId,  const char *szSwitchId);
	int AddPublishServer(std::string url, std::string appName, int localRtpPort=0, int remoteRtpPort=0, int serverPort=554);
	int RemovePublishServer(std::string url);

	int sendAudioData(const char *pData, int numBytes, long Pts, int Flags);
	int sendVideoData(const char *pData, int numBytes, long Pts, int Flags);

	void enableRtspLocalServer(const char *szId, const char *szInterfaceName, const char *szStreamName, int nPort, bool fEnableMux);

	int start();
	int stop();

public:
	CRtspMultiPublishClnt(CPublishEventBase *pEventBase);
	CRtspMultiPublishClnt(CConfigBase *pConfig);
	CMediaSwitch       *m_pPublishSwitch;
	COutputStream      *m_pOutputStream;
	ServerNodeMap      m_PublishServerList;
	ConnCtxT           *m_pAudConnSrc;
	ConnCtxT           *m_pVidConnSrc;
	CPublishEventBase  *m_EventCallback;
	CRtspSrvConfig     *m_pRtspSrvConfig;
	CConfigBase        *m_pConfig;
	CRtspPublishBridge *m_pRtspSrvBridge;
};

#endif
