#include "PublishClntBase.h"
#include <android/log.h>
#include "JdDbg.h"

static int modDbgLevel = CJdDbg::LVL_STRM;

int CPublishClntBase::AddPublishServer(std::string url, std::string appName, int localRtpPort, int remoteRtpPort, int serverPort)
{

}

int CPublishClntBase::AddS3PublishNode(std::string szId, std::string szHost, std::string szAccessId, std::string szSecKey,
		std::string szBucket, std::string szFolder, std::string szFilePerfix)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Enter", __FUNCTION__));

	CS3PublishNode *pNode = new CS3PublishNode(szHost, szAccessId, szSecKey,
			szBucket, szFolder, szFilePerfix);
	m_PublishServerList[szId] = pNode;
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));

}

int CPublishClntBase::RemovePublishServer(std::string url)
{
	int res = 0;
	return res;
}

int CPublishClntBase::CreateInputStrm(const char *szInputId, const char *szInputType, const char *szInputUri)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Enter", __FUNCTION__));
	int res = 0;
	JDBG_LOG(CJdDbg::LVL_ERR,("%s:Creating %s!",__FUNCTION__, szInputId));

	CAvmixInputStrm *pSwitchpInput = new CAvmixInputStrm();
	pSwitchpInput->nInputType = CStreamUtil::InputTypeStringToInt(szInputType);
	strncpy(pSwitchpInput->pszInputUri, szInputUri, 127);
	res  = CStreamUtil::InitInputStrm(pSwitchpInput);
	m_listInputStrmConn[szInputId] = pSwitchpInput;
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return res;
}

CAvmixInputStrm *CPublishClntBase::GetInputStrm(const char *szInputId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Enter", __FUNCTION__));

	CAvmixInputStrm *pSwitchpInput = NULL;
	std::map <std::string, CAvmixInputStrm *>::iterator it = m_listInputStrmConn.find(szInputId);
	if(it != m_listInputStrmConn.end()) {
		pSwitchpInput = it->second;
	} else {
		JDBG_LOG(CJdDbg::LVL_ERR,("%s:input id %s: not found!",__FUNCTION__, szInputId));
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return pSwitchpInput;
}

ConnCtxT *CPublishClntBase::GetInputStrmConn(const char *szInputId, int nCodec)
{
	CAvmixInputStrm *pSwitchpInput = GetInputStrm(szInputId);
	ConnCtxT *pConn = NULL;
	if (pSwitchpInput && pSwitchpInput->mpInputBridge) {
		if(nCodec == STRM_CONN_VID) {
			XADataSource *pDataSrc1 = pSwitchpInput->mpInputBridge->GetDataSource1();
			if(pDataSrc1) {
				XADataLocator_Address *pDataLocatorVideo = (XADataLocator_Address *)pDataSrc1->pLocator;
				pConn = (ConnCtxT  *)pDataLocatorVideo->pAddress;
			}
		} else if(nCodec == STRM_CONN_AUD){
			XADataSource *pDataSrc2 = pSwitchpInput->mpInputBridge->GetDataSource2();
			if(pDataSrc2) {
				XADataLocator_Address *pDataLocatorAudio = (XADataLocator_Address *)pDataSrc2->pLocator;
				pConn = (ConnCtxT   *)pDataLocatorAudio->pAddress;
			}
		}
	} else {
		JDBG_LOG(CJdDbg::LVL_ERR,("%s:input id codec=%d %s: not found!",__FUNCTION__, szInputId, nCodec));
	}
	return pConn;
}

int CPublishClntBase::CreateSwitch(const char *szSwitchId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Enter", __FUNCTION__));
	int res = 0;
	CMediaSwitch *pPublishSwitch = new CMediaSwitch(szSwitchId);
	m_listPublishSwitches[szSwitchId] = pPublishSwitch;
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return res;
}

CMediaSwitch *CPublishClntBase::getSwitch(std::string szSwitchId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Enter", __FUNCTION__));
	CMediaSwitch *pPublishSwitch = NULL;

	std::map<std::string, CMediaSwitch *>::iterator it = m_listPublishSwitches.find(szSwitchId);
	if(m_listPublishSwitches.end() != it) {
		//const char *pszMimetype = NULL;
		//CMpdSrvBridgeChan *pOutBridge;
		pPublishSwitch = (*it).second;
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return pPublishSwitch;
}

 int CPublishClntBase::startSwitch(std::string szSwitchId)
{
	 JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Enter", __FUNCTION__));
	int res = -1;
	CMediaSwitch *pPublishSwitch = getSwitch(szSwitchId);
	if(pPublishSwitch) {
		res = pPublishSwitch->Run();
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return res;
}

CS3PublishNode *CPublishClntBase::getPublishNode(std::string szPublishNode)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Enter", __FUNCTION__));
	CS3PublishNode *pNode = NULL;
	ServerNodeMap::iterator it = m_PublishServerList.find(szPublishNode);
	if(it == m_PublishServerList.end()) {
		pNode = (CS3PublishNode *)it->second;
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return pNode;
}

int CPublishClntBase::ConnectSwitchInput(const char *pszSwitchId, const char *szInputId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Enter", __FUNCTION__));
	int res = -1;
	CMediaSwitch *pPublishSwitch = NULL;
	CAvmixInputStrm *pSwitchpInput = NULL;
	ConnCtxT   *pVidConnSrc = NULL;
	ConnCtxT   *pAudConnSrc = NULL;
	do{
		pPublishSwitch = getSwitch(pszSwitchId);
		if(pPublishSwitch == NULL) {
			JDBG_LOG(CJdDbg::LVL_MSG,("%s:switch id %s: not found!",__FUNCTION__, pszSwitchId));
			break;
		}
		pSwitchpInput = GetInputStrm(szInputId);
		if(pSwitchpInput == NULL) {
			JDBG_LOG(CJdDbg::LVL_MSG,("%s:input id %s: not found!",__FUNCTION__,szInputId));
			break;
		}
		pVidConnSrc = GetInputStrmConn(szInputId, STRM_CONN_VID);
		pAudConnSrc = GetInputStrmConn(szInputId, STRM_CONN_AUD);

		pPublishSwitch->SetSource(pVidConnSrc, pAudConnSrc);
		if(pSwitchpInput->mpInputBridge) {
			pSwitchpInput->mpInputBridge->StartStreaming();
		}
		pPublishSwitch->m_pSwitchInput = pSwitchpInput;
	} while(0);

	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));
	return res;
}
