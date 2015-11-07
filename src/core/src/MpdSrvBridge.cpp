#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "JdDbg.h"
#include "MpdSrvBridge.h"


static int  modDbgLevel = CJdDbg::LVL_TRACE;

static int HasSps(unsigned char *pData, int nLen)
{
	int i = 0;
	while(i < nLen - 4) {
		if ((pData[i+4] & 0x1F) == 0x07 && pData[i+3] == 0x01 && pData[i+2] == 0x00 && pData[i+1] == 0x00 && pData[i+0] == 0x00)
			return 1;
		i++;
	}
	return 0;
}

CMpdSrvBridgeChan::CMpdSrvBridgeChan(int fMuxType, int nSegmentStart, int nSegmentTimeMs, int nTimeShiftBuffer) : CStrmOutBridge("MPD")
{
	m_fEnableServer = 0;
	m_fEnablePublish = 0;
	m_pHttpSrv = NULL;
	m_pUploader = NULL;
	m_pUploaderForExtHttpSrv = NULL;
	m_nSegmentStart = nSegmentStart;
	m_nSegmentTime = nSegmentTimeMs;
	m_nTimeShiftBuffer = nTimeShiftBuffer;
	m_pMpdRepresentation = NULL;
	m_nMuxType = fMuxType;
	m_pSegmenter = NULL;
	m_fDiscont = 1;
}

CMpdSrvBridgeChan::~CMpdSrvBridgeChan()
{
	if(m_pHttpSrv) {
		m_pHttpSrv->Stop(0);
		delete m_pHttpSrv;
	}

	if(m_pUploader){
		mpdPublishStop(m_pUploader);
	}
	if(m_pUploaderForExtHttpSrv){
		mpdPublishStop(m_pUploaderForExtHttpSrv);
	}
	if(m_pMpdRepresentation) {
		delete m_pMpdRepresentation;
	}
	if(m_pSegmenter) {
		mpdDeleteSegmenter(m_pSegmenter);
	}
}

int CMpdSrvBridgeChan::SendVideo(unsigned char *pData, int size, unsigned long lPts)
{
	long hr = 0;
	JDBG_LOG(CJdDbg::LVL_STRM, ("vid:size=%d pts=%d", size,lPts));
	m_Mutex.Acquire();
	if(m_pSegmenter) {
		if (mpdWriteFrameData(m_pSegmenter, (char *)pData, size, 1, m_fDiscont, lPts / 90) < 0 ) {
			hr = -1;
		}
	}
	if(m_fDiscont) {
		m_fDiscont = 0;
	}

	m_Mutex.Release();

	return hr;
}

int CMpdSrvBridgeChan::SetVideoDiscont() 
{ 
	m_fDiscont = 1;
	return 0;
}
int CMpdSrvBridgeChan::SetVideoEos() 
{ 
	return mpdEndOfSeq(m_pSegmenter);
}



int CMpdSrvBridgeChan::SendAudio(unsigned char *pData, int size, unsigned long lPts)
{
	long hr = 0;
#if 0
	unsigned long ulFlags = 0;
	CAccessUnit Au;
	Au.m_TSStreamID = 0;
	Au.m_SampleStreamType = SAMPLE_TYPE_AAC;
	Au.m_pRawData = (char *)pData;
	Au.m_RawSize = size;
	Au.m_pTsData = m_pTsBuffer;
	Au.m_PTS = lPts;
	Au.m_DTS = Au.m_PTS;
	
	m_TsMux.Mux(&Au);
	m_Mutex.Acquire();
	if(m_pSegmenter) {
		if (mpdWriteFrameData(m_pSegmenter, m_pTsBuffer, Au.m_TsSize, 1, ulFlags, lPts / 90) < 0 ) {
			hr = -1;
		}
	}
	m_Mutex.Release();
#endif
	return hr;
}

int CMpdSrvBridgeChan::SetServerConfig(
	CMpdRepresentation *pMpdRepresentation,  
	const char *pszFilePrefix, 
	const char *pszParentFolder, 
	const char *pszBucketOrServerRoot,
	const char *pszHost, const char *pszAccessId, const char *pszSecKey)
{ 
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:%s:%s:%s:%s", pszFilePrefix,pszParentFolder,pszBucketOrServerRoot, pszHost, pszAccessId));
	m_fEnableServer = 1;
	m_pMpdRepresentation = pMpdRepresentation;
	if(pszFilePrefix)
		strcpy(m_szFilePrefix, pszFilePrefix);
	if(pszParentFolder)
		strcpy(m_szParentFolder, pszParentFolder);
	if(pszBucketOrServerRoot)
		strcpy(m_szBucketOrServerRoot,pszBucketOrServerRoot);

	if(pszHost)
		strcpy(m_szHost, pszHost);

	if(pszAccessId)
		strcpy(m_szAccessId, pszAccessId);

	if(pszSecKey)
		strcpy(m_szSecKey, pszSecKey);
	return 0;
}

int CMpdSrvBridgeChan::Run(COutputStream *pOutputStream)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("Enter"));
	if(m_fEnableServer){
		int nUpLoadType;
		nUpLoadType = MPD_UPLOADER_TYPE_MEM;
		char szM3u8File[128];
		//hlsSetDebugLevel(m_pPublishCfg->m_nDdebugLevel);
		if(m_szHost)
			nUpLoadType = MPD_UPLOADER_TYPE_S3;
		else
			nUpLoadType = MPD_UPLOADER_TYPE_MEM;

		m_pSegmenter = mpdCreateSegmenter(m_pMpdRepresentation);

		m_pUploaderForExtHttpSrv = mpdPublishStart(
			-1, m_pSegmenter, 
			m_pMpdRepresentation, 
			m_szFilePrefix, 
			m_szParentFolder, 
			m_szBucketOrServerRoot,  
			m_szHost,
			m_szAccessId,
			m_szSecKey,
			m_pMpdRepresentation->IsLive(), 
			m_nSegmentStart, nUpLoadType);
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("Leave"));
	return 0;
}

int CMpdSrvBridgeChan::UpdateStats()
{
	if(m_pUploaderForExtHttpSrv) {
		int nStreamInTime;
		int nStreamOutTime;
		mpdPublishGetStats(m_pUploaderForExtHttpSrv, &m_nState, &nStreamInTime, &m_nErrors,  &nStreamOutTime, &m_nVidStrmTime);
	}
	return 0;
}

CMpdSrvBridge::CMpdSrvBridge()
{

}
CMpdSrvBridge::~CMpdSrvBridge()
{

}

CMpdSrvBridgeChan *CMpdSrvBridge::CreateChannel(
		std::string         szId,
		CMpdRepresentation *pCfgRepresenation, 
		int        nSegmentStart,
		int        nSegemtTimeMs, 
		int        nTimeShiftBuffer, 
		const char *pszFilePrefix, 
		const char *pszParentFolder, 
		const char *pszBucketOrServerRoot,
		const char *pszHost, const char *pszAccessId, const char *pszSecKey,
		int        nMimeType)
{
	CMpdSrvBridgeChan *pChan = new CMpdSrvBridgeChan(nMimeType, nSegmentStart, nSegemtTimeMs, nTimeShiftBuffer);
	pChan->SetServerConfig(pCfgRepresenation, pszFilePrefix, pszParentFolder, pszBucketOrServerRoot, pszHost, pszAccessId, pszSecKey);

	m_listChan[szId] = pChan;
	return pChan;
}

CMpdSrvBridgeChan *CMpdSrvBridge::getChannel(std::string szId)
{
	CMpdSrvBridgeChan *pChan = NULL;
	std::map<std::string, CMpdSrvBridgeChan *>::iterator it = m_listChan.find(szId);
	if (it != m_listChan.end()){
		pChan = it->second;
	}
	return pChan;
}
