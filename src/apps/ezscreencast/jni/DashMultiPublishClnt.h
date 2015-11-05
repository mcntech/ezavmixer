
#ifndef _DASH_MULTI_PUBLISH_CLNT_H_
#define _DASH_MULTI_PUBLISH_CLNT_H_

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

#include "MpdSrvBridge.h"
#include "Mpd.h"

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
#include "PublishClntBase.h"
#include "PublishEventBase.h"
#include "StreamUtil.h"

class CPublishEventBase;
class CDashMultiPublishClnt : public CPublishClntBase
{
public:
	CDashMultiPublishClnt(CPublishEventBase *pEventBase);
	CDashMultiPublishClnt(CConfigBase *pConfig);
	CDashMultiPublishClnt(){m_pConfig=NULL;}
	int CreateMpd(std::string szId);
	int CreatePeriod(std::string szId, std::string szperiodId);
	int CreateAdaptationSet(std::string szmpdId, std::string szperiodId, std::string szadaptId);
	int CreateRepresentation(std::string szmpdId, std::string szperiodId, std::string szadaptId, std::string szrepId);
	CMpdRepresentation * FindRepresentation(std::string szmpdId, std::string szperiodId, std::string szadaptId, std::string szrepId);

	int CreateMpdPublishStream(std::string szId, CMpdRoot  *pMpdRoot, CMediaSwitch *pPublishSwitch, CMpdRepresentation *pRepresentation, CS3PublishNode *pServerNode);
	int CreateMpdPublishStream(std::string szId, std::string szmpdId, std::string szperiodId, std::string szadaptId, std::string szrepId, std::string strSwitchId, std::string strServerNode);

	CConfigBase *m_pConfig;
	static CPublishClntBase *openInstance(CPublishEventBase *pEventBase);
	//void closeInstancce(CPublishClntBase *pInst);

	int SatrtMpdPublishStream(std::string szPublishId);
	int start();
	int stop();

private:
	CMpdRoot *getMpd(std::string szmpdId);

public:
	std::map <std::string, CMpdRoot *>  m_listMpd;
	COutputStream      *m_pOutputStream;
	CMpdSrvBridge      *m_pMpdSrvBridge;
	CPublishEventBase  *m_EventCallback;
};

typedef std::map <std::string, CMpdRoot *> MpdRootMap;
#endif
