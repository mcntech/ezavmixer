
#include "StreamUtil.h"
int modDbgLevel = 0;

int CStreamUtil::InitInputStrm(CInputStrmBase *pInputStream, int nSessionId)
{
	int res = -1;

	JDBG_LOG(CJdDbg::LVL_SETUP,("nSessionId=%d nInputType=%d Input=%s",nSessionId, pInputStream->nInputType, pInputStream->pszInputUri));
	pInputStream->nCodecSessionId = nSessionId;


	// TODO: Make it configurable
	pInputStream->fEnableVid = 1;
	pInputStream->nCmd = SESSION_CMD_RUN;
	switch(pInputStream->nInputType) {
#ifdef ENABLE_INPUT_HLS
		caseINPUT_TYPE_HLS:
		{
			int nResult;
			CHlsClntBridge *pHlsClnt = NULL;
			pHlsClnt = new CHlsClntBridge(pInputStream->pszInputUri, pInputStream->fEnableAud, pInputStream->fEnableVid, &nResult);

			if(nResult == 0) {

				if(pInputStream->nWidth == 0)
					pInputStream->nWidth = pHlsClnt->m_lWidth;
				if(pInputStream->nHeight == 0)
					pInputStream->nHeight = pHlsClnt->m_lHeight;

				pInputStream->mpInputBridge = pHlsClnt;
			} else {
				goto Exit;
			}
		}
		break;
#endif
#ifdef ENABLE_INPUT_RTSP
		case INPUT_TYPE_RTSP:
		{
			int nResult = 0;
			CRtspClntBridge *pRtspClnt = NULL;
			pRtspClnt = new CRtspClntBridge(pInputStream->pszInputUri, pInputStream->fEnableAud, pInputStream->fEnableVid, &nResult);

			if(nResult == 0) {
				if(pInputStream->nWidth == 0)
					pInputStream->nWidth = pRtspClnt->m_lWidth;
				if(pInputStream->nHeight == 0)
					pInputStream->nHeight = pRtspClnt->m_lHeight;
				pInputStream->mpInputBridge = pRtspClnt;
			} else {
				goto Exit;
			}
		}
		break;
#endif
#ifdef ENABLE_INPUT_FILE
		case INPUT_TYPE_FILE:
		{
			int nResult;
			CLocalFileSrc  *pFileSrc = new CLocalFileSrc(pInputStream->pszInputUri, pInputStream->fEnableAud, pInputStream->fEnableVid, pInputStream->nSelectProg, &nResult);
			if(nResult == 0) {
				if(pInputStream->nWidth == 0)
					pInputStream->nWidth = pFileSrc->m_lWidth;
				if(pInputStream->nHeight == 0)
					pInputStream->nHeight = pFileSrc->m_lHeight;

				pInputStream->mpInputBridge = pFileSrc;
			} else {
				goto Exit;
			}
		}
		break;
#endif
#ifdef ENABLE_SDI_CAPTURE
		case INPUT_TYPE_CAPTURE:
		{
	// PRODUCT_DVM
			int nResult;
			CSdiCaptureSrc *pCaptureSrc = new CSdiCaptureSrc(pInputStream->pszInputUri, pInputStream->fEnableAud, pInputStream->fEnableVid, 0/*TODO*/, &nResult);
			pInputStream->mpInputBridge =  pCaptureSrc;

		}
		break;
#endif
		case INPUT_TYPE_STRMCONN:
		{
			int nResult;
			// TODO: Retrieve the following interface
			CInprocStrmConnRegistry *pRegistry = CInprocStrmConnRegistry::getRegistry();
			CInprocStrmConn *pStrmInput = pRegistry->getEntry(pInputStream->pszInputUri);
			ConnCtxT *pVidCon = pStrmInput->m_pVidCon;
			ConnCtxT *pAudCon = pStrmInput->m_pAudCon;
			pInputStream->mpInputBridge =  new CStrmConnWrapper(pAudCon != NULL, pVidCon != NULL,  pAudCon, pVidCon, &nResult);
		}
		break;
#ifdef ENABLE_IPC
		case INPUT_TYPE_STRMCONN_IPC:
		{
			int nResult;
			EXT_PARAM_STRMCONN_IPC_T *pExtParam = &pInputStream->ExtParam.strmconn_ipc;
			pInputStream->mpInputBridge =  new CIpcUdpSrc(
													pExtParam->szAudSocketRxName,
													pExtParam->szAudSocketTxName,
													pExtParam->szVidSocketRxName,
													pExtParam->szVidSocketTxName, 1, 1,  &nResult);
		}
		break;
#endif
#ifdef ENABLE_ZMQ

		case INPUT_TYPE_STRMCONN_ZMQ:
		{
			int nResult;
			EXT_PARAM_STRMCONN_IPC_T *pExtParam = &pInputStream->ExtParam.strmconn_ipc;
			pInputStream->mpInputBridge =  new CIpcZmqSrc(
													pExtParam->szAudSocketTxName,
													pExtParam->szVidSocketTxName, 1, 1,  &nResult);
		}
		break;
#endif

		defaut:
			break;
	}
	Exit:
	return res;
}

void CStreamUtil::DeinitInputStrm(CInputStrmBase *pInputStream)
{
	if(pInputStream->mpInputBridge) {
		JDBG_LOG(CJdDbg::LVL_TRACE,("Stopping RtspClnt"));
		pInputStream->mpInputBridge->StopStreaming();
		delete (pInputStream->mpInputBridge);
		pInputStream->mpInputBridge = NULL;
	}
}

CAvmixInputStrm *CStreamUtil::GetStreamParamsFromCfgDb(const char *pszSection, const char *pszConfFile)
{
	INPUT_TYPE_T nType = INPUT_TYPE_UNKNOWN;
	char szType[16];
	char szInput[MAX_URI_SIZE];
	CAvmixInputStrm *pInputStream = new CAvmixInputStrm();

	ini_gets(pszSection, KEY_INPUT_TYPE, "", szType, 16, pszConfFile);

	JDBG_LOG(CJdDbg::LVL_SETUP,("Section=%s szType=%s\n", pszSection, szType));

	if (strcmp(szType, INPUT_STREAM_TYPE_FILE) == 0) {
		nType = INPUT_TYPE_FILE;
		ini_gets(pszSection, "location", "", szInput, MAX_URI_SIZE, pszConfFile);
	} else if (strcmp(szType, INPUT_STREAM_TYPE_RTSP) == 0) {
		char szHost[64];
		char szPort[64];
		char szStream[64];

		ini_gets(pszSection, KEY_INPUT_HOST, "", szHost, 64, pszConfFile);
		if(strlen(szHost)) {
			nType = INPUT_TYPE_RTSP;
			ini_gets(pszSection, KEY_INPUT_PORT, "554", szPort, 64, pszConfFile);
			ini_gets(pszSection, KEY_INPUT_STREAM, "v03", szStream, 64, pszConfFile);
			sprintf(szInput, "rtsp://%s:%s/%s", szHost, szPort, szStream);
		}
	} else if (strcmp(szType, INPUT_STREAM_TYPE_HLS) == 0) {
		char szHost[64];
		char szPort[64];
		char szApplication[64];
		char szStream[64];

		ini_gets(pszSection, KEY_INPUT_HOST, "", szHost, 64, pszConfFile);
		if(strlen(szHost)) {
			nType = INPUT_TYPE_HLS;
			ini_gets(pszSection, KEY_INPUT_PORT, "8080", szPort, 64, pszConfFile);
			ini_gets(pszSection, KEY_INPUT_APPLICATION, "httplive", szApplication, 64, pszConfFile);
			ini_gets(pszSection, KEY_INPUT_STREAM, "channel1", szStream, 64, pszConfFile);
			sprintf(szInput, "http://%s:%s/%s/%s.m3u8", szHost, szPort, szApplication, szStream);
		}
	} else if (strcmp(szType, INPUT_STREAM_TYPE_CAPTURE) == 0) {
		nType = INPUT_TYPE_CAPTURE;
		ini_gets(pszSection, KEY_INPUT_DEVICE, "/dev/dvm_sdi", szInput, 64, pszConfFile);
	} else if (strcmp(szType, INPUT_STREAM_TYPE_STRMCONN) == 0) {
		strcpy(szInput, "strmconn");
		nType = INPUT_TYPE_STRMCONN;
	} else if (strcmp(szType, INPUT_STREAM_TYPE_STRMCONN_IPC) == 0) {
		nType = INPUT_TYPE_STRMCONN_IPC;
		strcpy(szInput, "strmconn_ipc");
		EXT_PARAM_STRMCONN_IPC_T *pExtPram = &pInputStream->ExtParam.strmconn_ipc;
		ini_gets(pszSection, KEY_INPUT_AUD_PORT_LOCAL, "", pExtPram->szAudSocketRxName, IPC_SOCK_PORT_NAME_SIZE, pszConfFile);
		ini_gets(pszSection, KEY_INPUT_AUD_PORT_PEER, "", pExtPram->szAudSocketTxName, IPC_SOCK_PORT_NAME_SIZE, pszConfFile);
		ini_gets(pszSection, KEY_INPUT_VID_PORT_LOCAL, "", pExtPram->szVidSocketRxName, IPC_SOCK_PORT_NAME_SIZE, pszConfFile);
		ini_gets(pszSection, KEY_INPUT_VID_PORT_PEER, "", pExtPram->szVidSocketTxName, IPC_SOCK_PORT_NAME_SIZE, pszConfFile);
	} else if (strcmp(szType, INPUT_STREAM_TYPE_STRMCONN_ZMQ) == 0) {
		nType = INPUT_TYPE_STRMCONN_ZMQ;
		strcpy(szInput, "strmconn_zmq");
		EXT_PARAM_STRMCONN_IPC_T *pExtPram = &pInputStream->ExtParam.strmconn_ipc;
		ini_gets(pszSection, KEY_INPUT_AUD_PORT_PEER, "", pExtPram->szAudSocketTxName, IPC_SOCK_PORT_NAME_SIZE, pszConfFile);
		ini_gets(pszSection, KEY_INPUT_VID_PORT_PEER, "", pExtPram->szVidSocketTxName, IPC_SOCK_PORT_NAME_SIZE, pszConfFile);
	} else if (strcmp(szType, INPUT_STREAM_TYPE_AVMIXER) == 0) {
		ini_gets(pszSection, KEY_INPUT_DEVICE , "avmixer0", szInput, 64, pszConfFile);
		nType = INPUT_TYPE_AVMIXER;
	} else {
		strcpy(szInput, "Unknown");
	}
	strcpy(pInputStream->pszInputUri, szInput);
	pInputStream->nInputType = nType;
	return pInputStream;
}
