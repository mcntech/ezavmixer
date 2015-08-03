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
	rtp_port_offset = (rtp_port_offset + 4) % RTP_PORT_RANGE;
	rtp_port_alloc.usVRtpPort = RTP_PORT_START + rtp_port_offset;
	rtp_port_alloc.usVRtcpPort = RTP_PORT_START + rtp_port_offset + 1;
	rtp_port_alloc.usARtpPort = RTP_PORT_START + rtp_port_offset + 2;
	rtp_port_alloc.usARtcpPort = RTP_PORT_START + rtp_port_offset + 3;

	return 0;
}

int CRtspClntBridge::StartClient(const char *lpszRspServer)
{
	int nResult = 0;
	rtp_port_alloc_t rtp_port;
	InitRtpPort(rtp_port);
	int res = m_pRtspClnt->Open(lpszRspServer, &m_nVidCodec, &m_nAudCodec);
	if(res < 0){
		fprintf(stderr,"Failed to open rtsp connection for %s",lpszRspServer);
		nResult = -1; goto EXIT;
	}
	if (m_nVidCodec ==  RTP_CODEC_MP2T) {
		m_pRtspClnt->SendSetup("video", rtp_port.usVRtpPort, rtp_port.usVRtcpPort);
		//CreateTsOutputPin();
	} else {
		if(m_nVidCodec == RTP_CODEC_H264) {
			int nSpsSize = 256;
			unsigned char Sps[256];
			if(m_pRtspClnt->GetVideoCodecConfig(Sps, &nSpsSize)) {
				H264::cParser Parser;
				if(Parser.ParseSequenceParameterSetMin(Sps,nSpsSize, &m_lWidth, &m_lHeight) != 0){
					// Errorr
				}
			}

			m_pRtspClnt->SendSetup("video", rtp_port.usVRtpPort, rtp_port.usVRtcpPort);
			CreateH264OutputPin();
		}
		if(m_nAudCodec == RTP_CODEC_AAC) {
			CreateMP4AOutputPin();
		} else if(m_nAudCodec == RTP_CODEC_PCMU) {
				m_pRtspClnt->SendSetup("audio", rtp_port.usARtpPort, rtp_port.usARtcpPort);
				CreatePCMUOutputPin();
		}
	}

EXIT:
	return nResult;
}

CRtspClntBridge::CRtspClntBridge(const char *lpszRspServer, int fEnableAud, int fEnableVid, int *pResult) : CStrmInBridgeBase(fEnableAud, fEnableVid)
{
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
	m_lPktLoss = 0;
	m_usSeqNum = 0;
	m_fDisCont = 1;

	*pResult = StartClient(lpszRspServer);
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



long CRtspClntBridge::ProcessVideoFrame()
{
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
		JD_OAL_SLEEP(1)
	}

	m_lUsedLen = 0; 

	while(!fDone && m_fRun) {
		long lAvailEmpty = m_lMaxLen - m_lUsedLen;
		if(lAvailEmpty <= 0) {
			lResult = -1; goto Exit;
		}
		char *pWrite = m_pData + m_lUsedLen;
		long lBytesRead = m_pRfcRtp->GetData(m_pRtspClnt->m_pVRtp, pWrite, lAvailEmpty);
		if(lBytesRead <= 0){
			lResult = -1; goto Exit;
		}
		RTP_PKT_T *pRtpHdr = m_pRfcRtp->GetRtnHdr();
		m_lUsedLen += lBytesRead;
		fDone = pRtpHdr->m;
		m_lPts = pRtpHdr->ulTimeStamp;

		//ChkPktLoss(pRtpHdr);
		m_usSeqNum++;
		if (m_usSeqNum  != pRtpHdr->usSeqNum) {
			// skip starting pkt
			if(m_usSeqNum != 1)	 {
				m_lPktLoss++;
			}
			m_usSeqNum = pRtpHdr->usSeqNum;
		}
		
	}
	if(m_fDisCont) {
		m_fDisCont = 0;
		ulFlags |= OMX_EXT_BUFFERFLAG_DISCONT;
	}
	pConnSink->Write(pConnSink, m_pData, m_lUsedLen,  ulFlags, m_lPts * 1000 / 90);

Exit:
	return lResult;
}

long CRtspClntBridge::ProcessAudioFrame()
{
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
	//ChkPktLoss(pRtpHdr);

	pConnSink->Write(pConnSink, m_pAudData, lBytesRead,  ulFlags, m_lPts * 1000 / 90);

Exit:
	return lResult;
}

int CRtspClntBridge::InitAudioStreaming()
{
	if(m_fEnableAud) {
		m_pRtspClnt->SendPlay("audio");
	}

    return 0;
}

int CRtspClntBridge::InitVideoStreaming()
{
	m_fRun = 1;
	if(m_fEnableVid) {
		m_pRtspClnt->SendPlay("video");
	}

    return 0;
}

void *CRtspClntBridge::DoVideoBufferProcessing(void *pArg)
{
	CRtspClntBridge *pCtx = (CRtspClntBridge *)pArg;
	while(pCtx->m_fRun) {
		if(pCtx->ProcessVideoFrame() != 0){
			break;
		}
	}
    return NULL;
}

void *CRtspClntBridge::DoAudioBufferProcessing(void *pArg)
{
	CRtspClntBridge *pCtx = (CRtspClntBridge *)pArg;
	while(pCtx->m_fRun) {
		if(pCtx->ProcessAudioFrame() != 0){
			break;
		}
	}
    return NULL;
}


int CRtspClntBridge::StartStreaming()
{
	m_fRun = 1;
	if(m_fEnableVid) {
		InitVideoStreaming();
		jdoalThreadCreate((void **)&m_thrdHandleVideo, DoVideoBufferProcessing, this);
	}
	if(m_fEnableAud) {
		InitAudioStreaming();
		jdoalThreadCreate((void **)&m_thrdHandleAudio, DoAudioBufferProcessing, this);
	}

    return 0;
}

int CRtspClntBridge::StopStreaming()
{
	void *res;
	m_fRun = 0;
	if(m_thrdHandleVideo){
		jdoalThreadJoin((void *)m_thrdHandleVideo, 3000);
	}
	if(m_thrdHandleAudio){
		jdoalThreadJoin((void *)m_thrdHandleAudio, 3000);
	}
    return 0;
}

