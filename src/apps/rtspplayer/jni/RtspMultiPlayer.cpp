#include "RtspMultiPlayer.h"

#define TRACE_ENTER 	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Enter", __FUNCTION__));
#define TRACE_LEAVE 	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));


CRtspMultiPlayer::CRtspMultiPlayer(CPlayerEventBase *pEventBase)
{
	m_EventCallback = pEventBase;
}

CServerNode *CRtspMultiPlayer::getServerNode(std::string url)
{
	CServerNode *pNode = NULL;
	TRACE_ENTER
	ServerNodeMap::iterator it = m_ServerList.find(url);
	if(m_ServerList.end() == szId){
		pNode = it->second;
	}
	TRACE_LEAVE
	return pNode;
}

int CRtspMultiPlayer::addServer(std::string url)
{
	int nResult = 0;
	TRACE_ENTER
	CRtspServerNode *pServerNode = new CRtspServerNode(url);
	m_ServerList[url] = pServerNode;
	CRtspClntBridge *pRtspClntBridge = new  CRtspClntBridge(url, 1/*pInputStream->fEnableAud*/, /*pInputStream->fEnableVid*/, &nResult);
	pServerNode->m_pRtspClntBridge = pRtspClntBridge;
	TRACE_LEAVE
	return 0;
}

int CRtspMultiPlayer::removePublishServer(std::string url)
{
	TRACE_ENTER
	CServerNode *pNode = getServerNode(url);
	if(pNode) {
		if(pNode->m_pRtspClntBridge)
			delete pNode->m_pRtspClntBridge;
		m_ServerList.erase(it);
	}
	TRACE_LEAVE
	return 0;
}

int CRtspMultiPlayer::start(std::string url)
{
	TRACE_ENTER
	CServerNode *pNode = getServerNode(url);
	if(pNode)
		pNode->start();
	TRACE_LEAVE
	return 0;
}

int CRtspMultiPlayer::getAudioData(std::string url, char *pData, int numBytes)
{
	CServerNode *pNode = getServerNode(url);
	if(pNode) {
		return pNode->getAudioData((char *)pData, numBytes);
	}
	return 0;
}

int CRtspMultiPlayer::getVideoData(std::string url, char *pData, int numBytes)
{
	CServerNode *pNode = getServerNode(url);
	if(pNode) {
		return pNode->getVideoData((char *)pData, numBytes);
	}
	return 0;
}

long long CRtspMultiPlayer::getAudioPts(std::string url)
{
	CServerNode *pNode = getServerNode(url);
	if(pNode) {
		return pNode->getAudioPts();
	}
	return 0;
}

long long CRtspMultiPlayer::getVideoPts(std::string url)
{
	CServerNode *pNode = getServerNode(url);
	if(pNode) {
		return pNode->getVideoPts();
	}
	return 0;
}

virtual int  getVideoCodecType(std::string url)
{
	CServerNode *pNode = getServerNode(url);
	if(pNode) {
		return pNode->getVideoCodecType();
	}
	return 0;

}
virtual int  getAudioCodecType(std::string url)
{
	CServerNode *pNode = getServerNode(url);
	if(pNode) {
		return pNode->getAudioCodecType();
	}
	return 0;

}

int CRtspMultiPlayer::stop(std::string url)
{
	TRACE_ENTER
	CServerNode *pNode = getServerNode(url);
	if(pNode)
		pNode->stop();
	TRACE_LEAVE
}

CRtspMultiPlayer::getStatus(std::string url)
{
	CServerNode *pNode = getServerNode(url);
	if(pNode) {
		std::string status;
		pNode->getStatus(status);
	}
}

CPlayerBase *CRtspMultiPlayer::openInstance(CPublishEventBase *pEventBase)
{
	return new CRtspMultiPlayer(pEventBase);
}

//void CRtspPlayer::closeInstancce(CPlayerBase *pInst)
//{
//	delete (CRtspPlayer*)pInst;
//}
