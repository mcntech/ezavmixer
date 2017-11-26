#include "UdpMultiPlayer.h"
#include "jUdpPlayerEvents.h"
#include "JdDbg.h"

static int modDbgLevel = CJdDbg::LVL_TRACE;
#define TRACE_ENTER 	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Enter", __FUNCTION__));
#define TRACE_LEAVE 	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));


CUdpMultiPlayer::CUdpMultiPlayer(CPlayerEventBase *pEventBase)
{
	m_EventCallback = pEventBase;
}

CServerNodeBase *CUdpMultiPlayer::getServerNode(std::string url)
{
	CServerNodeBase *pNode = NULL;
	//TRACE_ENTER
	ServerNodeMap::iterator it = m_ServerList.find(url);
	if(m_ServerList.end() != it){
		pNode = it->second;
	}
	//TRACE_LEAVE
	return pNode;
}

void CUdpMultiPlayer::remServerNode(std::string url)
{
	TRACE_ENTER
	ServerNodeMap::iterator it = m_ServerList.find(url);
	if(m_ServerList.end() != it){
		m_ServerList.erase(it);
	}
	TRACE_LEAVE
}
int CUdpMultiPlayer::addServer(std::string url)
{
	int nResult = 0;
	TRACE_ENTER
	CUdpClntBridge *pRtspClntBridge = new  CUdpClntBridge(url.c_str(), 1/*pInputStream->fEnableAud*/, 1/*pInputStream->fEnableVid*/, &nResult, this);

	CUdpServerNode *pServerNode = new CUdpServerNode(pRtspClntBridge);
	m_ServerList[url] = pServerNode;
	TRACE_LEAVE
	return 0;
}

int CUdpMultiPlayer::removeServer(std::string url)
{
	TRACE_ENTER
	CUdpServerNode *pNode = (CUdpServerNode *)getServerNode(url);
	if(pNode) {
		if(pNode->m_pClntBridge)
			delete pNode->m_pClntBridge;
	}
	TRACE_LEAVE
	return 0;
}

int CUdpMultiPlayer::startServer(std::string url)
{
	TRACE_ENTER
	CServerNodeBase *pNode = getServerNode(url);
	if(pNode)
		pNode->start();
	TRACE_LEAVE
	return 0;
}

int CUdpMultiPlayer::subscribeStream(std::string url, int substrmId)
{
	CUdpServerNode *pNode = (CUdpServerNode *)getServerNode(url);
	if(pNode) {
		return pNode->subscribeStream(substrmId);
	}
	return 0;
}

int CUdpMultiPlayer::unsubscribeStream(std::string url, int substrmId)
{
	CUdpServerNode *pNode = (CUdpServerNode *)getServerNode(url);
	if(pNode) {
		return pNode->unsubscribeStream(substrmId);
	}
	return 0;
}

int CUdpMultiPlayer::getData(std::string url, int substrmId, char *pData, int numBytes)
{
	CUdpServerNode *pNode = (CUdpServerNode *)getServerNode(url);
	if(pNode) {
		return pNode->getData(substrmId, (char *)pData, numBytes);
	}
	return 0;
}


long long CUdpMultiPlayer::getPts(std::string url, int substrmId)
{
	CUdpServerNode *pNode = (CUdpServerNode *)getServerNode(url);
	if(pNode) {
		return pNode->getPts(substrmId);
	}
	return 0;
}

long long CUdpMultiPlayer::getClkUs(std::string url)
{
	CUdpServerNode *pNode = (CUdpServerNode *)getServerNode(url);
	if(pNode) {
		return pNode->getClkUs();
	}
	return 0;
}


int  CUdpMultiPlayer::getCodecType(std::string url, int substrmId)
{
	CUdpServerNode *pNode = (CUdpServerNode *)getServerNode(url);
	if(pNode) {
		return pNode->getCodecType(substrmId);
	}
	return 0;

}

int CUdpMultiPlayer::stopServer(std::string url)
{
	TRACE_ENTER
	CUdpServerNode *pNode = (CUdpServerNode *)getServerNode(url);
	if(pNode)
		pNode->stop();
	TRACE_LEAVE
}

int CUdpMultiPlayer::getStatus(std::string url, std::string &status)
{
	CUdpServerNode *pNode = (CUdpServerNode *)getServerNode(url);
	if(pNode) {
		std::string status;
		pNode->getStatus(status);
	}
}

int  CUdpMultiPlayer::getNumAvailFrames(std::string url, int substrmId)
{
	int res = 0;
	CUdpServerNode *pNode = (CUdpServerNode *)getServerNode(url);
	if(pNode) {
		res = pNode->getNumAvailFrames(substrmId);
	}
	return res;
}

void CUdpMultiPlayer::UpdateStats(const char *url, UDP_SERVER_STATS *pStats)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s: Bitrate aud=%d vid=%d APktLoss= %d VPktLoss=%d jitter=%d",
			url,
			pStats->nAudBitrate,
			pStats->nVidBitrate,
			pStats->nAudPktLoss,
			pStats->nVidPktLoss,
			pStats->nClockJitter));
	if(m_EventCallback)
		((CUdpPlayerEvents *)m_EventCallback)->onServerStatistics(url, pStats);
}

void CUdpMultiPlayer::NotifyPsiChange(const char *url, const char *pPsiData)
{
	if(m_EventCallback)
		((CUdpPlayerEvents *)m_EventCallback)->onPsiChange(url, pPsiData);

}


CPlayerBase *CUdpMultiPlayer::openInstance(CPlayerEventBase *pEventBase)
{
	return new CUdpMultiPlayer(pEventBase);
}

//void CRtspPlayer::closeInstancce(CPlayerBase *pInst)
//{
//	delete (CRtspPlayer*)pInst;
//}
