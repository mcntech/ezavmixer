#ifdef WIN32
//#include <Windows.h>
#include <winsock2.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include "JdRtspSrv.h"
#include "JdRtspClntRec.h"
#include "JdRfc3984.h"
#include "JdDbg.h"
#include "strmconn.h"
#include "h264parser.h"
#include "RtspClntBridge.h"
#include "JdRfc5391.h"
#include "JdOsal.h"
#include <time.h>

static int modDbgLevel = CJdDbg::LVL_TRACE;
#define TRACE_ENTER 	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Enter", __FUNCTION__));
#define TRACE_LEAVE 	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));

#define DEF_CLNT_RTP_PORT_START		59427
#define RTP_PORT_START     59200
#define RTP_PORT_RANGE     800

#define MAX_VID_FRAME_SIZE      (1920 * 1080)
#define MAX_AUD_FRAME_SIZE      (8 * 1024)

static unsigned short rtp_port_offset = 0; 
typedef struct _rtp_port_alloc_t
{
	unsigned short usVRtpPort;
	unsigned short usVRtcpPort;
	unsigned short usARtpPort;
	unsigned short usARtcpPort;
} rtp_port_alloc_t;

int InitRtpPort(rtp_port_alloc_t &rtp_port_alloc)
{
	TRACE_ENTER

	rtp_port_offset = (rtp_port_offset + 4) % RTP_PORT_RANGE;
	rtp_port_alloc.usVRtpPort = RTP_PORT_START + rtp_port_offset;
	rtp_port_alloc.usVRtcpPort = RTP_PORT_START + rtp_port_offset + 1;
	rtp_port_alloc.usARtpPort = RTP_PORT_START + rtp_port_offset + 2;
	rtp_port_alloc.usARtcpPort = RTP_PORT_START + rtp_port_offset + 3;

	TRACE_LEAVE

	return 0;
}

int CRtspClntBridge::StartClient(const char *lpszRspServer)
{
	TRACE_ENTER

	int nResult = 0;
	rtp_port_alloc_t rtp_port;
	InitRtpPort(rtp_port);

	JDBG_LOG(CJdDbg::LVL_TRACE, ("Open rtsp connection for %s",lpszRspServer));
	strncpy(m_szRemoteHost, lpszRspServer, MAX_NAME_SIZE - 1);

	int res = m_pRtspClnt->Open(lpszRspServer, &m_nVidCodec, &m_nAudCodec);
	if(res < 0){
		JDBG_LOG(CJdDbg::LVL_ERR, ("Failed to open rtsp connection for %s",lpszRspServer));
		nResult = -1;
		if(m_pCallback) {
			m_pCallback->NotifyStateChange(m_szRemoteHost, RTSP_SERVER_ERROR);
		}
		goto EXIT;
	}

	JDBG_LOG(CJdDbg::LVL_ERR, ("Open successful codecs aud=%d vid=%d", m_nAudCodec, m_nVidCodec));
	if (m_nVidCodec ==  RTP_CODEC_MP2T) {
		JDBG_LOG(CJdDbg::LVL_ERR, ("Setup:RTP_CODEC_MP2T"));
		m_pRtspClnt->SendSetup("video", rtp_port.usVRtpPort, rtp_port.usVRtcpPort);
		//CreateTsOutputPin();
	} else {
		if(m_nVidCodec == RTP_CODEC_H264) {
			int nSpsSize = 256;
			unsigned char Sps[256];
			if(m_pRtspClnt->GetVideoCodecConfig(Sps, &nSpsSize)) {
				H264::cParser Parser;
				if(Parser.ParseSequenceParameterSetMin(Sps,nSpsSize, &m_lWidth, &m_lHeight) != 0){
					JDBG_LOG(CJdDbg::LVL_ERR, ("Failed to parse stream nSpsSize=%d",nSpsSize));
				}
			}

			JDBG_LOG(CJdDbg::LVL_TRACE, ("Setup:RTP_CODEC_H264"));
			m_pRtspClnt->SendSetup("video", rtp_port.usVRtpPort, rtp_port.usVRtcpPort);
			CreateH264OutputPin();
		}

		if(m_nAudCodec == RTP_CODEC_AAC) {
			JDBG_LOG(CJdDbg::LVL_TRACE, ("Setup:RTP_CODEC_AAC"));
			m_pRtspClnt->SendSetup("audio", rtp_port.usARtpPort, rtp_port.usARtcpPort);
			CreateMP4AOutputPin();
		} else if(m_nAudCodec == RTP_CODEC_PCMU) {
			JDBG_LOG(CJdDbg::LVL_TRACE, ("Setup:RTP_CODEC_PCMU"));
			m_pRtspClnt->SendSetup("audio", rtp_port.usARtpPort, rtp_port.usARtcpPort);
			CreatePCMUOutputPin();
		}

		if(m_pCallback) {
			RTSP_SERVER_DESCRIPTION Descript = {0};
			m_pCallback->NotifyStateChange(m_szRemoteHost, RTSP_SERVER_SETUP);
		}
	}

EXIT:
	TRACE_LEAVE
	return nResult;
}

CRtspClntBridge::CRtspClntBridge(
		const char *lpszRspServer,
		int fEnableAud,
		int fEnableVid,
		int *pResult,
		CRtspServerCallback *pCallback)
		: CStrmInBridgeBase(fEnableAud, fEnableVid)
{
	TRACE_ENTER

	m_pRtspClnt = new CJdRtspClntSession;
	m_pRfcRtp = new CJdRfc3984Rx(2048);
	m_pAudRfcRtp = new CJdRfc5391;

	m_pData = (char *)malloc(MAX_VID_FRAME_SIZE);
	m_lMaxLen = MAX_VID_FRAME_SIZE;
	m_lPts = 0;
	m_lDts = 0;
	m_lUsedLen = 0;
	m_lWidth = 0;
	m_lHeight = 0;

	m_pAudData = (char *)malloc(MAX_AUD_FRAME_SIZE);
	m_lAudUsedLen = 0;
	m_lAudMaxLen = MAX_AUD_FRAME_SIZE;
	m_lAudPts = 0;

	m_thrdHandleAudio = 0;
	m_thrdHandleVideo = 0;
	m_usVidSeqNum = 0;
	m_fDisCont = 1;

	mDbgPrevTime = 0;
	mDbgTotalAudPrev = 0;
	mDbgTotalVidPrev = 0;
	mTotalAud = 0;
	mTotalVid = 0;
	memset(&m_RtspServerStats, 0x00, sizeof(RTSP_SERVER_STATS));

	m_pCallback = pCallback;
	mJitterUpdateTime = 0;

	*pResult = StartClient(lpszRspServer);

	TRACE_LEAVE
}


CRtspClntBridge::~CRtspClntBridge()
{
	if(m_pRtspClnt){
		m_pRtspClnt->Close();
		delete m_pRtspClnt;
	}
	if(m_pRfcRtp)
		delete m_pRfcRtp;
	if(m_pAudRfcRtp)
		delete m_pAudRfcRtp;
}

long CRtspClntBridge::ProcessVideoRtcp()
{
	char *pData = (char *)malloc(2048);
	while(!fDone && m_fRun) {
		long lBytesRead = m_pRtspClnt->m_pVRtp->ReadRtcp(pData, 2048, 0);
		if(lBytesRead > 0){
			m_pVRtcp
		}
		JD_OAL_SLEEP(100)
	}
	if(pData)
		free(pData);
}

long CRtspClntBridge::ProcessVideoFrame()
{
	//TRACE_ENTER

	unsigned ulFlags = 0;
	unsigned long dwBytesRead = 0;
	long lQboxBytes = 0;
	long lResult = 0;
	// Find Stream Type
	long lStrmType = 0;
	long fEof = 0;
	int fDone = 0;

	ConnCtxT   *pConnSink = (ConnCtxT *)mDataLocatorVideo.pAddress;
	while(pConnSink->IsFull(pConnSink) && m_fRun){
		JDBG_LOG(CJdDbg::LVL_STRM, ("ProcessVideoFrame:Buffer Full"));
		JD_OAL_SLEEP(1)
	}

	m_lUsedLen = 0; 

	while(!fDone && m_fRun) {
		long lAvailEmpty = m_lMaxLen - m_lUsedLen;
		if(lAvailEmpty <= 0) {
			JDBG_LOG(CJdDbg::LVL_TRACE, ("ProcessVideoFrame:Buffer Empty"));
			lResult = -1;
			goto Exit;
		}
		char *pWrite = m_pData + m_lUsedLen;
		long lBytesRead = m_pRfcRtp->GetData(m_pRtspClnt->m_pVRtp, pWrite, lAvailEmpty);
		if(lBytesRead <= 0){
			lResult = -1;
			JDBG_LOG(CJdDbg::LVL_ERR, ("ProcessVideoFrame:Failed to GetData"));
			goto Exit;
		}
		mTotalVid += lBytesRead;
		RTP_PKT_T *pRtpHdr = m_pRfcRtp->GetRtnHdr();
		m_lUsedLen += lBytesRead;
		fDone = pRtpHdr->m;
		m_lPts = pRtpHdr->ulTimeStamp;

		UpdateJitter(m_lPts);
		//ChkPktLoss(pRtpHdr);
		m_usVidSeqNum++;
		if (m_usVidSeqNum  != pRtpHdr->usSeqNum) {
			// skip starting pkt
			if(m_usVidSeqNum != 1)	 {
				m_RtspServerStats.nVidPktLoss++;
			}
			m_usVidSeqNum = pRtpHdr->usSeqNum;
		}
		UpdateStat();
	}
	if(m_fDisCont) {
		m_fDisCont = 0;
		ulFlags |= OMX_EXT_BUFFERFLAG_DISCONT;
	}
	pConnSink->Write(pConnSink, m_pData, m_lUsedLen,  ulFlags, m_lPts * 1000 / 90);
	JDBG_LOG(CJdDbg::LVL_STRM, ("ProcessVideoFrame:Write %d PTS=%lld", m_lUsedLen, m_lPts));

Exit:

	//TRACE_LEAVE
	return lResult;
}

long CRtspClntBridge::ProcessAudioFrame()
{
	//TRACE_ENTER

	unsigned ulFlags = 0;
	long lResult = 0;
	// Find Stream Type
	long lStrmType = 0;
	long fEof = 0;
	int fDone = 0;
	RTP_PKT_T *pRtpHdr;
	ConnCtxT   *pConnSink = (ConnCtxT *)mDataLocatorAudio.pAddress;
	while(pConnSink->IsFull(pConnSink) && m_fRun){
		JD_OAL_SLEEP(1)
	}

	char *pWrite = m_pAudData;
	long lBytesRead = m_pAudRfcRtp->GetData(m_pRtspClnt->m_pARtp, pWrite, m_lAudMaxLen);
	if(lBytesRead <= 0){
		lResult = -1; goto Exit;
	}
	pRtpHdr = m_pRfcRtp->GetRtnHdr();
	fDone = pRtpHdr->m;
	mTotalAud += lBytesRead;
	//ChkPktLoss(pRtpHdr);
	m_usAudSeqNum++;
	if (m_usAudSeqNum  != pRtpHdr->usSeqNum) {
		// skip starting pkt
		if(m_usAudSeqNum != 1)	 {
			m_RtspServerStats.nAudPktLoss++;
		}
		m_usAudSeqNum = pRtpHdr->usSeqNum;
	}

	pConnSink->Write(pConnSink, m_pAudData, lBytesRead,  ulFlags, m_lPts * 1000 / 90);
	JDBG_LOG(CJdDbg::LVL_STRM, ("ProcessVideoFrame:Write %d PTS=%lld", m_lUsedLen, m_lPts));
Exit:
	//TRACE_LEAVE
	return lResult;
}

int CRtspClntBridge::InitAudioStreaming()
{
	TRACE_ENTER

	if(m_fEnableAud) {
		m_pRtspClnt->SendPlay("audio");
	}
	TRACE_LEAVE
    return 0;
}

int CRtspClntBridge::InitVideoStreaming()
{
	TRACE_ENTER

	m_fRun = 1;
	if(m_fEnableVid) {
		m_pRtspClnt->SendPlay("video");
	}

	TRACE_LEAVE
    return 0;
}

void *CRtspClntBridge::DoVideoBufferProcessing(void *pArg)
{
	TRACE_ENTER

	CRtspClntBridge *pCtx = (CRtspClntBridge *)pArg;
	while(pCtx->m_fRun) {
		if(pCtx->ProcessVideoFrame() != 0){
			break;
		}
	}

	TRACE_LEAVE
    return NULL;
}

void *CRtspClntBridge::DoAudioBufferProcessing(void *pArg)
{
	TRACE_ENTER

	CRtspClntBridge *pCtx = (CRtspClntBridge *)pArg;
	while(pCtx->m_fRun) {
		if(pCtx->ProcessAudioFrame() != 0){
			break;
		}
	}

	TRACE_LEAVE
    return NULL;
}


int CRtspClntBridge::StartStreaming()
{
	TRACE_ENTER

	m_fRun = 1;
	if(m_fEnableVid && GetVideoConn() != NULL) {
		InitVideoStreaming();
		jdoalThreadCreate((void **)&m_thrdHandleVideo, DoVideoBufferProcessing, this);
	} else {
		JDBG_LOG(CJdDbg::LVL_ERR, ("Video not started"));
	}
	if(m_fEnableAud && GetAudioConn() != NULL) {
		InitAudioStreaming();
		jdoalThreadCreate((void **)&m_thrdHandleAudio, DoAudioBufferProcessing, this);
	} else {
		JDBG_LOG(CJdDbg::LVL_ERR, ("Audio not started m_fEnableAud=%d", m_fEnableAud));
	}

	TRACE_LEAVE
    return 0;
}

int CRtspClntBridge::StopStreaming()
{
	TRACE_ENTER

	void *res;
	m_fRun = 0;
	if(m_thrdHandleVideo){
		jdoalThreadJoin((void *)m_thrdHandleVideo, 3000);
	}
	if(m_thrdHandleAudio){
		jdoalThreadJoin((void *)m_thrdHandleAudio, 3000);
	}

	TRACE_LEAVE
    return 0;
}

void CRtspClntBridge::UpdateJitter(int nPktTime)
{
	struct timeval   tv;
	gettimeofday(&tv,NULL);
	int now =  tv.tv_sec * 1000 + tv.tv_usec / 1000;
	int nJitter = now - mJitterUpdateTime;
	m_RtspServerStats.nClockJitter = nJitter;
	if(nJitter > m_RtspServerStats.nClockJitterMax)
		m_RtspServerStats.nClockJitterMax = nJitter;
	else if(nJitter < m_RtspServerStats.nClockJitterMin)
		m_RtspServerStats.nClockJitterMin = nJitter;
	mJitterUpdateTime=now;
}
void CRtspClntBridge::UpdateStat()
{
	struct timeval   tv;
	gettimeofday(&tv,NULL);
	long long now =  (long long)tv.tv_sec * 1000 + tv.tv_usec;

	if( now > mDbgPrevTime + 1000) {
		m_RtspServerStats.nAudBitrate = (mTotalAud - mDbgTotalAudPrev) * 8;
		m_RtspServerStats.nVidBitrate = (mTotalVid - mDbgTotalVidPrev) * 8;

		if(m_pCallback){
			m_pCallback->UpdateStats(m_szRemoteHost, &m_RtspServerStats);
		}
		mDbgTotalAudPrev = mTotalAud;
		mDbgTotalVidPrev = mTotalVid;
		mDbgPrevTime = now;
	}
}
