#include "DashMultiPublishClnt.h"

CDashMultiPublishClnt::CDashMultiPublishClnt(CPublishEventBase *pEventBase)
{
	m_EventCallback = pEventBase;
}

int CDashMultiPublishClnt::AddPublishServer(std::string url, std::string appName, int localRtpPort, int remoteRtpPort, int serverPort)
{

}

int CDashMultiPublishClnt::RemovePublishServer(std::string url)
{

}


int CDashMultiPublishClnt::StartMpdServer(const char *pszConfFile)
{
	int res = 0;

	int nStartIndex;
	const char *pszFilePrefix = NULL;
	const char *pszParentFolder = NULL;
	const char *pszBucketOrServerRoot = NULL;
	int nSegmentTimeMs = 0;
	int nTimeShiftBufferMs = 0;
	char strMpdFileName[256];
	int nMuxType;
	int fFileUpdate = 0;
	//JDBG_LOG(CJdDbg::LVL_TRACE,("Enter"));

	CMediaSwitch *pPublishSwitch = NULL;
	m_pMpdSrvBridge = new CMpdSrvBridge;
	m_pMpdRoot = new CMpdRoot(ONYX_MPD_FILE_NAME);
	//pszBucketOrServerRoot = m_pMpdRoot->GetBaseURL();
	pszBucketOrServerRoot = m_pMpdRoot->GetCutomCfgFolder();
	sprintf(strMpdFileName, "%s/onyx.mpd",pszBucketOrServerRoot);
	m_pMpdRoot->SetSaveFileName(strMpdFileName);
	nSegmentTimeMs = m_pMpdRoot->GetMaxSegmentDuration();
	nTimeShiftBufferMs = m_pMpdRoot->GetTimeShiftBuffer();
	nStartIndex = time(NULL);

	if(m_pMpdRoot && !m_pMpdRoot->m_listPeriods.empty()) {
		CMpdPeriod *pMpdPeriod = m_pMpdRoot->m_listPeriods[0];
		for (std::vector<CMpdAdaptaionSet *>::iterator it = pMpdPeriod->m_listAdaptionSets.begin(); it !=  pMpdPeriod->m_listAdaptionSets.end(); it++) {
			CMpdAdaptaionSet *pAdapSet = *it;
			pszParentFolder = pAdapSet->GetBaseURL();
			nMuxType = pAdapSet->GetMimeType();
			if(pAdapSet->IsSegmentTemplate()) {
				char szSegmentExt[8] = {0};

				if(nMuxType == MPD_MUX_TYPE_TS)
					strcpy(szSegmentExt, TS_SEGEMNT_FILE_EXT);
				else
					strcpy(szSegmentExt, MP4_SEGEMNT_FILE_EXT);

				char *szMedia="$RepresentationID$_$Number$.m4s";
				pAdapSet->SetupTemplate(nStartIndex, nSegmentTimeMs, szMedia);
				fFileUpdate = 1;
			} else {
				// TODO Check for segmentlist xlink:href
				fFileUpdate = 1;
			}
			for (int j = 0; j < pAdapSet->m_listRepresentations.size(); j++) {
				CMpdRepresentation *pRepresentation = pAdapSet->m_listRepresentations[j];
				std::map<std::string, CMediaSwitch *>::iterator it = m_listPublishSwitches.find(pRepresentation->m_inputSwitch);
				if(m_listPublishSwitches.end() != it) {
					const char *pszMimetype = NULL;
					CMpdSrvBridgeChan *pOutBridge;
					pPublishSwitch = (*it).second;

					pszFilePrefix = pRepresentation->GetId();
					//pszMimetype = pRepresentation->GetMimetTpe();
					pOutBridge = m_pMpdSrvBridge->CreateChannel(pRepresentation, nStartIndex, nSegmentTimeMs, nTimeShiftBufferMs, pszFilePrefix, pszParentFolder, pszBucketOrServerRoot, nMuxType);

					pPublishSwitch->AddOutput(pOutBridge);
					pOutBridge->Run(m_pOutputStream);
					{
						int nWith = 0, nHeight = 0, nFrameRate = 0, nBandwidth = 0;
						pPublishSwitch->GetInputParams(&nWith, &nHeight, &nFrameRate, &nBandwidth);
						pRepresentation->SetStreamParams(nWith, nHeight, nFrameRate, nBandwidth);
					}
				}
			}
		}
	}
	if(fFileUpdate) {
		m_pMpdRoot->SaveFile();
	}
	//JDBG_LOG(CJdDbg::LVL_TRACE,("Leave"));
	return res;
}
int CDashMultiPublishClnt::start()
{
#if 0
	char szSwitchIdSection[128];
	int i = 0;
	sprintf(szSwitchIdSection,"%s%d",SWITCH_PREFIX, i);
	m_pOutputStream = new COutputStream("test");
	CMediaSwitch *pPublishSwitch = new CMediaSwitch(szSwitchIdSection);


	//pRtspSrvBridge->Init(m_pOutputStream);
	pPublishSwitch->AddOutput(m_pMpdSrvBridge);
	ConnCtxT   *m_pVidConnSrc = CreateStrmConn(1024*1024,3);
	ConnCtxT   *m_pAudConnSrc = CreateStrmConn(16*1024, 3);

	pPublishSwitch->SetSource(m_pVidConnSrc, m_pAudConnSrc);
	m_pPublishSwitch = pPublishSwitch;
	pPublishSwitch->Run();
#endif
	StartMpdServer(NULL);
}

int CDashMultiPublishClnt::sendAudioData(const char *pData, int numBytes, long Pts, int Flags)
{
	long long llPts = Pts;
	m_pAudConnSrc->Write(m_pAudConnSrc, (char *)pData, numBytes, (unsigned int)Flags, llPts);
	return 0;
}

int CDashMultiPublishClnt::sendVideoData(const char *pData, int numBytes, long Pts, int Flags)
{
	long long llPts = Pts;
	m_pVidConnSrc->Write(m_pAudConnSrc, (char *)pData, numBytes, (unsigned int)Flags, llPts);
	return 0;
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
