#include "DashMultiPublishClnt.h"
#include "jOnyxEvents.h"
#include <android/log.h>
#include "JdAwsContext.h"

static int modDbgLevel = 0;

CDashMultiPublishClnt::CDashMultiPublishClnt(CPublishEventBase *pEventBase)
{
	m_EventCallback = pEventBase;
	m_pMpdSrvBridge = new CMpdSrvBridge;
	m_pOutputStream = new COutputStream("Test");
}

int CDashMultiPublishClnt::CreateMpd(std::string szId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	CMpdRoot *pMpdRoot = new CMpdRoot(1);
	m_listMpd[szId] = pMpdRoot;
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return 0;
}
int CDashMultiPublishClnt::CreatePeriod(std::string szmpdId, std::string szperiodId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	int res = -1;
	CMpdRoot *pMpdRoot = m_listMpd[szmpdId];
	if(pMpdRoot) {
		CMpdPeriod *pPeriod = pMpdRoot->CreatePeriod(szperiodId);
		res = 0;
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return res;
}

int CDashMultiPublishClnt::CreateAdaptationSet(std::string szmpdId, std::string szperiodId, std::string szadaptId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	int res = -1;
	CMpdRoot *pMpdRoot = m_listMpd[szmpdId];
	if(pMpdRoot) {
		CMpdPeriod *pPeriod = pMpdRoot->FindPeriod(szperiodId);
		if(pPeriod) {
			pPeriod->CreateAdaptationSet(szadaptId);
			res = 0;
		}
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return res;
}

int CDashMultiPublishClnt::CreateRepresentation(std::string szmpdId, std::string szperiodId, std::string szadaptId, std::string szrepId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	int fSegmentTemplate = 0;
	int res = -1;
	CMpdRoot *pMpdRoot = m_listMpd[szmpdId];
	if(pMpdRoot) {
		CMpdPeriod *pPeriod = pMpdRoot->FindPeriod(szperiodId);
		if(pPeriod) {
			CMpdAdaptaionSet *pAdaptationSet = pPeriod->FindAdaptationSet(szadaptId);
			if(pAdaptationSet) {
				res = pAdaptationSet->CreateRepresentation(szrepId, fSegmentTemplate);
			}
		}
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return res;
}

CMpdRoot *CDashMultiPublishClnt::getMpd(std::string szmpdId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	CMpdRoot *pMpdRoot = NULL;
	MpdRootMap::iterator it = m_listMpd.find(szmpdId);
	if(it != m_listMpd.end()){
		pMpdRoot = it->second;
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return pMpdRoot;
}

CMpdRepresentation *CDashMultiPublishClnt::FindRepresentation(std::string szmpdId, std::string szperiodId, std::string szadaptId, std::string szrepId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	CMpdRepresentation *pRep = NULL;
	MpdRootMap::iterator it = m_listMpd.find(szmpdId);
	if(it != m_listMpd.end()){
		CMpdRoot *pMpdRoot = it->second;
		if(pMpdRoot){
			pRep = pMpdRoot->FindRepresentation(szperiodId, szadaptId, szrepId);
		}
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return pRep;
}

int CDashMultiPublishClnt::CreateMpdPublishStream(std::string szId, std::string szmpdId, std::string szperiodId, std::string szadaptId, std::string szrepId, std::string strSwitchId, std::string strServerNode)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	int res = -1;
	CMpdRepresentation *pRepresentation = NULL;
	CS3PublishNode *pServerNode = NULL;
	CMediaSwitch *pSwitch = NULL;
	do {
		CMpdRoot  *pMpdRoot = getMpd(szmpdId);
		if(pMpdRoot == NULL) {
			JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%d notfound %s", __FUNCTION__, __LINE__, szmpdId.c_str()));
			break;
		}
		pRepresentation = FindRepresentation(szmpdId, szperiodId, szadaptId, szrepId);
		if(pMpdRoot == NULL) {
			JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%d notfound %s", __FUNCTION__, __LINE__, szrepId.c_str()));
			break;
		}
		ServerNodeMap::iterator it = m_PublishServerList.find(strServerNode);
		if(it == m_PublishServerList.end()) {
			JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%d: m_PublishServerList is empty", __FUNCTION__, __LINE__));
			break;
		}
		pServerNode = (CS3PublishNode *)it->second;
		if(pServerNode) {
			pSwitch =  getSwitch(strSwitchId);
			if(pSwitch) {
				res = CreateMpdPublishStream(szId, pMpdRoot, pSwitch, pRepresentation, pServerNode);
			}
		}
	} while(0);
Exit:
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return res;
}

int CDashMultiPublishClnt::SatrtMpdPublishStream(std::string szPublishId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	int res = -1;
	CMpdSrvBridgeChan *pOutBridge = m_pMpdSrvBridge->getChannel(szPublishId);
	if(pOutBridge) {
		res = pOutBridge->Run(m_pOutputStream);
	}
	return res;
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
}

int CDashMultiPublishClnt::UpdateMpdPublishStatus(std::string szPublishId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Ener", __FUNCTION__));
	int res = -1;
	CMpdSrvBridgeChan *pOutBridge = m_pMpdSrvBridge->getChannel(szPublishId);
	if(pOutBridge) {
		pOutBridge->UpdateStats();
		int nState = 0, nStremInTime = 0, nLostBufferTime = 0, nStreamOutTime = 0, nSegmentTime = 0;
		pOutBridge->GetPublishStatistics(&nState, &nStremInTime, &nLostBufferTime, &nStreamOutTime, &nSegmentTime);
		COnyxEvents *pCallback = (COnyxEvents *)m_EventCallback;
		pCallback->onMpdPublishStatus(szPublishId.c_str(),nState, nStremInTime, nStreamOutTime, nLostBufferTime);
		res = 0;
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return res;
}
int CDashMultiPublishClnt::CreateMpdPublishStream(std::string szId, CMpdRoot *pMpdRoot, CMediaSwitch *pPublishSwitch, CMpdRepresentation *pRepresentation, CS3PublishNode *pServerNode)
{
	int result = 0;
	int nStartIndex;
	int nSegmentTimeMs = 0;
	int nTimeShiftBufferMs = 0;
	char strMpdFileName[256];
	int nMuxType;
	int fFileUpdate = 0;

	JDBG_LOG(CJdDbg::LVL_TRACE,("%s:Enter", __FUNCTION__));
	nSegmentTimeMs = pMpdRoot->GetMaxSegmentDuration();
	nTimeShiftBufferMs = pMpdRoot->GetTimeShiftBuffer();
	nStartIndex = time(NULL);

	const char *pszMimetype = NULL;
	CMpdSrvBridgeChan *pOutBridge;

	//pszFilePrefix = pRepresentation->GetId();
	//pszMimetype = pRepresentation->GetMimetTpe();
	CJdAwsContext JdAwsContext;
	pOutBridge = m_pMpdSrvBridge->CreateChannel(szId, pRepresentation,
			nStartIndex, nSegmentTimeMs, nTimeShiftBufferMs,
			pServerNode->m_szFilePefix.c_str(), pServerNode->m_szFolder.c_str(), pServerNode->m_szBucket.c_str(),
			&pServerNode->m_AwsContext,
			/*pServerNode->m_szHost.c_str(), pServerNode->m_szAccesId.c_str(), pServerNode->m_szSecKey.c_str(),*/
			nMuxType);

	if(pOutBridge) {
		pPublishSwitch->AddOutput(pOutBridge);
		int nWith = 0, nHeight = 0, nFrameRate = 0, nBandwidth = 0;
		pPublishSwitch->GetInputParams(&nWith, &nHeight, &nFrameRate, &nBandwidth);
		pRepresentation->SetStreamParams(nWith, nHeight, nFrameRate, nBandwidth);
	} else {
		JDBG_LOG(CJdDbg::LVL_ERR,("Failed to create OutputBridge"));
		result = -1;
	}
	JDBG_LOG(CJdDbg::LVL_TRACE,("%s:Leave",__FUNCTION__));
	return result;
}

CPublishClntBase *CDashMultiPublishClnt::openInstance(CPublishEventBase *pEventBase)
{
	return new CDashMultiPublishClnt(pEventBase);
}

void CDashMultiPublishClnt::closeInstancce(CPublishClntBase *pInst)
{
	delete (CDashMultiPublishClnt*)pInst;
}
