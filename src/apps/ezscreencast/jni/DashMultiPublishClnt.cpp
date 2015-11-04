#include "DashMultiPublishClnt.h"
#include <android/log.h>
static int modDbgLevel = 0;
#define DBGLOG(...) ((void) __android_log_print(ANDROID_LOG_DEBUG  ,"ezscreencast",  __VA_ARGS__))

CDashMultiPublishClnt::CDashMultiPublishClnt(CPublishEventBase *pEventBase)
{
	m_EventCallback = pEventBase;
}

int CDashMultiPublishClnt::CreateMpd(std::string szId)
{
	DBGLOG("%s:%d", __FILE__, __LINE__);
	CMpdRoot *pMpdRoot = new CMpdRoot(1);
	DBGLOG("%s:%d", __FILE__, __LINE__);
	m_listMpd[szId] = pMpdRoot;
	DBGLOG("%s:%d", __FILE__, __LINE__);
	return 0;
}
int CDashMultiPublishClnt::CreatePeriod(std::string szmpdId, std::string szperiodId)
{
	CMpdRoot *pMpdRoot = m_listMpd[szmpdId];
	if(pMpdRoot) {
		CMpdPeriod *pPeriod = pMpdRoot->CreatePeriod(szperiodId);
		return 0;
	}
	return -1;
}

int CDashMultiPublishClnt::CreateAdaptationSet(std::string szmpdId, std::string szperiodId, std::string szadaptId)
{
	CMpdRoot *pMpdRoot = m_listMpd[szmpdId];
	if(pMpdRoot) {
		CMpdPeriod *pPeriod = pMpdRoot->FindPeriod(szperiodId);
		if(pPeriod) {
			pPeriod->CreateAdaptationSet(szadaptId);
			return 0;
		}
	}
	return -1;
}

int CDashMultiPublishClnt::CreateRepresentation(std::string szmpdId, std::string szperiodId, std::string szadaptId, std::string szrepId)
{
	CMpdRoot *pMpdRoot = m_listMpd[szmpdId];
	if(pMpdRoot) {
		CMpdPeriod *pPeriod = pMpdRoot->FindPeriod(szperiodId);
		if(pPeriod) {
			CMpdAdaptaionSet *pAdaptationSet = pPeriod->FindAdaptationSet(szadaptId);
			if(pAdaptationSet) {
				pAdaptationSet->CreateRepresentation(szrepId, 1);
				return 0;
			}
		}
	}
	return -1;
}

CMpdRoot *CDashMultiPublishClnt::getMpd(std::string szmpdId)
{
	MpdRootMap::iterator it = m_listMpd.find(szmpdId);
	if(it != m_listMpd.end()){
		CMpdRoot *pMpdRoot = it->second;
		return pMpdRoot;
	}
	return NULL;
}

CMpdRepresentation *CDashMultiPublishClnt::FindRepresentation(std::string szmpdId, std::string szperiodId, std::string szadaptId, std::string szrepId)
{
	DBGLOG("%s:%d", __FILE__, __LINE__);
	MpdRootMap::iterator it = m_listMpd.find(szmpdId);
	if(it != m_listMpd.end()){
		CMpdRoot *pMpdRoot = it->second;
		if(pMpdRoot){
			return pMpdRoot->FindRepresentation(szperiodId, szadaptId, szrepId);
		}
	}
	DBGLOG("%s:%d", __FILE__, __LINE__);
}

int CDashMultiPublishClnt::CreateMpdPublishStream(std::string szmpdId, std::string szperiodId, std::string szadaptId, std::string szrepId, std::string strSwitchId, std::string strServerNode)
{
	DBGLOG("%s:%d", __FILE__, __LINE__);

	CMpdRoot  *pMpdRoot = getMpd(szmpdId);
	CMpdRepresentation *pRepresentation = FindRepresentation(szmpdId, szperiodId, szadaptId, szrepId);
	if(pRepresentation) {
		ServerNodeMap::iterator it = m_PublishServerList.find(strServerNode);
		if(it == m_PublishServerList.end()) {
			DBGLOG("%s:%d: m_PublishServerList is empty", __FILE__, __LINE__);
			return -1;
		}
		CS3PublishNode *pServerNode = (CS3PublishNode *)it->second;
		if(pServerNode) {
			CreateMpdPublishStream(pMpdRoot, strSwitchId, pRepresentation, pServerNode);
		}
	} else {
		DBGLOG("%s:%d: Representation not found", __FILE__, __LINE__);
	}
	DBGLOG("%s:%d", __FILE__, __LINE__);
	return 0;
}

int CDashMultiPublishClnt::CreateMpdPublishStream(CMpdRoot *pMpdRoot, std::string strSwitchId, CMpdRepresentation *pRepresentation, CS3PublishNode *pServerNode)
{
	int nStartIndex;
	int nSegmentTimeMs = 0;
	int nTimeShiftBufferMs = 0;
	char strMpdFileName[256];
	int nMuxType;
	int fFileUpdate = 0;
	//JDBG_LOG(CJdDbg::LVL_TRACE,("Enter"));
	nSegmentTimeMs = pMpdRoot->GetMaxSegmentDuration();
	nTimeShiftBufferMs = pMpdRoot->GetTimeShiftBuffer();
	nStartIndex = time(NULL);

	CMediaSwitch *pPublishSwitch = NULL;

	std::map<std::string, CMediaSwitch *>::iterator it = m_listPublishSwitches.find(strSwitchId);
	if(m_listPublishSwitches.end() != it) {
		const char *pszMimetype = NULL;
		CMpdSrvBridgeChan *pOutBridge;
		pPublishSwitch = (*it).second;

		//pszFilePrefix = pRepresentation->GetId();
		//pszMimetype = pRepresentation->GetMimetTpe();

		pOutBridge = m_pMpdSrvBridge->CreateChannel(pRepresentation,
				nStartIndex, nSegmentTimeMs, nTimeShiftBufferMs,
				pServerNode->m_szFilePefix.c_str(), pServerNode->m_szFolder.c_str(), pServerNode->m_szBucket.c_str(),
				pServerNode->m_szHost.c_str(), pServerNode->m_szAccesId.c_str(), pServerNode->m_szSecKey.c_str(),
				nMuxType);

		pPublishSwitch->AddOutput(pOutBridge);
		pOutBridge->Run(m_pOutputStream);
		{
			int nWith = 0, nHeight = 0, nFrameRate = 0, nBandwidth = 0;
			pPublishSwitch->GetInputParams(&nWith, &nHeight, &nFrameRate, &nBandwidth);
			pRepresentation->SetStreamParams(nWith, nHeight, nFrameRate, nBandwidth);
		}

	}
}

int CDashMultiPublishClnt::start()
{
	//StartMpdServer(NULL);
}


int CDashMultiPublishClnt::stop()
{
#if 0
	m_pPublishSwitch->Stop();
	for(ServerNodeMap::iterator it = m_PublishServerList.begin(); it != m_PublishServerList.end(); it++){
		CServerNode *pServerNode = it->second;
		CRtspPublishBridge *pRtspPublishBridge = pServerNode->m_pRtspPublishBridge;
		if(pRtspPublishBridge) {
			delete pRtspPublishBridge;
		}
		m_PublishServerList.erase(it);
	}
	delete m_pPublishSwitch;
#endif

}

CPublishClntBase *CDashMultiPublishClnt::openInstance(CPublishEventBase *pEventBase)
{
	return new CDashMultiPublishClnt(pEventBase);
}

//void CDashMultiPublishClnt::closeInstancce(CPublishClntBase *pInst)
//{
//	delete (CDashMultiPublishClnt*)pInst;
//}
