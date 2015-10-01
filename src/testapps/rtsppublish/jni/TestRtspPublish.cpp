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

#ifdef HAS_PLUGIN_RTSPSRV
#include "JdRtspSrv.h"
#endif

#ifdef HAS_PLUGIN_RTSPCLTNREC
#include "JdRtspClntRec.h"
#endif
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

class CTestRtspPublish
{
public:
	CTestRtspPublish(CConfigBase *pConfig);
	unsigned short m_usRtpLocalPort;
	unsigned short m_usRtpRemotePort;
	unsigned short m_usServerRtspPort;

	CRtspPublishBridge *m_pRtspSrvBridge;
	CRtspCommonConfig  *m_pRtspCommonCfg;
	CMediaSwitch       *m_pPublishSwitch;
	COutputStream      *m_pOutputStream;
	std::string        m_szRtspServerAddr;
	std::string        m_szApplicationName;
	CRtspPublishConfig *m_pRtspPublishCfg;
public:
	CConfigBase *m_pConfig;
	int run();

};
CTestRtspPublish::CTestRtspPublish(CConfigBase *pConfig)
{
	m_pConfig = pConfig;
	m_szRtspServerAddr = pConfig->gets(SECTION_RTSP_PUBLISH, KEY_RTSP_PUBLISH_HOST, "");
	m_szApplicationName = pConfig->gets(SECTION_RTSP_PUBLISH, KEY_RTSP_PUBLISH_APPLICATION, "");
	m_usRtpLocalPort =  (unsigned short)pConfig->getl(SECTION_RTSP_PUBLISH, "local_port");
	m_usRtpRemotePort =  (unsigned short)pConfig->getl(SECTION_RTSP_PUBLISH, "remote_port");
	m_usServerRtspPort =  (unsigned short)pConfig->getl(SECTION_RTSP_PUBLISH, KEY_RTSP_PUBLISH_RTSP_PORT);
}


CRtspPublishBridge  *m_pRtspSrvBridge;
int CTestRtspPublish::run()
{
	char szSwitchIdSection[128];
	int i = 0;
	sprintf(szSwitchIdSection,"%s%d",SWITCH_PREFIX, i);
	m_pRtspPublishCfg = new CRtspPublishConfig(m_szRtspServerAddr.c_str(), m_szRtspServerAddr.c_str(), m_usRtpLocalPort, m_usRtpRemotePort, m_usServerRtspPort);
	m_pRtspSrvBridge = new CRtspPublishBridge;
	m_pOutputStream = new COutputStream("test");
	m_pRtspCommonCfg = new CRtspCommonConfig(m_pConfig);
	CMediaSwitch *pPublishSwitch = new CMediaSwitch(szSwitchIdSection);

	m_pRtspSrvBridge->SetStreamCfg(m_pRtspCommonCfg);
	m_pRtspSrvBridge->Init(m_pOutputStream);

	m_pRtspSrvBridge->SetPublishServerCfg(m_pRtspPublishCfg);
	m_pRtspSrvBridge->ConnectToPublishServer();

	if(m_pRtspSrvBridge)
		pPublishSwitch->AddOutput(m_pRtspSrvBridge);
}

int main()
{	CConfigBase *pConfig = new CConfigBase;
	CTestRtspPublish TestRtspPublish(pConfig);
	TestRtspPublish.run();
}
