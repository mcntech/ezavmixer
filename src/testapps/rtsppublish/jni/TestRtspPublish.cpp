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

#include "JdDbg.h"
#include "strmconn.h"
#include "strmconn_ipc.h"
#include "strmconn_zmq.h"
#include "sock_ipc.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <signal.h>

#include "RtspPublishBridge.h"


CRtspPublishConfig::CRtspPublishConfig(const char *pszConfFile)
{
	/*
	ini_gets(SECTION_RTSP_PUBLISH, KEY_RTSP_PUBLISH_HOST, "", szRtspServerAddr, 64, pszConfFile);
	ini_gets(SECTION_RTSP_PUBLISH, KEY_RTSP_PUBLISH_APPLICATION, "", szApplicationName, 64, pszConfFile);
	usRtpLocalPort =  (unsigned short)ini_getl(SECTION_RTSP_PUBLISH, "local_port",   59500, pszConfFile);
	usRtpRemotePort =  (unsigned short)ini_getl(SECTION_RTSP_PUBLISH, "remote_port",   49500, pszConfFile);
	usServerRtspPort =  (unsigned short)ini_getl(SECTION_RTSP_PUBLISH, KEY_RTSP_PUBLISH_RTSP_PORT,   1935, pszConfFile);
	*/
}
CRtspPublishBridge  *m_pRtspSrvBridge;
int main()
{
	m_pRtspSrvBridge = new CRtspPublishBridge;
	m_pRtspCommonCfg = new CRtspCommonConfig(ONYX_PUBLISH_FILE_NAME);

	m_pRtspSrvBridge->SetStreamCfg(m_pRtspCommonCfg);
	m_pRtspSrvBridge->Init(m_pOutputStream);

	if(m_fEnableRtspSrv) {
		m_pRtspSrvBridge->SetRtspServerCfg(m_pRtspSvrCfg);
		m_pRtspSrvBridge->StartRtspServer();
	}
	if(m_fEnableRtspPublish) {
		m_pRtspSrvBridge->SetPublishServerCfg(m_pRtspPublishCfg);
		m_pRtspSrvBridge->ConnectToPublishServer();
	}

	if(m_pRtspSrvBridge)
		pPublishSwitch->AddOutput(m_pRtspSrvBridge);

}
