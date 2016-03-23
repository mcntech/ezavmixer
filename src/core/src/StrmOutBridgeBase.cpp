#include "StrmOutBridgeBase.h"
#include <string.h>
CStrmOutBridge::CStrmOutBridge(const char *szId)
{
	m_szId = szId;
	m_nState = 0;
	m_nVidStrmTime = 0;
	m_nAudStrmTime = 0;
	m_nErrors = 0;
}

CRtspSrvConfig::CRtspSrvConfig(const char *_network_interface, const char *_szStreamName, int _port, int _fEnableMux)
{
	strncpy(network_interface, _network_interface, 128 - 1);
	strncpy(szStreamName, _szStreamName, 128 - 1);
	usServerRtspPort = _port;
	fEnableMux = _fEnableMux;
}
