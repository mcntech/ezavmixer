#include "RtspPlayer.h"

CRtspPlayer::CRtspPlayer(CPublishEventBase *pEventBase)
{
	m_EventCallback = pEventBase;
}

CServerNode *CRtspPlayer::getServerNode(std::string url)
{
	CServerNode *pNode = NULL;
	for (ServerNodeMap::iterator it = m_ServerList.begin(); it != m_ServerList.end(); it++) {
		pNode = *it;
		if(pRep->m_szId == szId){
			return pNode;
		}
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return NULL;
}
int CRtspPlayer::AddServer(std::string url)
{
	CRtspServerNode *pServerNode = new CRtspServerNode(url);
	m_ServerList[url] = pServerNode;
	CRtspClntBridge *pRtspClntBridge = new CRtspPublishBridge;
	pRtspClntBridge->SetStreamCfg(pServerNode->m_pRtspCommonCfg);
	pServerNode->m_pRtspClntBridge = pRtspClntBridge;
}

int CRtspPlayer::RemovePublishServer(std::string url)
{
	CServerNode *pNode = getServerNode(url);
	if(pNode) {
		if(pNode->m_pRtspPublishBridge)
			delete pNode->m_pRtspPublishBridge;
		m_ServerList.erase(it);
	}
}

int CRtspPlayer::start(std::string url)
{
	CServerNode *pNode = getServerNode(url);
	if(pNode)
		pNode->start();
}

int CRtspPlayer::getAudioData(std::string url, const char *pData, int numBytes, long Pts, int Flags)
{
	long long llPts = Pts;
	m_pAudConnSrc->Write(m_pAudConnSrc, (char *)pData, numBytes, (unsigned int)Flags, llPts);
	return 0;
}

int CRtspPlayer::getVideoData(std::string url, const char *pData, int numBytes, long Pts, int Flags)
{
	long long llPts = Pts;
	m_pVidConnSrc->Write(m_pAudConnSrc, (char *)pData, numBytes, (unsigned int)Flags, llPts);
	return 0;
}

int CRtspPlayer::stop(std::string url)
{
	CServerNode *pNode = getServerNode(url);
	if(pNode)
		pNode->stop();
}

CPlayerBase *CRtspPlayer::openInstance(CPublishEventBase *pEventBase)
{
	return new CRtspPlayer(pEventBase);
}

//void CRtspPlayer::closeInstancce(CPlayerBase *pInst)
//{
//	delete (CRtspPlayer*)pInst;
//}
