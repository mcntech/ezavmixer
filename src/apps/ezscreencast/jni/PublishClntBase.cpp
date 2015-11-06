#include "PublishClntBase.h"
#include <android/log.h>

#define DBGLOG(...) ((void) __android_log_print(ANDROID_LOG_DEBUG  ,"ezscreencast",  __VA_ARGS__))

#define TRACE_BEGIN DBGLOG("%s:%d:begin", __FILE__, __LINE__);
#define TRACE_END DBGLOG("%s:%d:end", __FILE__, __LINE__);

static int modDbgLevel = 0;

int CPublishClntBase::AddPublishServer(std::string url, std::string appName, int localRtpPort, int remoteRtpPort, int serverPort)
{

}

int CPublishClntBase::AddS3PublishNode(std::string szId, std::string szHost, std::string szAccessId, std::string szSecKey,
		std::string szBucket, std::string szFolder, std::string szFilePerfix)
{
	 TRACE_BEGIN

	CS3PublishNode *pNode = new CS3PublishNode(szHost, szAccessId, szSecKey,
			szBucket, szFolder, szFilePerfix);
	m_PublishServerList[szId] = pNode;
	TRACE_END

}

int CPublishClntBase::RemovePublishServer(std::string url)
{
	int res = 0;
	return res;
}

int CPublishClntBase::CreateInputStrm(const char *szInputId, const char *szInputType, const char *szInputUri)
{
	TRACE_BEGIN
	int res = 0;
	CMediaSwitch *pPublishSwitch = NULL;
	CAvmixInputStrm *pSwitchpInput = new CAvmixInputStrm();
	pSwitchpInput->nInputType = CStreamUtil::InputTypeStringToInt(szInputType);
	strncpy(pSwitchpInput->pszInputUri, szInputUri, 127);
	res  = CStreamUtil::InitInputStrm(pSwitchpInput);
	m_listInputStrmConn[szInputId] = pSwitchpInput;
	TRACE_END
	return res;
}

CAvmixInputStrm *CPublishClntBase::GetInputStrm(const char *szInputId)
{
	TRACE_BEGIN
	CAvmixInputStrm *pSwitchpInput = NULL;
	std::map <std::string, CAvmixInputStrm *>::iterator it = m_listInputStrmConn.find(szInputId);
	if(it != m_listInputStrmConn.end()) {
		pSwitchpInput = it->second;
	}
	TRACE_END
	return pSwitchpInput;
}

ConnCtxT *CPublishClntBase::GetInputStrmConn(const char *szInputId, int nCodec)
{
	CAvmixInputStrm *pSwitchpInput = GetInputStrm(szInputId);
	ConnCtxT *pConn = NULL;
	if (pSwitchpInput && pSwitchpInput->mpInputBridge) {
		if(nCodec == 0/*audio TODO*/) {
			XADataSource *pDataSrc1 = pSwitchpInput->mpInputBridge->GetDataSource1();
			if(pDataSrc1) {
				XADataLocator_Address *pDataLocatorVideo = (XADataLocator_Address *)pDataSrc1->pLocator;
				pConn = (ConnCtxT  *)pDataLocatorVideo->pAddress;
			}
		} else if(nCodec == 1 /*video*/){
			XADataSource *pDataSrc2 = pSwitchpInput->mpInputBridge->GetDataSource2();
			if(pDataSrc2) {
				XADataLocator_Address *pDataLocatorAudio = (XADataLocator_Address *)pDataSrc2->pLocator;
				pConn = (ConnCtxT   *)pDataLocatorAudio->pAddress;
			}
		}
	}
	return pConn;
}

int CPublishClntBase::CreateSwitch(const char *szSwitchId)
{
	TRACE_BEGIN
	int res = 0;
	CMediaSwitch *pPublishSwitch = new CMediaSwitch(szSwitchId);
	m_listPublishSwitches[szSwitchId] = pPublishSwitch;
	TRACE_END
	return res;
}

CMediaSwitch *CPublishClntBase::getSwitch(std::string szSwitchId)
{
	TRACE_BEGIN
	CMediaSwitch *pPublishSwitch = NULL;

	std::map<std::string, CMediaSwitch *>::iterator it = m_listPublishSwitches.find(szSwitchId);
	if(m_listPublishSwitches.end() != it) {
		//const char *pszMimetype = NULL;
		//CMpdSrvBridgeChan *pOutBridge;
		pPublishSwitch = (*it).second;
	}
	TRACE_END
	return pPublishSwitch;
}

 int CPublishClntBase::startSwitch(std::string szSwitchId)
{
	 TRACE_BEGIN
	int res = -1;
	CMediaSwitch *pPublishSwitch = getSwitch(szSwitchId);
	if(pPublishSwitch) {
		res = pPublishSwitch->Run();
	}
	TRACE_END
	return res;
}

CS3PublishNode *CPublishClntBase::getPublishNode(std::string szPublishNode)
{
	TRACE_BEGIN
	CS3PublishNode *pNode = NULL;
	ServerNodeMap::iterator it = m_PublishServerList.find(szPublishNode);
	if(it == m_PublishServerList.end()) {
		pNode = (CS3PublishNode *)it->second;
	}
	TRACE_END
	return pNode;
}

int CPublishClntBase::ConnectSwitchInput(const char *szInputId, const char *pszSwitchId)
{
	TRACE_BEGIN
	int res = -1;
	CMediaSwitch *pPublishSwitch = NULL;
	CAvmixInputStrm *pSwitchpInput = NULL;
	ConnCtxT   *pVidConnSrc = NULL;
	ConnCtxT   *pAudConnSrc = NULL;
	do{
		std::map<std::string, CMediaSwitch *>::iterator it = m_listPublishSwitches.find(pszSwitchId);
		if(m_listPublishSwitches.end() != it){
			pPublishSwitch = (*it).second;
		} else {
			break;
		}


		std::map<std::string, CAvmixInputStrm *>::iterator it2 = m_listInputStrmConn.find(pszSwitchId);
		if(m_listInputStrmConn.end() != it2){
			pSwitchpInput = (*it2).second;
		} else {
			break;
		}

		if (pSwitchpInput->mpInputBridge) {
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
	} while(0);

	TRACE_END
	return res;
}
