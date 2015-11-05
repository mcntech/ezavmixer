#include "DashMultiPublishClnt.h"
#include <android/log.h>
static int modDbgLevel = 0;
#define DBGLOG(...) ((void) __android_log_print(ANDROID_LOG_DEBUG  ,"ezscreencast",  __VA_ARGS__))

#define TRACE_BEGIN DBGLOG("%s:%d:begin", __FILE__, __LINE__);
#define TRACE_END DBGLOG("%s:%d:begin", __FILE__, __LINE__);

CDashMultiPublishClnt::CDashMultiPublishClnt(CPublishEventBase *pEventBase)
{
	m_EventCallback = pEventBase;
	m_pMpdSrvBridge = new CMpdSrvBridge;
	m_pOutputStream = new COutputStream("Test");
}

int CDashMultiPublishClnt::CreateMpd(std::string szId)
{
	TRACE_BEGIN
	CMpdRoot *pMpdRoot = new CMpdRoot(1);
	m_listMpd[szId] = pMpdRoot;
	TRACE_END
	return 0;
}
int CDashMultiPublishClnt::CreatePeriod(std::string szmpdId, std::string szperiodId)
{
	TRACE_BEGIN
	int res = -1;
	CMpdRoot *pMpdRoot = m_listMpd[szmpdId];
	if(pMpdRoot) {
		CMpdPeriod *pPeriod = pMpdRoot->CreatePeriod(szperiodId);
		res = 0;
	}
	TRACE_END
	return res;
}

int CDashMultiPublishClnt::CreateAdaptationSet(std::string szmpdId, std::string szperiodId, std::string szadaptId)
{
	TRACE_BEGIN
	int res = -1;
	CMpdRoot *pMpdRoot = m_listMpd[szmpdId];
	if(pMpdRoot) {
		CMpdPeriod *pPeriod = pMpdRoot->FindPeriod(szperiodId);
		if(pPeriod) {
			pPeriod->CreateAdaptationSet(szadaptId);
			res = 0;
		}
	}
	TRACE_END
	return res;
}

int CDashMultiPublishClnt::CreateRepresentation(std::string szmpdId, std::string szperiodId, std::string szadaptId, std::string szrepId)
{
	TRACE_BEGIN
	int res = -1;
	CMpdRoot *pMpdRoot = m_listMpd[szmpdId];
	if(pMpdRoot) {
		CMpdPeriod *pPeriod = pMpdRoot->FindPeriod(szperiodId);
		if(pPeriod) {
			CMpdAdaptaionSet *pAdaptationSet = pPeriod->FindAdaptationSet(szadaptId);
			if(pAdaptationSet) {
				res = pAdaptationSet->CreateRepresentation(szrepId, 1);
			}
		}
	}
	TRACE_END
	return res;
}

CMpdRoot *CDashMultiPublishClnt::getMpd(std::string szmpdId)
{
	TRACE_BEGIN
	CMpdRoot *pMpdRoot = NULL;
	MpdRootMap::iterator it = m_listMpd.find(szmpdId);
	if(it != m_listMpd.end()){
		pMpdRoot = it->second;
	}
	TRACE_END
	return pMpdRoot;
}

CMpdRepresentation *CDashMultiPublishClnt::FindRepresentation(std::string szmpdId, std::string szperiodId, std::string szadaptId, std::string szrepId)
{
	TRACE_BEGIN
	CMpdRepresentation *pRep = NULL;
	MpdRootMap::iterator it = m_listMpd.find(szmpdId);
	if(it != m_listMpd.end()){
		CMpdRoot *pMpdRoot = it->second;
		if(pMpdRoot){
			pRep = pMpdRoot->FindRepresentation(szperiodId, szadaptId, szrepId);
		}
	}
	TRACE_END
	return pRep;
}

int CDashMultiPublishClnt::CreateMpdPublishStream(std::string szId, std::string szmpdId, std::string szperiodId, std::string szadaptId, std::string szrepId, std::string strSwitchId, std::string strServerNode)
{
	TRACE_BEGIN
	int res = -1;
	CMpdRepresentation *pRepresentation = NULL;
	CS3PublishNode *pServerNode = NULL;
	CMediaSwitch *pSwitch = NULL;
	do {
		CMpdRoot  *pMpdRoot = getMpd(szmpdId);
		if(pMpdRoot == NULL) {
			DBGLOG("%s:%d notfound %s", __FILE__, __LINE__, szmpdId.c_str());
			break;
		}
		pRepresentation = FindRepresentation(szmpdId, szperiodId, szadaptId, szrepId);
		if(pMpdRoot == NULL) {
			DBGLOG("%s:%d notfound %s", __FILE__, __LINE__, szrepId.c_str());
			break;
		}
		ServerNodeMap::iterator it = m_PublishServerList.find(strServerNode);
		DBGLOG("%s:%d", __FILE__, __LINE__);
		if(it == m_PublishServerList.end()) {
			DBGLOG("%s:%d: m_PublishServerList is empty", __FILE__, __LINE__);
			break;
		}
		pServerNode = (CS3PublishNode *)it->second;
		DBGLOG("%s:%d", __FILE__, __LINE__);
		if(pServerNode) {
			pSwitch =  getSwitch(strSwitchId);
			if(pSwitch) {
				res = CreateMpdPublishStream(szId, pMpdRoot, pSwitch, pRepresentation, pServerNode);
			}
		}
	} while(0);
Exit:
	TRACE_END
	return res;
}

int CDashMultiPublishClnt::SatrtMpdPublishStream(std::string szPublishId)
{
	TRACE_BEGIN
	int res = -1;
	CMpdSrvBridgeChan *pOutBridge = m_pMpdSrvBridge->getChannel(szPublishId);
	if(pOutBridge) {
		res = pOutBridge->Run(m_pOutputStream);
	}
	return res;
	TRACE_END
}

int CDashMultiPublishClnt::CreateMpdPublishStream(std::string szId, CMpdRoot *pMpdRoot, CMediaSwitch *pPublishSwitch, CMpdRepresentation *pRepresentation, CS3PublishNode *pServerNode)
{
	int nStartIndex;
	int nSegmentTimeMs = 0;
	int nTimeShiftBufferMs = 0;
	char strMpdFileName[256];
	int nMuxType;
	int fFileUpdate = 0;

	TRACE_BEGIN
	//JDBG_LOG(CJdDbg::LVL_TRACE,("Enter"));
	nSegmentTimeMs = pMpdRoot->GetMaxSegmentDuration();
	DBGLOG("%s:%d", __FILE__, __LINE__);
	nTimeShiftBufferMs = pMpdRoot->GetTimeShiftBuffer();
	nStartIndex = time(NULL);

	const char *pszMimetype = NULL;
	CMpdSrvBridgeChan *pOutBridge;

	//pszFilePrefix = pRepresentation->GetId();
	//pszMimetype = pRepresentation->GetMimetTpe();

	pOutBridge = m_pMpdSrvBridge->CreateChannel(szId, pRepresentation,
			nStartIndex, nSegmentTimeMs, nTimeShiftBufferMs,
			pServerNode->m_szFilePefix.c_str(), pServerNode->m_szFolder.c_str(), pServerNode->m_szBucket.c_str(),
			pServerNode->m_szHost.c_str(), pServerNode->m_szAccesId.c_str(), pServerNode->m_szSecKey.c_str(),
			nMuxType);

	pPublishSwitch->AddOutput(pOutBridge);
	//pOutBridge->Run(m_pOutputStream);
	{
		int nWith = 0, nHeight = 0, nFrameRate = 0, nBandwidth = 0;
		pPublishSwitch->GetInputParams(&nWith, &nHeight, &nFrameRate, &nBandwidth);
		pRepresentation->SetStreamParams(nWith, nHeight, nFrameRate, nBandwidth);
	}
	TRACE_END
}

int CDashMultiPublishClnt::start()
{
	// Run switch

	// Run output bridge
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
