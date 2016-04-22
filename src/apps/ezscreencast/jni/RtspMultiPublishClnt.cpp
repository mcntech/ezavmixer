#include "RtspMultiPublishClnt.h"

CRtspMultiPublishClnt::CRtspMultiPublishClnt(CPublishEventBase *pEventBase)
{
	m_EventCallback = pEventBase;
}

int CRtspMultiPublishClnt::CreateRtspPublishBridge(const char *szPublishId)
{
	// One publish stream for now.
	m_pOutputStream = new COutputStream(szPublishId);
	m_pRtspCommonCfg = new CRtspCommonConfig(/*TODO*/NULL, NULL,1,1,1);
	CRtspPublishBridge *pRtspSrvBridge = new CRtspPublishBridge();
	pRtspSrvBridge->SetStreamCfg(m_pRtspCommonCfg);
	pRtspSrvBridge->Init(m_pOutputStream);

	m_listPublishBridges[szPublishId] = pRtspSrvBridge;
}

int CRtspMultiPublishClnt::AddPublishBridgeToMediaSwitch(const char *szPublishId,  const char *szSwitchId)
{
	CRtspPublishBridge *pRtspSrvBridge = GetRtspPublishBridge(szPublishId);
	CMediaSwitch *pMediaSwitch = getSwitch(szSwitchId);
	if(pMediaSwitch && pRtspSrvBridge) {
		pMediaSwitch->AddOutput(pRtspSrvBridge);
	}
	return 0;
}

CRtspPublishBridge *CRtspMultiPublishClnt::GetRtspPublishBridge(const char *szPublishId)
{
	// One publish stream for now.
	RtspPublishMap_T::iterator it = m_listPublishBridges.find(szPublishId);
	 if(it != m_listPublishBridges.end()) {
		 CRtspPublishBridge *pRtspSrvBridge = it->second;
		if(pRtspSrvBridge)
			return pRtspSrvBridge;
	 }
	 return NULL;
}

void CRtspMultiPublishClnt::RemoveRtspPublishBridge(const char *szPublishId)
{
	// One publish stream for now.
	CRtspPublishBridge *pBridge = GetRtspPublishBridge(szPublishId);
	 if(pBridge) {
		 delete pBridge;
	 }
}

int CRtspMultiPublishClnt::AddPublishServer(std::string url, std::string publishStream, int localRtpPort, int remoteRtpPort, int serverPort)
{
	CRtspServerNode *pServerNode = new CRtspServerNode(url, publishStream, localRtpPort, remoteRtpPort, serverPort);
	m_PublishServerList[url] = pServerNode;
	pServerNode->m_pRtspPublishBridge = GetRtspPublishBridge(publishStream.c_str());
}

int CRtspMultiPublishClnt::RemovePublishServer(std::string url)
{
	ServerNodeMap::iterator it = m_PublishServerList.find(url);
	if(it != m_PublishServerList.end()) {
		CRtspServerNode *pNode = (CRtspServerNode *)it->second;
		delete pNode;
		m_PublishServerList.erase(it);
	}
}

void CRtspMultiPublishClnt::enableRtspLocalServer(const char *szId, const char *szInterfaceName, const char *szStreamName, int nPort, bool fEnableMux)
{
	m_pRtspSrvConfig = new CRtspSrvConfig(szInterfaceName, szStreamName, nPort, false);
}

int CRtspMultiPublishClnt::sendAudioData(const char *szInputId, const char *pData, int numBytes, long Pts, int Flags)
{
	ConnCtxT *pConn = GetInputStrmConn(szInputId, STRM_CONN_AUD);
	long long llPts = Pts;
	pConn->Write(m_pAudConnSrc, (char *)pData, numBytes, (unsigned int)Flags, llPts);
	return 0;
}

int CRtspMultiPublishClnt::sendVideoData(const char *szInputId, const char *pData, int numBytes, long Pts, int Flags)
{
	ConnCtxT *pConn = GetInputStrmConn(szInputId, STRM_CONN_VID);
	long long llPts = Pts;
	pConn->Write(m_pAudConnSrc, (char *)pData, numBytes, (unsigned int)Flags, llPts);
	return 0;
}


int CRtspMultiPublishClnt::StartRtspPublishBridge(std::string szPublishId)
{
	CRtspPublishBridge *pRtspSrvBridge = GetRtspPublishBridge(szPublishId.c_str());
	if(pRtspSrvBridge) {
		//pRtspSrvBridge->Run();
		// TODO: Check for rnable flag
		pRtspSrvBridge->StartRtspServer();
	}
	return 0;
}
int CRtspMultiPublishClnt::StopRtspPublishBridge(std::string szPublishId)
{
	return 0;
}

int CRtspMultiPublishClnt::StartRtspPublishNode(std::string szPublishUrl)
{
	CRtspServerNode *pNode = (CRtspServerNode *)getPublishNode(szPublishUrl);
	if(pNode) {
		pNode->start();
	}
	return 0;
}

int CRtspMultiPublishClnt::StopRtspPublishNode(std::string szPublishNodeId)
{
	CRtspServerNode *pNode = (CRtspServerNode *)getPublishNode(szPublishNodeId);
	if(pNode) {
		pNode->stop();
	}
	return 0;
}

int CRtspMultiPublishClnt::UpdateRtspPublishStatus(std::string szPublishId)
{
	return 0;
}


CPublishClntBase *CRtspMultiPublishClnt::openInstance(CPublishEventBase *pEventBase)
{
	return new CRtspMultiPublishClnt(pEventBase);
}

//void CRtspMultiPublishClnt::closeInstancce(CPublishClntBase *pInst)
//{
//	delete (CRtspMultiPublishClnt*)pInst;
//}
