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
#include "UdpClntBridge.h"
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


CUdpClntBridge::CUdpClntBridge(
		const char *lpszRspServer,
		int fEnableAud,
		int fEnableVid,
		int *pResult,
		CUdpServerCallback *pCallback)
		: CStrmInBridgeBase(fEnableAud, fEnableVid)
{
	TRACE_ENTER

	//m_pRtspClnt = new CJdRtspClntSession;

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
	memset(&m_UdpServerStats, 0x00, sizeof(UDP_SERVER_STATS));

	m_pCallback = pCallback;
	mJitterUpdateTime = 0;
	m_thrdHandleVideoRtcp = NULL;
	m_thrdHandleAudioRtcp = NULL;
	m_fEnableRtcp = true;
	*pResult = StartClient(lpszRspServer);

	TRACE_LEAVE
}

CUdpClntBridge::~CUdpClntBridge()
{
	//if(m_pRtspClnt){
	//	//m_pRtspClnt->Close();
	//	delete m_pRtspClnt;
	//}
}

int CUdpClntBridge::StartClient(const char *lpszRspServer)
{
	TRACE_ENTER
#if 0
	int nClock;
	unsigned char ucPlType;
	int nResult = 0;
	rtp_port_alloc_t rtp_port;
	InitRtpPort(rtp_port);

	JDBG_LOG(CJdDbg::LVL_TRACE, ("Open rtsp connection for %s",lpszRspServer));
	strncpy(m_szRemoteHost, lpszRspServer, MAX_NAME_SIZE - 1);

	int res = m_pRtspClnt->Open(lpszRspServer);
	if(res < 0){
		JDBG_LOG(CJdDbg::LVL_ERR, ("Failed to open rtsp connection for %s",lpszRspServer));
		nResult = -1;
		if(m_pCallback) {
			m_pCallback->NotifyStateChange(m_szRemoteHost, RTSP_SERVER_ERROR);
		}
		goto EXIT;
	}

	m_nVidCodec = m_pRtspClnt->GetVideoCodec(&nClock, &ucPlType);
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
			m_pVRtcp = new CJdRtcp(false, nClock, ucPlType);
		}

		m_nAudCodec = m_pRtspClnt->GetAudioCodec(&nClock, &ucPlType);;
		if(m_nAudCodec == RTP_CODEC_AAC) {
			JDBG_LOG(CJdDbg::LVL_TRACE, ("Setup:RTP_CODEC_AAC"));
			m_pRtspClnt->SendSetup("audio", rtp_port.usARtpPort, rtp_port.usARtcpPort);
			CreateMP4AOutputPin();
			m_pARtcp = new CJdRtcp(false, nClock, ucPlType);
		} else if(m_nAudCodec == RTP_CODEC_PCMU) {
			JDBG_LOG(CJdDbg::LVL_TRACE, ("Setup:RTP_CODEC_PCMU"));
			m_pRtspClnt->SendSetup("audio", rtp_port.usARtpPort, rtp_port.usARtcpPort);
			CreatePCMUOutputPin();
			m_pARtcp = new CJdRtcp(false, nClock, ucPlType);
		}

		if(m_pCallback) {
			RTSP_SERVER_DESCRIPTION Descript = {0};
			m_pCallback->NotifyStateChange(m_szRemoteHost, RTSP_SERVER_SETUP);
		}
	}

EXIT:
	TRACE_LEAVE
	return nResult;
#else
	return 0;
#endif
}

long CUdpClntBridge::ProcessVideoFrame()
{
	//TRACE_ENTER
#if 0
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
		if(m_pVRtcp) {
			m_pVRtcp->UpdateStatForRtpPkt(pRtpHdr, lBytesRead);
		}
		m_lUsedLen += lBytesRead;
		fDone = pRtpHdr->m;
		m_lPts = pRtpHdr->ulTimeStamp;

		UpdateJitter(m_lPts);
		//ChkPktLoss(pRtpHdr);
		m_usVidSeqNum++;
		if (m_usVidSeqNum  != pRtpHdr->usSeqNum) {
			// skip starting pkt
			if(m_usVidSeqNum != 1)	 {
				m_UdpServerStats.nVidPktLoss++;
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
#else
	return 0;
#endif
}

long CUdpClntBridge::ProcessAudioFrame()
{
	//TRACE_ENTER
#if 0
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
			m_UdpServerStats.nAudPktLoss++;
		}
		m_usAudSeqNum = pRtpHdr->usSeqNum;
	}

	pConnSink->Write(pConnSink, m_pAudData, lBytesRead,  ulFlags, m_lPts * 1000 / 90);
	JDBG_LOG(CJdDbg::LVL_STRM, ("ProcessVideoFrame:Write %d PTS=%lld", m_lUsedLen, m_lPts));
Exit:
	//TRACE_LEAVE
	return lResult;
#else
	return 0;
#endif
}

int CUdpClntBridge::InitAudioStreaming()
{
	TRACE_ENTER
#if 0
	if(m_fEnableAud) {
		m_pRtspClnt->SendPlay("audio");
	}
	TRACE_LEAVE
#endif
    return 0;
}

int CUdpClntBridge::InitVideoStreaming()
{
	TRACE_ENTER
#if 0
	m_fRun = 1;
	if(m_fEnableVid) {
		m_pRtspClnt->SendPlay("video");
	}
#endif
	TRACE_LEAVE
    return 0;
}

void *CUdpClntBridge::DoVideoBufferProcessing(void *pArg)
{
	TRACE_ENTER
#if 0
	CUdpClntBridge *pCtx = (CUdpClntBridge *)pArg;
	while(pCtx->m_fRun) {
		if(pCtx->ProcessVideoFrame() != 0){
			break;
		}
	}

	TRACE_LEAVE
#endif
    return NULL;
}

void *CUdpClntBridge::DoAudioBufferProcessing(void *pArg)
{
	TRACE_ENTER
#if 0
	CUdpClntBridge *pCtx = (CUdpClntBridge *)pArg;
	while(pCtx->m_fRun) {
		if(pCtx->ProcessAudioFrame() != 0){
			break;
		}
	}

	TRACE_LEAVE
#endif
    return NULL;
}


int CUdpClntBridge::StartStreaming()
{
	TRACE_ENTER
#if 0
	m_fRun = 1;
	if(m_fEnableVid && GetVideoConn() != NULL) {
		InitVideoStreaming();
		jdoalThreadCreate((void **)&m_thrdHandleVideo, DoVideoBufferProcessing, this);
		if(m_fEnableRtcp)
			jdoalThreadCreate((void **)&m_thrdHandleVideoRtcp, DoVideoRtcpProcessing, this);
	} else {
		JDBG_LOG(CJdDbg::LVL_ERR, ("Video not started"));
	}
	if(m_fEnableAud && GetAudioConn() != NULL) {
		InitAudioStreaming();
		jdoalThreadCreate((void **)&m_thrdHandleAudio, DoAudioBufferProcessing, this);
		if(m_fEnableRtcp)
			jdoalThreadCreate((void **)&m_thrdHandleAudioRtcp, DoAudioRtcpProcessing, this);
	} else {
		JDBG_LOG(CJdDbg::LVL_ERR, ("Audio not started m_fEnableAud=%d", m_fEnableAud));
	}

	TRACE_LEAVE
#endif
    return 0;
}

int CUdpClntBridge::StopStreaming()
{
	TRACE_ENTER
#if 0
	void *res;
	m_fRun = 0;
	if(m_pRtspClnt){
		m_pRtspClnt->Close();
	}
	if(m_thrdHandleVideo){
		jdoalThreadJoin((void *)m_thrdHandleVideo, 3000);
	}
	if(m_thrdHandleAudio){
		jdoalThreadJoin((void *)m_thrdHandleAudio, 3000);
	}
	if(m_thrdHandleVideoRtcp){
		jdoalThreadJoin((void *)m_thrdHandleVideoRtcp, 3000);
	}
	if(m_thrdHandleAudioRtcp){
		jdoalThreadJoin((void *)m_thrdHandleAudioRtcp, 3000);
	}

	TRACE_LEAVE
#endif
    return 0;
}

void CUdpClntBridge::UpdateJitter(int nPktTime)
{
#if 0
	struct timeval   tv;
	gettimeofday(&tv,NULL);
	int now =  tv.tv_sec * 1000 + tv.tv_usec / 1000;
	int nJitter = now - mJitterUpdateTime;
	m_UdpServerStats.nClockJitter = nJitter;
	if(nJitter > m_UdpServerStats.nClockJitterMax)
		m_UdpServerStats.nClockJitterMax = nJitter;
	else if(nJitter < m_UdpServerStats.nClockJitterMin)
		m_UdpServerStats.nClockJitterMin = nJitter;
	mJitterUpdateTime=now;
#endif
}
void CUdpClntBridge::UpdateStat()
{
	struct timeval   tv;
	gettimeofday(&tv,NULL);
	long long now =  (long long)tv.tv_sec * 1000 + tv.tv_usec;

	if( now > mDbgPrevTime + 1000) {
		m_UdpServerStats.nAudBitrate = (mTotalAud - mDbgTotalAudPrev) * 8;
		m_UdpServerStats.nVidBitrate = (mTotalVid - mDbgTotalVidPrev) * 8;

		if(m_pCallback){
			m_pCallback->UpdateStats(m_szRemoteHost, &m_UdpServerStats);
		}
		mDbgTotalAudPrev = mTotalAud;
		mDbgTotalVidPrev = mTotalVid;
		mDbgPrevTime = now;
	}
}
