#ifdef WIN32
//#include <Windows.h>
#include <winsock2.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <sys/time.h>

#include "JdDbg.h"
#include "strmconn.h"
#include "UdpClntBridge.h"
#include "JdOsal.h"
#include "filesrc.h"
#include "udprx.h"
#include "xport.h"
#include "h264parser.h"
#include <time.h>
#include "json.hpp"

static int modDbgLevel = CJdDbg::LVL_SETUP;
#define TRACE_ENTER 	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Enter", __FUNCTION__));
#define TRACE_LEAVE 	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:Leave", __FUNCTION__));

#define DEF_CLNT_RTP_PORT_START		59427
#define RTP_PORT_START     59200
#define RTP_PORT_RANGE     800

#define MAX_VID_FRAME_SIZE      (1920 * 1080)
#define MAX_AUD_FRAME_SIZE      (8 * 1024)

using json = nlohmann::json;

CUdpClntBridge::CUdpClntBridge(
		const char *lpszRspServer,
		int fEnableAud,
		int fEnableVid,
		int *pResult,
		CUdpServerCallback *pCallback)
		//: CStrmInBridgeBase(fEnableAud, fEnableVid)
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

	m_thrdHandle = 0;
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
	m_fEnableRtcp = true;
	*pResult = 0;
	strncpy(m_szRemoteHost, lpszRspServer, MAX_NAME_SIZE - 1);
	m_pat = NULL;
	TRACE_LEAVE
}

CUdpClntBridge::~CUdpClntBridge()
{
	//if(m_pRtspClnt){
	//	//m_pRtspClnt->Close();
	//	delete m_pRtspClnt;
	//}
}

void patCallback(void *ctx, const char *pData, int len)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("Received PSI len=%d", len));
	CUdpClntBridge *pObj = (CUdpClntBridge *)ctx;
	pObj->UpdatePat(pData, len);
}

void pmtCallback(void *ctx, int nPid, const char *pData, int len)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("Received PMT len=%d", len));
	CUdpClntBridge *pObj = (CUdpClntBridge *)ctx;
	pObj->UpdatePmt(nPid, pData, len);
}

void formatCallback(void *ctx, int nPid, int nCodecType, const char *pData, int len)
{
	JDBG_LOG(CJdDbg::LVL_TRACE, ("Received PMT len=%d", len));
	CUdpClntBridge *pObj = (CUdpClntBridge *)ctx;
	pObj->UpdateFormat(nPid, nCodecType, pData, len);
}

void CUdpClntBridge::UpdatePmt(int nPid, const char *pData, int len )
{
	std::map<int, struct MPEG2_PMT_SECTION *>::iterator it = m_pmts.find(nPid);

	if(it != m_pmts.end()) {
		// PMT Exists
		struct MPEG2_PMT_SECTION *pmt =  it->second;
		// Check if version change
		if(!pmt->IsVesionChanged((unsigned char *)pData)) {
			return;
		} else {
			m_pmts.erase(nPid);
			delete pmt;
		}
	}

	MPEG2_PMT_SECTION *pmt = new MPEG2_PMT_SECTION;
	pmt->Parse((unsigned char *)pData);

	//for(int i=0; i < pmt->number_of_elementary_streams ; i++) {
	//int nPid = pmt->elementary_stream_info[i].elementary_PID;
	//pObj->m_demuxComp->SetOutputConn(pObj->m_demuxComp, nPid, (char *)&pgm);
	m_pmts[nPid] = pmt;

	// Notify
	if(m_pCallback) {
		std::string psiString;
		psiPmtJson(pmt, psiString);
		m_pCallback->NotifyPsiPmtChange(m_szRemoteHost, pmt->program_number, psiString.c_str());
	}
}

void CUdpClntBridge::UpdateFormat(int nPid, int nCodecType, const char *pData, int len)
{
    if(nCodecType == 0x1B) {
        if(m_pCallback) {
            std::string fmtString;
            strmFmtJson(pData, len, fmtString);
            m_pCallback->NotifyPsiPmtChange(m_szRemoteHost, nPid, fmtString.c_str());
        }
    }
}

void CUdpClntBridge::strmFmtJson(const char *pFmtData, int len, std::string &psiString)
{
    json jFmt = {};
    long lWidth = 0;
    long lHeight = 0;
    H264::cParser *mH264Parser = new H264::cParser();
    mH264Parser->ParseSequenceParameterSetMin((unsigned char *)pFmtData, len, &lWidth, &lHeight);
    if(lWidth != 0){
        jFmt["width"] = lWidth;
        jFmt["height"] = lHeight;
    }
    psiString = jFmt.dump();
}

std::string StrmTypeToString(int strmType)
{
	std::string str;

	if(strmType == 0x81 || strmType == 0x6)  {
		str = "AC3";
	}
	else if(strmType == 0x3 || strmType == 0x4)  {
		str = "MP2";
	}
	else if(strmType == 0x80)  {
		str = "LPCM";
	}
	else if(strmType == 0x0f)  {
		str = "AAC";
	}
	else if(strmType == 0x1 || strmType == 0x2 || strmType == 0x80)  {
		str = "MPEG2";
	}
	else if(strmType == 0x1b)  {
		str = "H264";
	}
	else  {
		str = "UNKNOWN";
	}

	return str;
}
/*
void CUdpClntBridge::psiJson(std::string &psiString)
{
	json jPsi = {};

	int j = 0;
	for (std::map<int, struct MPEG2_PMT_SECTION *>::iterator it = m_pmts.begin(); it != m_pmts.end(); ++it) {
		int nPid = m_pat->program_descriptor[j].network_or_program_map_PID;
		struct MPEG2_PMT_SECTION *pmt = it->second;
		if(pmt != NULL) {
			json jPmt = {};
			for(int i=0; i < pmt-> number_of_elementary_streams; i++) {
				json jEs = {};
				ELEMENTARY_STREAM_INFO *es = &pmt->elementary_stream_info[i];
				jEs.emplace("pid", es->elementary_PID);
				jEs.emplace("type", es->stream_type);
				jEs.emplace("codec",StrmTypeToString(es->stream_type));
				// TODO es info other attributes
				jPmt["streams"][i] = jEs;
#ifdef DEMUX_DUMP_OUTPUT
				if(es->stream_type == 0x1b)
                	ConnectStreamForPid(es->elementary_PID, NULL);
#endif

			}
			jPmt["pid"] = pmt->program_number;
			jPmt["program"] = m_pat->program_descriptor[j].program_number;
            jPmt["PCR_PID"] = pmt->PCR_PID;
			jPsi[j++] = jPmt;
		}
	}
	psiString = jPsi.dump();
}
*/
void CUdpClntBridge::psiPmtJson(MPEG2_PMT_SECTION *pmt, std::string &psiString)
{
		if(pmt != NULL) {
			json jPmt = {};
			for(int i=0; i < pmt-> number_of_elementary_streams; i++) {
				json jEs = {};
				ELEMENTARY_STREAM_INFO *es = &pmt->elementary_stream_info[i];
				jEs.emplace("pid", es->elementary_PID);
				jEs.emplace("type", es->stream_type);
				jEs.emplace("codec",StrmTypeToString(es->stream_type));
				// TODO es info other attributes
				jPmt["streams"][i] = jEs;
#ifdef DEMUX_DUMP_OUTPUT
				if(es->stream_type == 0x1b)
                	ConnectStreamForPid(es->elementary_PID, NULL);
#endif

			}
			jPmt["program"] = pmt->program_number;
			jPmt["PCR_PID"] = pmt->PCR_PID;
			psiString = jPmt.dump();
		}
}

void CUdpClntBridge::psiPatJson(std::string &psiString)
{
	json jPsi = {};

	for(int j=0; j< m_pat->number_of_programs ; j++) {
		int nPid = m_pat->program_descriptor[j].network_or_program_map_PID;
		int nPprogram = m_pat->program_descriptor[j].program_number;
		json jPmt = {};
		jPmt["pid"] =nPid;
		jPmt["program"] = nPprogram;
		jPsi[j]=jPmt;
	}
	psiString = jPsi.dump();
}

void CUdpClntBridge::UpdatePat(const char *pData, int len)
{
	if(m_pat != NULL) {
		if(!m_pat->IsVesionChanged((unsigned char *)pData)) {
			return;
		} else {
			delete m_pat;
			m_pat = NULL;
		}
	}
	m_pat = new MPEG2_PAT_SECTION;
	m_pat->Parse((unsigned char *)pData);
#ifdef DEMUX_DUMP_OUTPUT
	for(int i=0; i< m_pat->number_of_programs ; i++) {
		//pat.program_descriptor[i].program_number;
		DemuxSubscribeProgramPidT pgm;
		pgm.nPid = m_pat->program_descriptor[i].network_or_program_map_PID;
		pgm.pmt_callback = pmtCallback;
		m_demuxComp->SetOption(m_demuxComp, DEMUX_CMD_SUBSCRIBE_PROGRAM_PID, (char *)&pgm);
	}
#endif
	// Notify
	if(m_pCallback) {
		std::string psiString;
		psiPatJson(psiString);
		m_pCallback->NotifyPsiPatChange(m_szRemoteHost, psiString.c_str());
	}
}


void CUdpClntBridge::SubscribeProgram(int strmId)
{

    if(m_pat != NULL) {
		DemuxSubscribeProgramPidT pgm;
		int nPid = -1;
		for (int i = 0; i < strmId < m_pat->number_of_programs; i++) {
			if  (m_pat->program_descriptor[i].program_number == strmId) {
				nPid = m_pat->program_descriptor[i].network_or_program_map_PID;
				break;
			}
		}
		if(nPid != -1 ) {
			pgm.nPid = nPid;
			pgm.pmt_callback = pmtCallback;
			m_demuxComp->SetOption(m_demuxComp, DEMUX_CMD_SUBSCRIBE_PROGRAM_PID, (char *) &pgm);
		}
	}
}

void CUdpClntBridge::UnsubscribeProgram(int strmId)
{

    if(m_pat != NULL && strmId < m_pat->number_of_programs ) {
        DemuxSubscribeProgramPidT pgm;
        pgm.nPid = m_pat->program_descriptor[strmId].network_or_program_map_PID;
        pgm.pmt_callback = NULL;
        m_demuxComp->SetOption(m_demuxComp, DEMUX_CMD_SUBSCRIBE_PROGRAM_PID, (char *)&pgm);
    }
}

int CUdpClntBridge::StartStreaming()
{
	TRACE_ENTER

	int nResult = 0;
	int nClock;
	unsigned char ucPlType;

	DemuxSelectProgramT m_demuxArgs;
	DemuxSelectProgramT *pdemuxArgs = &m_demuxArgs;

	//
	// Open Source
	//
	if (strncmp(m_szRemoteHost, UDP_SRC_PREFIX, strlen(UDP_SRC_PREFIX)) == 0 ) {
		JDBG_LOG(CJdDbg::LVL_TRACE, ("Using UDP Source"));
		m_srcComp = udprxCreate();
	} else {
		JDBG_LOG(CJdDbg::LVL_TRACE, ("Using File Source"));
		m_srcComp = filesrcCreate();
	}
	if( m_srcComp->Open(m_srcComp, m_szRemoteHost) != 0) {
		nResult = -1;
		goto EXIT;
	}

	//
	// Open Demux
	//
	// TODO
	m_pConnCtxSrcToDemux = CreateStrmConn(DMA_READ_SIZE, 64);
	m_demuxComp = demuxCreate();
	m_demuxComp->Open(m_demuxComp, NULL);
	m_demuxComp->SetOption(m_demuxComp, DEMUX_CMD_SET_PAT_CALLBACK, (char *)patCallback);
	m_demuxComp->SetOption(m_demuxComp, DEMUX_CMD_SET_PMT_CALLBACK, (char *)pmtCallback);
	m_demuxComp->SetOption(m_demuxComp, DEMUX_CMD_SET_PSI_CALLBACK_CTX, (char *) this);
	m_demuxComp->SetOption(m_demuxComp, DEMUX_CMD_SET_FMT_CALLBACK, (char *)formatCallback);

	m_srcComp->SetOutputConn(m_srcComp, 0, m_pConnCtxSrcToDemux);
	m_demuxComp->SetInputConn(m_demuxComp, 0, m_pConnCtxSrcToDemux);


	//
	// Start
	//
	m_srcComp->Start(m_srcComp);
	m_demuxComp->Start(m_demuxComp);

EXIT:
	TRACE_LEAVE
	return nResult;
}

int CUdpClntBridge::ConnectStreamForPid(int nPid, ConnCtxT *pConn)
{
	if(m_demuxComp) {
		m_demuxComp->SetOutputConn(m_demuxComp, nPid, pConn);
	}
	return 0;
}


int CUdpClntBridge::StopStreaming()
{
	TRACE_ENTER
	void *res;
	m_fRun = 0;

	if(m_srcComp) {
		m_srcComp->Stop(m_srcComp);
	}

	if(m_demuxComp) {
		m_demuxComp->Stop(m_demuxComp);
	}


	// TODO Dis Conn

	if(m_srcComp) {
		m_srcComp->Close(m_srcComp);
		m_srcComp->Delete(m_srcComp);
		m_srcComp = NULL;
	}

	if(m_demuxComp) {
		m_demuxComp->Delete(m_demuxComp);
		m_demuxComp = NULL;
	}
	if(m_thrdHandle){
		jdoalThreadJoin((void *)m_thrdHandle, 3000);
	}

	TRACE_LEAVE
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
