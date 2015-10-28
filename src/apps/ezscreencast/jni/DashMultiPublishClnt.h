
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

	CConfigBase *m_pConfig;
	static CPublishClntBase *openInstance(CPublishEventBase *pEventBase);
	//void closeInstancce(CPublishClntBase *pInst);

	int start();
	int stop();

private:
	int StartMpdServer(const char *pszConfFile);

public:
	COutputStream      *m_pOutputStream;
	CMpdSrvBridge      *m_pMpdSrvBridge;
	CMpdRoot           *m_pMpdRoot;
	CPublishEventBase  *m_EventCallback;
};

#endif
