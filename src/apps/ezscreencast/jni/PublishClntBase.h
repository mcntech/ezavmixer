
#ifndef _PUBLISH_CLNT_BASE_H_
#define _PUBLISH_CLNT_BASE_H_

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
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <signal.h>

#include "MediaSwitch.h"

#include "RtspPublishBridge.h"
#include "ConfigBase.h"
#include "uimsg.h"
#include "ServerNode.h"
#include "StreamUtil.h"

class CPublishClntBase
{
public:
	virtual int Configure(std::string env){ return 0;}
	virtual int GetStatus(std::string &status){ return 0;}

	virtual int AddPublishServer(std::string url, std::string appName, int localRtpPort=0, int remoteRtpPort=0, int serverPort=554);
	virtual int RemovePublishServer(std::string url);

	virtual int start() = 0;
	virtual int stop() = 0;

	int CreateSwitch(const char *pszSwitchId);
	int CreateInputStrm(const char *szInputId, const char *szInputType, const char *szInputUri);
	int ConnectSwitchInput(const char *szInputId, const char *pszSwitchId);

protected:
	std::map <std::string, CMediaSwitch *>  m_listPublishSwitches;
	std::map <std::string, CAvmixInputStrm *>  m_listInputStrmConn;
	std::map <std::string, CStrmOutBridge *>  m_listPublishServer;

};

#endif // _PUBLISH_CLNT_BASE_H_
