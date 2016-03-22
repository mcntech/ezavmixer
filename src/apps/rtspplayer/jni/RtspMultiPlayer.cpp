#include "RtspMultiPlayer.h"

static int modDbgLevel = CJdDbg::LVL_TRACE;
#define TRACE_ENTER 	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Enter", __FUNCTION__));
#define TRACE_LEAVE 	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));


CRtspMultiPlayer::CRtspMultiPlayer(CPlayerEventBase *pEventBase)
{
	m_EventCallback = pEventBase;
}

CServerNode *CRtspMultiPlayer::getServerNode(std::string url)
{
	CServerNode *pNode = NULL;
	//TRACE_ENTER
	ServerNodeMap::iterator it = m_ServerList.find(url);
	if(m_ServerList.end() != it){
		pNode = it->second;
	}
	//TRACE_LEAVE
	return pNode;
}

void CRtspMultiPlayer::remServerNode(std::string url)
{
	TRACE_ENTER
	ServerNodeMap::iterator it = m_ServerList.find(url);
	if(m_ServerList.end() != it){
		m_ServerList.erase(it);
	}
	TRACE_LEAVE
}
int CRtspMultiPlayer::addServer(std::string url)
{
	int nResult = 0;
	TRACE_ENTER
	CRtspClntBridge *pRtspClntBridge = new  CRtspClntBridge(url.c_str(), 1/*pInputStream->fEnableAud*/, 1/*pInputStream->fEnableVid*/, &nResult);

	CRtspServerNode *pServerNode = new CRtspServerNode(pRtspClntBridge);
	m_ServerList[url] = pServerNode;
	TRACE_LEAVE
	return 0;
}

int CRtspMultiPlayer::removeServer(std::string url)
{
	TRACE_ENTER
	CRtspServerNode *pNode = (CRtspServerNode *)getServerNode(url);
	if(pNode) {
		if(pNode->m_pRtspClntBridge)
			delete pNode->m_pRtspClntBridge;
	}
	TRACE_LEAVE
	return 0;
}

int CRtspMultiPlayer::startServer(std::string url)
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
	CRtspServerNode *pNode = (CRtspServerNode *)getServerNode(url);
	if(pNode) {
		return pNode->getAudioData((char *)pData, numBytes);
	}
	return 0;
}

int CRtspMultiPlayer::getVideoData(std::string url, char *pData, int numBytes)
{
	CRtspServerNode *pNode = (CRtspServerNode *)getServerNode(url);
	if(pNode) {
		return pNode->getVideoData((char *)pData, numBytes);
	}
	return 0;
}

long long CRtspMultiPlayer::getAudioPts(std::string url)
{
	CRtspServerNode *pNode = (CRtspServerNode *)getServerNode(url);
	if(pNode) {
		return pNode->getAudioPts();
	}
	return 0;
}

long long CRtspMultiPlayer::getClkUs(std::string url)
{
	CRtspServerNode *pNode = (CRtspServerNode *)getServerNode(url);
	if(pNode) {
		return pNode->getClkUs();
	}
	return 0;
}

long long CRtspMultiPlayer::getVideoPts(std::string url)
{
	CRtspServerNode *pNode = (CRtspServerNode *)getServerNode(url);
	if(pNode) {
		return pNode->getVideoPts();
	}
	return 0;
}

int  CRtspMultiPlayer::getVideoCodecType(std::string url)
{
	CRtspServerNode *pNode = (CRtspServerNode *)getServerNode(url);
	if(pNode) {
		return pNode->getVideoCodecType();
	}
	return 0;

}
int  CRtspMultiPlayer::getAudioCodecType(std::string url)
{
	CRtspServerNode *pNode = (CRtspServerNode *)getServerNode(url);
	if(pNode) {
		return pNode->getAudioCodecType();
	}
	return 0;

}

int CRtspMultiPlayer::stopServer(std::string url)
{
	TRACE_ENTER
	CRtspServerNode *pNode = (CRtspServerNode *)getServerNode(url);
	if(pNode)
		pNode->stop();
	TRACE_LEAVE
}

int CRtspMultiPlayer::getStatus(std::string url, std::string &status)
{
	CRtspServerNode *pNode = (CRtspServerNode *)getServerNode(url);
	if(pNode) {
		std::string status;
		pNode->getStatus(status);
	}
}

int  CRtspMultiPlayer::getNumAvailVideoFrames(std::string url)
{
	int res = 0;
	CRtspServerNode *pNode = (CRtspServerNode *)getServerNode(url);
	if(pNode) {
		res = pNode->getNumAvailVideoFrames();
	}
	return res;
}
int  CRtspMultiPlayer::getNumAvailAudioFrames(std::string url)
{
	int res = 0;
	CRtspServerNode *pNode = (CRtspServerNode *)getServerNode(url);
	if(pNode) {
		res = pNode->getNumAvailAudioFrames();
	}
	return res;
}

CPlayerBase *CRtspMultiPlayer::openInstance(CPlayerEventBase *pEventBase)
{
	return new CRtspMultiPlayer(pEventBase);
}

//void CRtspPlayer::closeInstancce(CPlayerBase *pInst)
//{
//	delete (CRtspPlayer*)pInst;
//}
