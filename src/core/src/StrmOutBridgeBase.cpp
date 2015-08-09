#include "StrmOutBridgeBase.h"
#include <string.h>
CStrmOutBridge::CStrmOutBridge(const char *szId)
{
	strncpy(m_szId, szId, MAX_ID_SIZE - 1);
	m_nState = 0;
	m_nVidStrmTime = 0;
	m_nAudStrmTime = 0;
	m_nErrors = 0;
}
