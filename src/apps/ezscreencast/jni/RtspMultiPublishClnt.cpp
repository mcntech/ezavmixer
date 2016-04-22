#include "RtspMultiPublishClnt.h"
#include "JdDbg.h"

static int  modDbgLevel = CJdDbg::LVL_TRACE;

CRtspMultiPublishClnt::CRtspMultiPublishClnt(CPublishEventBase *pEventBase)
{
	m_EventCallback = pEventBase;
}

int CRtspMultiPublishClnt::CreateRtspPublishBridge(const char *szPublishId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));

	// One publish stream for now.
	m_pOutputStream = new COutputStream(szPublishId);
	m_pRtspCommonCfg = new CRtspCommonConfig(szPublishId, 1, 1,0);
	CRtspPublishBridge *pRtspSrvBridge = new CRtspPublishBridge();
	pRtspSrvBridge->SetStreamCfg(m_pRtspCommonCfg);
	pRtspSrvBridge->Init(m_pOutputStream);

	m_listPublishBridges[szPublishId] = pRtspSrvBridge;

	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
}

int CRtspMultiPublishClnt::AddPublishBridgeToMediaSwitch(const char *szPublishId,  const char *szSwitchId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));

	CRtspPublishBridge *pRtspSrvBridge = GetRtspPublishBridge(szPublishId);
	CMediaSwitch *pMediaSwitch = getSwitch(szSwitchId);
	if(pMediaSwitch && pRtspSrvBridge) {
		pMediaSwitch->AddOutput(pRtspSrvBridge);
	}

	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return 0;
}

CRtspPublishBridge *CRtspMultiPublishClnt::GetRtspPublishBridge(const char *szPublishId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));

	// One publish stream for now.
	RtspPublishMap_T::iterator it = m_listPublishBridges.find(szPublishId);
	 if(it != m_listPublishBridges.end()) {
		 CRtspPublishBridge *pRtspSrvBridge = it->second;
		if(pRtspSrvBridge)
			return pRtspSrvBridge;
	 }

	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return NULL;
}

void CRtspMultiPublishClnt::RemoveRtspPublishBridge(const char *szPublishId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));

	// One publish stream for now.
	CRtspPublishBridge *pBridge = GetRtspPublishBridge(szPublishId);
	 if(pBridge) {
		 delete pBridge;
	 }

	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
}

int CRtspMultiPublishClnt::AddPublishServer(std::string url, std::string publishStream, int localRtpPort, int remoteRtpPort, int serverPort)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));

	CRtspServerNode *pServerNode = new CRtspServerNode(url, publishStream, localRtpPort, remoteRtpPort, serverPort);
	m_PublishServerList[url] = pServerNode;
	pServerNode->m_pRtspPublishBridge = GetRtspPublishBridge(publishStream.c_str());

	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
}

int CRtspMultiPublishClnt::RemovePublishServer(std::string url)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));

	ServerNodeMap::iterator it = m_PublishServerList.find(url);
	if(it != m_PublishServerList.end()) {
		CRtspServerNode *pNode = (CRtspServerNode *)it->second;
		delete pNode;
		m_PublishServerList.erase(it);
	}

	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
}

void CRtspMultiPublishClnt::enableRtspLocalServer(const char *szId, const char *szInterfaceName, const char *szStreamName, int nPort, bool fEnableMux)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));

	m_pRtspSrvConfig = new CRtspSrvConfig(szInterfaceName, szStreamName, nPort, false);

	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
}


int CRtspMultiPublishClnt::StartRtspPublishBridge(std::string szPublishId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	CRtspPublishBridge *pRtspSrvBridge = GetRtspPublishBridge(szPublishId.c_str());
	if(pRtspSrvBridge) {
		//pRtspSrvBridge->Run();
		// TODO: Check for rnable flag
		pRtspSrvBridge->SetRtspServerCfg(m_pRtspSrvConfig);
		pRtspSrvBridge->StartRtspServer();
	}

	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return 0;
}
int CRtspMultiPublishClnt::StopRtspPublishBridge(std::string szPublishId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));

	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return 0;
}

int CRtspMultiPublishClnt::StartRtspPublishNode(std::string szPublishUrl)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	CRtspServerNode *pNode = (CRtspServerNode *)getPublishNode(szPublishUrl);
	if(pNode) {
		pNode->start();
	}

	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return 0;
}

int CRtspMultiPublishClnt::StopRtspPublishNode(std::string szPublishNodeId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	CRtspServerNode *pNode = (CRtspServerNode *)getPublishNode(szPublishNodeId);
	if(pNode) {
		pNode->stop();
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return 0;
}

int CRtspMultiPublishClnt::UpdateRtspPublishStatus(std::string szPublishId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));

	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return 0;
}


CPublishClntBase *CRtspMultiPublishClnt::openInstance(CPublishEventBase *pEventBase)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	CPublishClntBase *pPublishClnt = new CRtspMultiPublishClnt(pEventBase);
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return pPublishClnt;
}

//void CRtspMultiPublishClnt::closeInstancce(CPublishClntBase *pInst)
//{
//	delete (CRtspMultiPublishClnt*)pInst;
//}
