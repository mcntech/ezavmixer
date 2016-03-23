#include "RtspMultiPublishClnt.h"

CRtspMultiPublishClnt::CRtspMultiPublishClnt(CPublishEventBase *pEventBase)
{
	m_EventCallback = pEventBase;
}

int CRtspMultiPublishClnt::CreateRtspPublishStream(const char *szId,  const char *szSwitchId)
{
	CRtspPublishBridge *pRtspPublishBridge = new CRtspPublishBridge;
	pRtspPublishBridge->SetStreamCfg(pServerNode->m_pRtspCommonCfg);
}

int CRtspMultiPublishClnt::AddPublishServer(std::string url, std::string appName, int localRtpPort, int remoteRtpPort, int serverPort)
{
	CRtspServerNode *pServerNode = new CRtspServerNode(url, appName, localRtpPort, remoteRtpPort, serverPort);
	m_PublishServerList[url] = pServerNode;
	pServerNode->m_pRtspPublishBridge = pRtspPublishBridge;
}

int CRtspMultiPublishClnt::RemovePublishServer(std::string url)
{
	ServerNodeMap::iterator it = m_PublishServerList.find(url);
	if(it != m_PublishServerList.end()) {
		CRtspServerNode *pNode = (CRtspServerNode *)it->second;
		if(pNode->m_pRtspPublishBridge)
			delete pNode->m_pRtspPublishBridge;
		m_PublishServerList.erase(it);
	}
}

void CRtspMultiPublishClnt::enableRtspLocalServer(const char *szId, const char *szInterfaceName, const char *szStreamName, int nPort, bool fEnableMux)
{
	m_pRtspSrvConfig = new CRtspSrvConfig(szInterfaceName, szStreamName, nPort, szStreamName);
}

int CRtspMultiPublishClnt::start()
{
	char szSwitchIdSection[128];
	int i = 0;
	CRtspServerNode *pServerNode;
	sprintf(szSwitchIdSection,"%s%d",SWITCH_PREFIX, i);
	m_pOutputStream = new COutputStream("test");
	CMediaSwitch *pPublishSwitch = new CMediaSwitch(szSwitchIdSection);

	for(ServerNodeMap::iterator it = m_PublishServerList.begin(); it != m_PublishServerList.end(); it++){
		pServerNode = (CRtspServerNode *)it->second;
		if(pServerNode) {
			CRtspPublishBridge *pRtspSrvBridge = pServerNode->m_pRtspPublishBridge;
			pRtspSrvBridge->Init(m_pOutputStream);
			pPublishSwitch->AddOutput(pRtspSrvBridge);
		}
	}
	ConnCtxT   *m_pVidConnSrc = CreateStrmConn(1024*1024,3);
	ConnCtxT   *m_pAudConnSrc = CreateStrmConn(16*1024, 3);

	pPublishSwitch->SetSource(m_pVidConnSrc, m_pAudConnSrc);
	m_pPublishSwitch = pPublishSwitch;
	pPublishSwitch->Run();
}

int CRtspMultiPublishClnt::sendAudioData(const char *pData, int numBytes, long Pts, int Flags)
{
	long long llPts = Pts;
	m_pAudConnSrc->Write(m_pAudConnSrc, (char *)pData, numBytes, (unsigned int)Flags, llPts);
	return 0;
}

int CRtspMultiPublishClnt::sendVideoData(const char *pData, int numBytes, long Pts, int Flags)
{
	long long llPts = Pts;
	m_pVidConnSrc->Write(m_pAudConnSrc, (char *)pData, numBytes, (unsigned int)Flags, llPts);
	return 0;
}

int CRtspMultiPublishClnt::stop()
{
	m_pPublishSwitch->Stop();
	for(ServerNodeMap::iterator it = m_PublishServerList.begin(); it != m_PublishServerList.end(); it++){
		CRtspServerNode *pServerNode = (CRtspServerNode *)it->second;
		CRtspPublishBridge *pRtspPublishBridge = pServerNode->m_pRtspPublishBridge;
		if(pRtspPublishBridge) {
			delete pRtspPublishBridge;
		}
		m_PublishServerList.erase(it);
	}
	delete m_pPublishSwitch;
}

CPublishClntBase *CRtspMultiPublishClnt::openInstance(CPublishEventBase *pEventBase)
{
	return new CRtspMultiPublishClnt(pEventBase);
}

//void CRtspMultiPublishClnt::closeInstancce(CPublishClntBase *pInst)
//{
//	delete (CRtspMultiPublishClnt*)pInst;
//}
