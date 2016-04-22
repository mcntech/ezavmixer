
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

typedef std::map <std::string, CRtspPublishBridge *> RtspPublishMap_T;
typedef std::map <std::string, CMediaSwitch *> MediaSwitch_T;

class CRtspMultiPublishClnt : public CPublishClntBase
{
public:

	static CPublishClntBase *openInstance(CPublishEventBase *pEventBase);
	//void closeInstancce(CPublishClntBase *pInst);
	int CreateRtspPublishBridge(const char *szStreamId);
	void RemoveRtspPublishBridge(const char *szStreamId);
	int AddPublishBridgeToMediaSwitch(const char *szPublishId,  const char *szSwitchId);

	int AddPublishServer(std::string url, std::string szStreamId, int localRtpPort=0, int remoteRtpPort=0, int serverPort=554);
	int RemovePublishServer(std::string url);

	int sendAudioData(const char *szInputId, const char *pData, int numBytes, long Pts, int Flags);
	int sendVideoData(const char *szInputId, const char *pData, int numBytes, long Pts, int Flags);

	void enableRtspLocalServer(const char *szId, const char *szInterfaceName, const char *szStreamName, int nPort, bool fEnableMux);

	int StartRtspPublishBridge(std::string szPublishId);
	int StopRtspPublishBridge(std::string szPublishId);
	int UpdateRtspPublishStatus(std::string szPublishId);

	int StartRtspPublishNode(std::string szPublishUrl);
	int StopRtspPublishNode(std::string szPublishId);

	CRtspPublishBridge *GetRtspPublishBridge(const char *szPublishId);

public:
	CRtspMultiPublishClnt(CPublishEventBase *pEventBase);
	CRtspMultiPublishClnt(CConfigBase *pConfig);
	COutputStream      *m_pOutputStream;
	ServerNodeMap      m_PublishServerList;
	ConnCtxT           *m_pAudConnSrc;
	ConnCtxT           *m_pVidConnSrc;
	CPublishEventBase  *m_EventCallback;
	CRtspSrvConfig     *m_pRtspSrvConfig;
	CConfigBase        *m_pConfig;
	CRtspCommonConfig  *m_pRtspCommonCfg;

	MediaSwitch_T        m_listMediaSwitches;
	RtspPublishMap_T     m_listPublishBridges;
};



#endif
