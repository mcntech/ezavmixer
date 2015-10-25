#include "PublishClntBase.h"

static int modDbgLevel = 0;

int CPublishClntBase::AddPublishServer(std::string url, std::string appName, int localRtpPort, int remoteRtpPort, int serverPort)
{

}

int CPublishClntBase::RemovePublishServer(std::string url)
{
	int res = 0;
	return res;
}

int CPublishClntBase::CreateInputStrm(const char *szInputId, const char *szInputType, const char *szInputUri)
{
	int res = 0;
	CMediaSwitch *pPublishSwitch = NULL;
	CAvmixInputStrm *pSwitchpInput = new CAvmixInputStrm();
	pSwitchpInput->nInputType = CStreamUtil::InputTypeStringToInt(szInputType);
	strncpy(pSwitchpInput->pszInputUri, szInputUri, 127);
	res  = CStreamUtil::InitInputStrm(pSwitchpInput);
	m_listInputStrmConn[szInputId] = pSwitchpInput;
	return res;
}

int CPublishClntBase::CreateSwitch(const char *szSwitchId)
{
	int res = 0;
	CMediaSwitch *pPublishSwitch = new CMediaSwitch(szSwitchId);
	m_listPublishSwitches[szSwitchId] = pPublishSwitch;
	return res;
}

int CPublishClntBase::ConnectSwitchInput(const char *szInputId, const char *pszSwitchId)
{
	CMediaSwitch *pPublishSwitch = NULL;
	std::map<std::string, CMediaSwitch *>::iterator it = m_listPublishSwitches.find(pszSwitchId);
	if(m_listPublishSwitches.end() != it){
		pPublishSwitch = (*it).second;
	} else {
		return -1;
	}

	CAvmixInputStrm *pSwitchpInput = NULL;
	std::map<std::string, CAvmixInputStrm *>::iterator it2 = m_listInputStrmConn.find(pszSwitchId);
	if(m_listInputStrmConn.end() != it2){
		pSwitchpInput = (*it2).second;
	} else {
		return -1;
	}

	if (pSwitchpInput->mpInputBridge) {
		ConnCtxT   *pVidConnSrc = NULL;
		ConnCtxT   *pAudConnSrc = NULL;
		XADataSource *pDataSrc1 = pSwitchpInput->mpInputBridge->GetDataSource1();
		XADataSource *pDataSrc2 = pSwitchpInput->mpInputBridge->GetDataSource2();
		if(pDataSrc1) {
			XADataLocator_Address *pDataLocatorVideo = (XADataLocator_Address *)pDataSrc1->pLocator;
			pVidConnSrc = (ConnCtxT  *)pDataLocatorVideo->pAddress;
		}
		if(pDataSrc2) {
			XADataLocator_Address *pDataLocatorAudio = (XADataLocator_Address *)pDataSrc2->pLocator;
			pAudConnSrc = (ConnCtxT   *)pDataLocatorAudio->pAddress;
		}

		pPublishSwitch->SetSource(pVidConnSrc, pAudConnSrc);
		if(pSwitchpInput->mpInputBridge) {
			pSwitchpInput->mpInputBridge->StartStreaming();
		}
		pPublishSwitch->m_pSwitchInput = pSwitchpInput;
	} else {
		delete pSwitchpInput;
	}
	return 0;
}
