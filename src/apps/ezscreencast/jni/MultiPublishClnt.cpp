#include "MultiPublishClnt.h"


void CServerNode::start(COutputStream *pOutputStream)
{
	m_pRtspPublishBridge->Init(pOutputStream);
	m_pRtspPublishBridge->SetPublishServerCfg(&m_Config);
	m_pRtspPublishBridge->ConnectToPublishServer();
}

void CServerNode::stop()
{
	//m_pRtspPublishBridge->deinit();
}

CMultiPublish::CMultiPublish()
{

}

int CMultiPublish::AddPublishServer(std::string url, std::string appName, int localRtpPort, int remoteRtpPort, int serverPort)
{
	CServerNode *pServerNode = new CServerNode(url, appName, localRtpPort, remoteRtpPort, serverPort);
	m_PublishServerList[url] = pServerNode;
	CRtspPublishBridge *pRtspPublishBridge = new CRtspPublishBridge;
	pRtspPublishBridge->SetStreamCfg(pServerNode->m_pRtspCommonCfg);
	pServerNode->m_pRtspPublishBridge = pRtspPublishBridge;
}

int CMultiPublish::RemovePublishServer(std::string url)
{
	ServerNodeMap::iterator it = m_PublishServerList.find(url);
	if(it != m_PublishServerList.end()) {
		CServerNode *pNode = it->second;
		if(pNode->m_pRtspPublishBridge)
			delete pNode->m_pRtspPublishBridge;
		m_PublishServerList.erase(it);
	}
}

int CMultiPublish::start()
{
	char szSwitchIdSection[128];
	int i = 0;
	CServerNode *pServerNode;
	sprintf(szSwitchIdSection,"%s%d",SWITCH_PREFIX, i);
	m_pOutputStream = new COutputStream("test");
	CMediaSwitch *pPublishSwitch = new CMediaSwitch(szSwitchIdSection);

	for(ServerNodeMap::iterator it = m_PublishServerList.begin(); it != m_PublishServerList.end(); it++){
		pServerNode = it->second;
		if(pServerNode) {
			CRtspPublishBridge *pRtspSrvBridge = pServerNode->m_pRtspPublishBridge;
			// TODO pRtspSrvBridge->Prepare(m_pOutputStream);
			pPublishSwitch->AddOutput(pRtspSrvBridge);
		}
	}
}

int CMultiPublish::sendAudioData(const char *pData, int numBytes, long Pts, int Flags)
{
	// TODO
	return 0;
}

int CMultiPublish::sendVideoData(const char *pData, int numBytes, long Pts, int Flags)
{
	// TODO
	return 0;
}

int CMultiPublish::stop()
{
	for(ServerNodeMap::iterator it = m_PublishServerList.begin(); it != m_PublishServerList.end(); it++){
		CServerNode *pServerNode = it->second;
		CRtspPublishBridge *pRtspPublishBridge = pServerNode->m_pRtspPublishBridge;
		if(pRtspPublishBridge) {
			delete pRtspPublishBridge;
		}
		m_PublishServerList.erase(it);
	}
}

CMultiPublish *CMultiPublish::getInstance()
{
	return new CMultiPublish();
}

void CMultiPublish::closeInstancce(CMultiPublish *pInst)
{
	delete pInst;
}
