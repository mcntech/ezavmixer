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
