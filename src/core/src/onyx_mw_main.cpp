/*
 *
 *  Copyright 2010 MCN Technologies Inc.. All rights reserved.
 *
 */
/** @file onyx_mw_main.cpp
 *  @brief Onyx middle ware main.
 *
 *  The classes in this file provide the iniitalization
 *  and configuration of streaming components of the 
 *  software stack.
 *
 *  @author Ram Penke
 *  @bug No known bugs.
 */

#ifdef WIN32
#include <winsock2.h>
#else // Linux
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef PLATFORM_ANDROID
#include <execinfo.h>
#endif

#endif
#include <assert.h>
#include "minini.h"

#ifdef HAS_PLUGIN_RTSPSRV
#include "JdRtspSrv.h"
#endif

#ifdef HAS_PLUGIN_RTSPCLTNREC
#include "JdRtspClntRec.h"
#endif

#include "JdDbg.h"
#include "strmconn.h"
#include "strmconn_ipc.h"
#include "strmconn_zmq.h"
#include "sock_ipc.h"
#include "uimsg.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <signal.h>

#ifdef HAS_PLUGIN_OMX
#include "OpenMAXAL/OpenMAXAL.h"
#include  "onyx_omxext_api.h"
#include "OmxIf.h"
#endif

//#include "h264parser.h"
//#include "TsDemuxIf.h"
//#include "TsPsiIf.h"
//#include "tsfilter.h"

#include "StrmOutBridgeBase.h"

#ifdef HAS_PLUGIN_RTSPCLNT
#include "RtspClntBridge.h"
#endif

#ifdef HAS_PLUGIN_HLSCLNT
#include "HlsClntBridge.h"
#endif

#ifdef HAS_PLUGIN_RTSPPUBLISH
#include "RtspPublishBridge.h"
#endif

#ifdef HAS_PLUGIN_RTMPPUBLISH
#include "RtmpPublishBridge.h"
#endif

#ifdef HAS_PLUGIN_UDPSRV
#include "UdpSrvBridge.h"
#endif

#ifdef HAS_PLUGIN_RTPSRV
#include "RtpSrvBridge.h"
#endif

#ifdef HAS_PLUGIN_HLSSRV
#include "HlsSrvBridge.h"
#endif

#ifdef HAS_PLUGIN_MDPSRV
#include "MpdSrvBridge.h"
#endif

#include "Mpd.h"

#include "MediaSwitch.h"
#include "onyx_mw_util.h"



#ifdef HAS_PLUGIN_HLSSRV
#include "JdHttpSrv.h"
#include "AccessUnit.h"
#include "SimpleTsMux.h"
#include "JdHttpLiveSgmt.h"
#endif

#include "AvMixure.h"

#define MAX_PATH                256
#define MAX_CODEC_NAME_SIZE		128

#define RTSP_PREFIX "rtsp://"
#define HLS_PREFIX "http://"

#define XA_MIME_MP2TS  "mp2t"
#define XA_MIME_YUY2   "yuy2"

#define SESSION_CMD_STOP   1
#define SESSION_CMD_PAUSE  2
#define SESSION_CMD_RUN    3
//#define NO_DEBUG_LOG

static int  modDbgLevel = CJdDbg::LVL_TRACE;


#define OUTPUT_STREAM_PREFIX    "output"
#define ONYX_MW_SECTION         "onyx_mw"
#define EN_IPC_STRM_CONN
#define EN_IPC_STRM_ZMQ
class CRtspPublishBridge;
/**
 *  A utility class to read stream informtion from configuration file
 */
class CStreamUtil
{
public:
	/**
	 *  Converts configuration parameters to a URI and returns enumerated id for the type of stream.
	 *  @param pszSection(in) configuration section for the stream
 	 *  @param pszConfFile(in) configuration file
	 *  @returns Stream Context
	 */
	static CAvmixInputStrm *GetStreamParamsFromCfgDb( const char *pszSection, const char *pszConfFile);
	/**
	 *  Initializes stream parameters 
	 *  @param pInputStream(in/out) stores the state and configuration of the session
	 *  @param nSessionId(in) identifies the stream in configuration file
 	 *  @param pszConfFile(in) configuration file
	 *  @param nInputType(in) identifies stream type
	 *  @returns 0 on success
	 */
	static int InitInputStrm(CInputStrmBase *pInputStream, int nSessionId);

	/**
	 *  Uninitializes stream parameters
	 */
	static void DeinitInputStrm(CInputStrmBase *pInputStream);
};



CRtspPublishConfig::CRtspPublishConfig(const char *pszConfFile)
{
	ini_gets(SECTION_RTSP_PUBLISH, KEY_RTSP_PUBLISH_HOST, "", szRtspServerAddr, 64, pszConfFile);
	ini_gets(SECTION_RTSP_PUBLISH, KEY_RTSP_PUBLISH_APPLICATION, "", szApplicationName, 64, pszConfFile);
	usRtpLocalPort =  (unsigned short)ini_getl(SECTION_RTSP_PUBLISH, "local_port",   59500, pszConfFile);
	usRtpRemotePort =  (unsigned short)ini_getl(SECTION_RTSP_PUBLISH, "remote_port",   49500, pszConfFile);
	usServerRtspPort =  (unsigned short)ini_getl(SECTION_RTSP_PUBLISH, KEY_RTSP_PUBLISH_RTSP_PORT,   1935, pszConfFile);
}

CRtmpPublishConfig::CRtmpPublishConfig(const char *pszConfFile)
{
	ini_gets(SECTION_RTSP_PUBLISH, KEY_RTMP_PUBLISH_PRIMARY_HOST, "", szPrimarySrvAddr, 64, pszConfFile);
	usPrimarySrvPort = (unsigned short)ini_getl(SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_PRIMARY_PORT,   1935, pszConfFile);
	ini_gets(SECTION_RTSP_PUBLISH, KEY_RTMP_PUBLISH_PRIMARY_APP, "", szPrimarySrvAppName, 64, pszConfFile);
	ini_gets(SECTION_RTSP_PUBLISH, KEY_RTMP_PUBLISH_PRIMARY_STRM, "", szPrimarySrvStrmName, 64, pszConfFile);
	ini_gets(SECTION_RTSP_PUBLISH, KEY_RTMP_PUBLISH_PRIMARY_USER, "", szPrimarySrvUserId, 64, pszConfFile);
	ini_gets(SECTION_RTSP_PUBLISH, KEY_RTMP_PUBLISH_PRIMARY_PASSWD, "", szPrimarySrvPasswd, 64, pszConfFile);

	ini_gets(SECTION_RTSP_PUBLISH, KEY_RTMP_PUBLISH_SECONDARY_HOST, "", szSecondarySrvAddr, 64, pszConfFile);
	usSecondarySrvPort = (unsigned short)ini_getl(SECTION_RTMP_PUBLISH, KEY_RTMP_PUBLISH_SECONDARY_PORT,   1935, pszConfFile);
	ini_gets(SECTION_RTSP_PUBLISH, KEY_RTMP_PUBLISH_SECONDARY_APP, "", szSecondarySrvAppName, 64, pszConfFile);
	ini_gets(SECTION_RTSP_PUBLISH, KEY_RTMP_PUBLISH_SECONDARY_STRM, "", szSecondarySrvStrmName, 64, pszConfFile);
	ini_gets(SECTION_RTSP_PUBLISH, KEY_RTMP_PUBLISH_SECONDARY_USER, "", szSecondarySrvUserId, 64, pszConfFile);
	ini_gets(SECTION_RTSP_PUBLISH, KEY_RTMP_PUBLISH_SECONDARY_PASSWD, "", szSecondarySrvPasswd, 64, pszConfFile);

}



CRtspSrvConfig::CRtspSrvConfig(const char *pszConfFile)
{
	ini_gets(SECTION_RTSP_SERVER, KEY_RTSP_SERVER_STREAM, "", szStreamName, 64, pszConfFile);
	usServerRtspPort =  (unsigned short)ini_getl(SECTION_RTSP_SERVER, KEY_RTSP_SERVER_RTSP_PORT,   554, pszConfFile);
}

CRtpServerConfig::CRtpServerConfig(const char *pszConfFile)
{
	//ini_gets(SECTION_RTSP_SERVER, KEY_RTSP_SERVER_STREAM, "", m_szStreamName, 64, pszConfFile);
	m_usServerPort =  (unsigned short)ini_getl(SECTION_RTSP_SERVER, KEY_RTSP_SERVER_RTSP_PORT,   554, pszConfFile);
}

CUdpServerConfig::CUdpServerConfig(const char *pszConfFile)
{
	//ini_gets(SECTION_RTSP_SERVER, KEY_RTSP_SERVER_STREAM, "", m_szStreamName, 64, pszConfFile);
	m_usServerPort =  (unsigned short)ini_getl(SECTION_RTSP_SERVER, KEY_RTSP_SERVER_RTSP_PORT,   554, pszConfFile);
}

CHlsServerConfig::CHlsServerConfig(const char *pszConfFile)
{
	ini_gets(SECTION_HLS_SERVER, "application", "", m_szApplicationName, 64, pszConfFile);
	m_usServerRtspPort =  (unsigned short)ini_getl(SECTION_HLS_SERVER, "port", 8080, pszConfFile);
	m_fInternaHttpSrv = ini_getl(SECTION_HLS_SERVER, KEY_HLS_SRVR_INTERNAL, 0, pszConfFile);
	ini_gets(SECTION_HLS_SERVER, KEY_HLS_SRVR_ROOT, "", m_szServerRoot, 128, pszConfFile);
	ini_gets(SECTION_HLS_SERVER, KEY_HLS_FOLDER, "", m_szFolder, 128, pszConfFile);
	ini_gets(SECTION_HLS_SERVER, KEY_HLS_STREAM, "", m_szStream, 128, pszConfFile);
	m_nSegmentDuration = ini_getl(SECTION_HLS_SERVER, KEY_HLS_SEGMENT_DURATION, 4000, pszConfFile);
	m_fLiveOnly = ini_getl(SECTION_HLS_SERVER, KEY_HLS_LIVE_ONLY, 1, pszConfFile);
	ini_gets(SECTION_HLS_SERVER, KEY_HLS_INPUT0, "", m_szInput0, 64, pszConfFile);
}


CHlsPublishCfg::CHlsPublishCfg(const char *pszConfFile)
{
	int nInputs;

	ini_gets(SECTION_HLS_PUBLISH, KEY_HLS_PROTOCOL, "", m_szProtocol, 32, pszConfFile);
	ini_gets(SECTION_HLS_PUBLISH, KEY_HLS_FOLDER, "", m_szFolder, 128, pszConfFile);
	ini_gets(SECTION_HLS_PUBLISH, KEY_HLS_STREAM, "", m_szStream, 128, pszConfFile);
	ini_gets(SECTION_HLS_PUBLISH, KEY_MPD_HTTP_SERVER, "", m_pszHost, 128, pszConfFile);
	ini_gets(SECTION_HLS_PUBLISH, KEY_MPD_S3_BUCKET, "", m_pszBucket, 128, pszConfFile);
	ini_gets(SECTION_HLS_PUBLISH, KEY_MPD_ACCESS_ID, "", m_szAccessId, 128, pszConfFile);
	ini_gets(SECTION_HLS_PUBLISH, KEY_MPD_SECURITY_KEY, "", m_szSecKey, 128, pszConfFile);
	m_fLiveOnly = ini_getl(SECTION_HLS_PUBLISH, "live_only", 1, pszConfFile);
	m_nSegmentDuration = ini_getl(SECTION_HLS_PUBLISH, "segment_duration", 4000, pszConfFile);
	m_nDdebugLevel = ini_getl(SECTION_HLS_PUBLISH, "debug_level", 1, pszConfFile);
	ini_gets(SECTION_HLS_PUBLISH, KEY_HLS_INPUT0, "", m_szInput0, 64, pszConfFile);
	nInputs = ini_getl(SECTION_MPD_PUBLISH, KEY_MPD_INPUT_COUNT, 0, pszConfFile);
}


static void DumpHex(unsigned char *pData, int nSize)
{
	int i;
	for (i=0; i < nSize; i++)
		printf("%02x ", pData[i]);
	printf("\n");
}

static void SignalHandler(int sig)
{
	DBG_PRINT("!!! omax_qt_app:SignalHandler:Recevied Siganl !!! %d\n", sig);
	CUiMsg::GetSingleton()->Abort();
}

#ifndef WIN32
static void SegvHandler(int sig) {
	void *array[10];
	size_t size;

	// get void*'s for all entries on the stack
	size = backtrace(array, 10);

	// print out all the frames to stderr
	fprintf(stderr, "Error: signal %d:\n", sig);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(1);
}
#endif


#define MAX_SESSIONS 6

/**
 *  A wrapper class to parse a ts file ans set the stream parameters.
 *  This calss is used only for initialization.
 *  Stream parsing during playback is deleteated to decoder subsystem in the codec library
 */
class  CLocalFileSrc : public CStrmInBridgeBase
{
public:
	CLocalFileSrc(char *pszFileName, int fEnableAud, int fEnableVid, int nSelectProg, int *pResult)
		:CStrmInBridgeBase(fEnableAud, fEnableVid)
	{
		int res= -1;
		long hr;
		CH264Stream *pVidSrc = NULL;
		CAacStream *pAudSrc = NULL;

		CTsFilter  *pTsFilter = CTsFilter::CreateInstance(&hr);
		pTsFilter->InitFileSrc(pszFileName);
		if(pTsFilter->InitProgram(nSelectProg, &pVidSrc, &pAudSrc) != 0) {
			DBG_PRINT("CreatePlayerSession:Could not locate program %d in %s...\n", nSelectProg, pszFileName);
			delete pTsFilter;
			goto Exit;
		}
		if(pVidSrc->m_nSpsSize > 0) {
			H264::cParser Parser;
			if(Parser.ParseSequenceParameterSetMin(pVidSrc->m_Sps,pVidSrc->m_nSpsSize, &m_lWidth, &m_lHeight) != 0){
				goto Exit;
			}
		}
		mDataLocatorUri.locatorType = XA_DATALOCATOR_URI;
		mDataLocatorUri.pURI = (XAchar *) pszFileName;

		mFormatVideo.formatType = XA_DATAFORMAT_MIME;
		mFormatVideo.pMimeType = (XAchar *) XA_MIME_MP2TS;
		mFormatVideo.containerType = XA_CONTAINERTYPE_MPEG_TS;

		mDataSrcVideo.pLocator = &mDataLocatorUri;
		mDataSrcVideo.pFormat = &mFormatVideo;

		res = 0;
Exit:
		if(pTsFilter)
				delete pTsFilter;
		*pResult = res;
	}
	int StartStreaming(void) {return 0;}
    int StopStreaming(void) {return 0;}
};

/**
 *  A wrapper class for supplying SDI capture parameters.
 *  Audio video data flows on the HW interface i.e. streming is
 *  not handled by this class.
 */

class  CSdiCaptureSrc : public CStrmInBridgeBase
{
public:
	CSdiCaptureSrc(char *pszDeviceName, int fEnableAud, int fEnableVid, int nSelectChan, int *pResult)
		:CStrmInBridgeBase(fEnableAud, fEnableVid)
	{
		int res= -1;
		mDataLocatorUri.locatorType = XA_DATALOCATOR_URI;
		mDataLocatorUri.pURI = (XAchar *) pszDeviceName;

		mFormatVideo.formatType = XA_DATAFORMAT_MIME;
		mFormatVideo.pMimeType = (XAchar *) XA_MIME_YUY2;
		mFormatVideo.containerType = XA_CONTAINERTYPE_RAW;

		// TODO: Get width from Capture
		//if(pInputStream->nWidth == 0) 
		//	pInputStream->nWidth = lWidth;
		//if(pInputStream->nHeight == 0) 
		//	pInputStream->nHeight = lHeight;

		mDataSrcVideo.pLocator = &mDataLocatorUri;
		mDataSrcVideo.pFormat = &mFormatVideo;
		res = 0;
		*pResult = res;
	}
	int StartStreaming(void) {return 0;}
    int StopStreaming(void) {return 0;}

};

class  CStrmConnWrapper : public CStrmInBridgeBase
{
public:
	CStrmConnWrapper(int fEnableAud, int fEnableVid, ConnCtxT *pAudCompOut, ConnCtxT *pVidCompOut, int *pResult)
		:CStrmInBridgeBase(fEnableAud, fEnableVid)
	{
		int res= -1;
		if(pVidCompOut) {
			CreateH264OutputPin(pVidCompOut);
		}
		if(pAudCompOut) {
			CreateMP4AOutputPin(pAudCompOut);
		}

		res = 0;
Exit:
		*pResult = res;
	}
	int StartStreaming(void) {return 0;}
    int StopStreaming(void) {return 0;}

};

class  CIpcUdpSrc : public CStrmInBridgeBase
{
public:
	CIpcUdpSrc(
		char *pszAudSockLocal, 
		char *pszAudSockPeer, 
		char *pszVidSockLocal, 
		char *pszVidSockPeer, 
		int fEnableAud, int fEnableVid, int *pResult)
		:CStrmInBridgeBase(fEnableAud, fEnableVid)
	{
		int res= -1;

#ifdef EN_IPC_STRM_CONN
		m_pVidConn = CreateIpcStrmConn(pszVidSockLocal, pszVidSockPeer, 1024*1024);
		if(m_pVidConn)
			CreateH264OutputPin(m_pVidConn);
		m_pAudConn = CreateIpcStrmConn(pszAudSockLocal, pszAudSockPeer, 16*1024);
		if(m_pAudConn)
			CreateMP4AOutputPin(m_pAudConn);
#endif

		*pResult = res;
	}
	~CIpcUdpSrc()
	{
		DeleteIpcStrmConn(m_pVidConn);
		DeleteIpcStrmConn(m_pAudConn);
	}
	int StartStreaming(void) {return 0;}
    int StopStreaming(void) {return 0;}
private:
	ConnCtxT *m_pVidConn;
	ConnCtxT *m_pAudConn;
};


class  CIpcZmqSrc : public CStrmInBridgeBase
{
public:
	CIpcZmqSrc(
		char *pszAudSockPeer, 
		char *pszVidSockPeer, 
		int fEnableAud, int fEnableVid, int *pResult)
		:CStrmInBridgeBase(fEnableAud, fEnableVid)
	{
		int res= -1;

#ifdef EN_IPC_STRM_ZMQ
		m_pVidConn = CreateZmqStrmConn(0, pszVidSockPeer, 1024*1024);
		if(m_pVidConn)
			CreateH264OutputPin(m_pVidConn);
		m_pAudConn = CreateZmqStrmConn(0, pszAudSockPeer, 16*1024);
		if(m_pAudConn)
			CreateMP4AOutputPin(m_pAudConn);
#endif

		*pResult = res;
	}
	~CIpcZmqSrc()
	{
#ifdef ENABLE_ZMQ
		DeleteZmqStrmConn(m_pVidConn);
		DeleteZmqStrmConn(m_pAudConn);
#endif
	}
	int StartStreaming(void) {return 0;}
    int StopStreaming(void) {return 0;}
private:
	ConnCtxT *m_pVidConn;
	ConnCtxT *m_pAudConn;
};



#define MAX_PUBLISH_SWICTHES           2
#define MAX_PUBLISH_SWICTH_INPUTS      2
/*
** Onyx middleware main class
*/

class COnyxMw
{
public:
	COnyxMw();
	int Run(const char *pszConfFile, int nLayoutOption);

private:	
	int StartSession(const char *pszConfFile, int nLayoutOption);

	int StartAvMixers(const char *pszConfFile, int nLayoutOption);
	int SetupPublishSwiches(const char *pszConfFile);
	void StartPublishSwiches(const char *pszConfFile);
	int StopPublishSwiches();
	int StartServers(const char *pszConfFile);
	int GetSwitchesStats(switchesStats_t *pStats);

	int StartRtspServer(const char *pszConfFile);
	int StartHlsServer(const char *pszConfFile);
	int StartMpdServer(const char *pszConfFile);

	int CloseSession();
	int StopPublishStreams();
	int StopAvMixers();
	//int StopEncodeSubsystem(int nSwitchId);
	//int StopDecodeSubsystem();
	void GetPublishConfig(const char *pszConfFile);
	void SetModuleDbgLvl(int ModId, int nDbgLevel);

	int SetPublishSwitchSrc(const char *szSwitchId, int nSrcId, const char *pszConfFile);
	int StartPublishSwitch(const char *pszSwitchId);
	int StopPublishSwitch(const char *pszSwitchId);

	CAvMixer *GetAvMixer(const char *szId);
private:	
	char               *pszConfFile;

	
	CRtspPublishBridge  *m_pRtspSrvBridge;
	CHlsSrvBridge       *m_pHlsSrvBridge;
	CMpdSrvBridge       *m_pMpdSrvBridge;
#ifdef ENABLE_RTMP
	CRtmpPublishBridge	*m_pRtmpPublishBridge;
#endif
	CRtpSrvBridge       *m_pRtpSrvBridge;
	CUdpSrvBridge       *m_pUdpSrvBridge;

	CRtspCommonConfig   *m_pRtspCommonCfg;
	CRtspSrvConfig      *m_pRtspSvrCfg;
	CRtspPublishConfig  *m_pRtspPublishCfg;
	CRtpServerConfig    *m_pRtpSrvCfg;
	CUdpServerConfig    *m_pUdpSrvCfg;

	CRtmpPublishConfig  *m_pRtmpPublishCfg;
	CHlsServerConfig    *m_pHlsSvrCfg;
	CHlsPublishCfg	    *m_pHlsPublishCfg;
	CMpdRoot             *m_pMpdRoot;

	int                 m_fEnableRtspPublish;
	int                 m_fEnableRtspSrv;
	int                 m_fEnableHlsSvr;
	int                 m_fEnableHlsPublish;
	int                 m_fEnableMpdSvr;
	int                 m_fEnableMpdPublish;

	int                 m_fEnableRtmpPublish;
	int                 m_fEnableRtpSrv;
	int                 m_fEnableUdpSrv;
	time_t              session_start_time;
	COutputStream       *m_pOutputStream;
	std::map <std::string, CAvMixer *>      m_listAvMixers;//*m_pAvMixer;	
	std::map <std::string, CMediaSwitch *>  m_listPublishSwitches;
};


void COnyxMw::GetPublishConfig(const char *pszConfFile)
{
	m_fEnableHlsSvr =  ini_getl(SECTION_SERVERS, EN_HLS_SERVER, 0, pszConfFile);
	m_fEnableHlsPublish =  ini_getl(SECTION_SERVERS, EN_HLS_PUBLISH, 0, pszConfFile);
	m_fEnableMpdSvr =  ini_getl(SECTION_SERVERS, EN_MPD_SERVER, 0, pszConfFile);
	m_fEnableMpdPublish =  ini_getl(SECTION_SERVERS, EN_MPD_PUBLISH, 0, pszConfFile);

	m_fEnableRtspPublish =  ini_getl(SECTION_SERVERS, EN_RTSP_PUBLISH, 0, pszConfFile);
	m_fEnableRtspSrv = ini_getl(SECTION_SERVERS, EN_RTSP_SERVER, 0, pszConfFile);
	m_fEnableRtmpPublish = ini_getl(SECTION_SERVERS, EN_RTMP_PUBLISH, 0, pszConfFile);
	m_fEnableRtpSrv = ini_getl(SECTION_SERVERS, EN_RTP_SERVER, 0, pszConfFile);
	m_fEnableUdpSrv = ini_getl(SECTION_SERVERS, EN_UDP_SERVER, 0, pszConfFile);
}

COnyxMw::COnyxMw()
{
	m_fEnableRtspPublish = 0;
	m_fEnableRtmpPublish = 0;
	m_fEnableRtspSrv = 0;
	m_fEnableHlsSvr = 0;
	m_fEnableHlsPublish = 0;
	m_fEnableMpdSvr = 0;
	m_fEnableMpdPublish = 0;

	m_pHlsSrvBridge = NULL;
	m_pMpdSrvBridge = NULL;
	m_pMpdRoot = NULL;
	m_pRtspSrvBridge = NULL;
#ifdef ENABLE_RTMP
	m_pRtmpPublishBridge = NULL;
#endif
	m_fEnableRtpSrv = 0;
	m_fEnableUdpSrv = 0;

	
	m_pHlsPublishCfg = NULL;
	m_pRtspPublishCfg = NULL;
	m_pRtmpPublishCfg = NULL;
	m_pHlsSvrCfg = NULL;

	m_pRtspCommonCfg = NULL;
	m_pRtspSvrCfg = NULL;
	m_pHlsSvrCfg = NULL;
	m_pRtpSrvCfg = NULL;
	m_pUdpSrvCfg = NULL;
	m_pRtpSrvBridge = NULL;
	m_pUdpSrvBridge = NULL;

	m_pOutputStream = NULL;
};


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
		ini_gets(pszSection, "location", "", szInput, MAX_FILE_NAME_SIZE, pszConfFile);
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

int GetPlayerSessionUserOverrides(CAvmixInputStrm *pInputStream, const char *pszConfFile, const char *szSessionName, int fEnableAudio)
{
	//if(strcmp(pInputStream->vid_codec_name, "h264") == 0)
	{
		pInputStream->nWidth =  ini_getl(szSessionName, "width",   0, pszConfFile);
		pInputStream->nHeight =  ini_getl(szSessionName, "height",  0, pszConfFile);
		pInputStream->nFrameRate =  ini_getl(szSessionName, "framerate",  30, pszConfFile);
		pInputStream->nDeinterlace =  ini_getl(szSessionName, "deinterlace",  0, pszConfFile);
		pInputStream->nSelectProg =  ini_getl(szSessionName, "program",  0, pszConfFile);
	}
	if(fEnableAudio) {
		ini_gets(szSessionName, "aud_codec", "", pInputStream->aud_codec_name, MAX_CODEC_NAME_SIZE, pszConfFile);
		if(strcmp(pInputStream->aud_codec_name, "g711u") == 0){
			pInputStream->fEnableAud = 1;
		}
	} else {
		strcpy(pInputStream->aud_codec_name, "null");
		pInputStream->fEnableAud = 0;
	}
	return 0;
}

int COnyxMw::StartSession(const char *pszConfFile, int nLayoutOption)
{
	int res = 0;
	JDBG_LOG(CJdDbg::LVL_TRACE,("Enter"));
	signal(SIGINT, SignalHandler);
	signal(SIGTERM, SignalHandler);
#ifndef WIN32
	signal(SIGSEGV, SegvHandler);
#endif

	char               szOutputSection[MAX_FILE_NAME_SIZE];
	sprintf(szOutputSection,"%s%d", OUTPUT_STREAM_PREFIX, 1);
	m_pOutputStream = new COutputStream(ONYX_PUBLISH_FILE_NAME, szOutputSection);

	GetPublishConfig(ONYX_PUBLISH_FILE_NAME);

	time((time_t *) &session_start_time);

	StartAvMixers(pszConfFile, nLayoutOption);
	SetupPublishSwiches(pszConfFile);
	StartServers(pszConfFile);
	StartPublishSwiches(pszConfFile);
	JDBG_LOG(CJdDbg::LVL_TRACE,("Leave"));	
	return res;
}

int COnyxMw::StartAvMixers(const char *pszConfFile, int nLayoutOption)
{
	char szAvMixerIdSection[128];
	int nNumAvMixers =  ini_getl(SECTION_GLOBAL, KEY_MW_AVMIXERS,   0, pszConfFile);
	JDBG_LOG(CJdDbg::LVL_SETUP,("nNumAvMixers=%d", nNumAvMixers));
	for(int i=0; i < nNumAvMixers; i++) {
		int nInputs = 0;
		sprintf(szAvMixerIdSection,"%s%d",AVMIXER_PREFIX, i);

		CAvMixer *pAvMixer = new CAvMixer;
		JDBG_LOG(CJdDbg::LVL_SETUP,("Starting %s", szAvMixerIdSection));
		pAvMixer->Start(pszConfFile, szAvMixerIdSection, nLayoutOption);
		m_listAvMixers[szAvMixerIdSection] = pAvMixer;
	}
	return 0;
}

int COnyxMw::StartPublishSwitch(const char *pszSwitchId)
{
	CMediaSwitch *pPublishSwitch = NULL;
	std::map<std::string, CMediaSwitch *>::iterator it = m_listPublishSwitches.find(pszSwitchId);

	if(m_listPublishSwitches.end() != it){
		pPublishSwitch = (*it).second;
		pPublishSwitch->Run();
	} else {
		JDBG_LOG(CJdDbg::LVL_ERR,("!!!  switch_id=%s not mapped !!!\n", pszSwitchId));
		return -1;
	}
	return 0;
}

int COnyxMw::StopPublishSwitch(const char *pszSwitchId)
{
	CMediaSwitch *pPublishSwitch = NULL;
	std::map<std::string, CMediaSwitch *>::iterator it = m_listPublishSwitches.find(pszSwitchId);

	if(m_listPublishSwitches.end() != it){
		pPublishSwitch = (*it).second;
		pPublishSwitch->Stop();
	} else {
		JDBG_LOG(CJdDbg::LVL_ERR,("!!!  switch_id=%s not mapped !!!\n", pszSwitchId));
		return -1;
	}
	return 0;
}


int COnyxMw::SetPublishSwitchSrc(const char *pszSwitchId, int nSrcId, const char *pszConfFile)
{
	INPUT_TYPE_T  nInputType;
	char          szSwitchInputId[64];
	char          szInputSourceId[64];
	char          szInputUri[64];
	// TODO: Modify
	CMediaSwitch *pPublishSwitch = NULL;
	std::map<std::string, CMediaSwitch *>::iterator it = m_listPublishSwitches.find(pszSwitchId);

	if(m_listPublishSwitches.end() != it){
		pPublishSwitch = (*it).second;
	} else {
		JDBG_LOG(CJdDbg::LVL_ERR,("!!!  switch_id=%s not mapped !!!\n", pszSwitchId));
		return -1;
	}
	JDBG_LOG(CJdDbg::LVL_SETUP,("SwitchId=%s nSrcId=%d", pszSwitchId, nSrcId));
	CAvmixInputStrm *pSwitchpInput = pPublishSwitch->m_pSwitchInput;
	//pPublishSwitch->Stop();
	if(pSwitchpInput){
		CStreamUtil::DeinitInputStrm(pSwitchpInput);
		delete pSwitchpInput;
		pPublishSwitch->m_pSwitchInput = NULL;
	}

	sprintf(szSwitchInputId, "%s%d",SECTION_SWITCH_INPUT_ID_PREFIX, nSrcId);
	JDBG_LOG(CJdDbg::LVL_SETUP,("SwitchId=%s szSwitchInputId=%s", pszSwitchId, szSwitchInputId));
	ini_gets(pszSwitchId, szSwitchInputId, "", szInputSourceId, 63, pszConfFile);
	JDBG_LOG(CJdDbg::LVL_SETUP,("SwitchId=%s szSwitchInputId=%s szInputSourceId=%s", pszSwitchId, szSwitchInputId, szInputSourceId));
	if(strlen(szInputSourceId)) {
		int nRes = 0;
		pSwitchpInput = CStreamUtil::GetStreamParamsFromCfgDb(szInputSourceId, pszConfFile);
		nInputType = pSwitchpInput->nInputType;

		if(nInputType == INPUT_TYPE_AVMIXER) {
			CAvMixer *pAvMixer = GetAvMixer(pSwitchpInput->pszInputUri);
			if(pAvMixer){
				pSwitchpInput->mpInputBridge = pAvMixer->GetOutputConn();
			}
		} else {
			nRes  = CStreamUtil::InitInputStrm(pSwitchpInput, nSrcId);
		}
		if (nRes == 0 && pSwitchpInput->mpInputBridge) {
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
			JDBG_LOG(CJdDbg::LVL_ERR,("!!! Failed to set input switch_id=%s src_id=%d !!!\n", pszSwitchId, nSrcId));
			delete pSwitchpInput;
		}
	} else {
		JDBG_LOG(CJdDbg::LVL_ERR,("!!! Source not specified SwitchId=%s SrcId=%d !!!\n", pszSwitchId, nSrcId));
	}
	return 0;
}


int COnyxMw::StartRtspServer(const char *pszConfFile)
{
	int res = 0;
	int i;
	JDBG_LOG(CJdDbg::LVL_TRACE,("Enter"));
	CMediaSwitch *pPublishSwitch = NULL;

	m_pRtspSrvBridge = new CRtspPublishBridge;
	m_pRtspCommonCfg = new CRtspCommonConfig(ONYX_PUBLISH_FILE_NAME);

	std::map<std::string, CMediaSwitch *>::iterator it = m_listPublishSwitches.find(m_pRtspCommonCfg->m_szInput0);

	if(m_listPublishSwitches.end() != it){
		pPublishSwitch = (*it).second;
	} else {
		return -1;
	}

	if(m_fEnableRtspPublish)
		m_pRtspPublishCfg = new CRtspPublishConfig(ONYX_PUBLISH_FILE_NAME);

	if(m_fEnableRtspSrv)
		m_pRtspSvrCfg = new CRtspSrvConfig(ONYX_PUBLISH_FILE_NAME);

	m_pRtspSrvBridge->SetStreamCfg(m_pRtspCommonCfg);
	m_pRtspSrvBridge->Init(m_pOutputStream);

	if(m_fEnableRtspSrv) {
		m_pRtspSrvBridge->SetRtspServerCfg(m_pRtspSvrCfg);
		m_pRtspSrvBridge->StartRtspServer();
	}
	if(m_fEnableRtspPublish) {
		m_pRtspSrvBridge->SetPublishServerCfg(m_pRtspPublishCfg);
		m_pRtspSrvBridge->ConnectToPublishServer();
	}

	if(m_pRtspSrvBridge)
		pPublishSwitch->AddOutput(m_pRtspSrvBridge);

	JDBG_LOG(CJdDbg::LVL_TRACE,("Leave"));	
	return res;
}

int COnyxMw::StartHlsServer(const char *pszConfFile)
{
	int res = 0;
	int i;
	JDBG_LOG(CJdDbg::LVL_TRACE,("Enter"));
	CMediaSwitch *pPublishSwitch = NULL;

	m_pHlsSrvBridge = new CHlsSrvBridge;

	m_pHlsSvrCfg = new CHlsServerConfig(ONYX_PUBLISH_FILE_NAME);
	m_pHlsSrvBridge->SetServerConfig(m_pHlsSvrCfg);

	std::map<std::string, CMediaSwitch *>::iterator it = m_listPublishSwitches.find(m_pHlsSvrCfg->m_szInput0);

	if(m_listPublishSwitches.end() != it){
		pPublishSwitch = (*it).second;
	} else {
		return -1;
	}

	pPublishSwitch->AddOutput(m_pHlsSrvBridge);

	m_pHlsSrvBridge->Run(m_pOutputStream);

	JDBG_LOG(CJdDbg::LVL_TRACE,("Leave"));	
	return res;
}

int COnyxMw::StartMpdServer(const char *pszConfFile)
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
	JDBG_LOG(CJdDbg::LVL_TRACE,("Enter"));

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
	JDBG_LOG(CJdDbg::LVL_TRACE,("Leave"));	
	return res;
}

int COnyxMw::StartServers(const char *pszConfFile)
{
	GetPublishConfig(ONYX_PUBLISH_FILE_NAME);
	if(m_fEnableRtspPublish || m_fEnableRtspSrv) {
		StartRtspServer(pszConfFile);
	}
	if(m_fEnableHlsSvr || m_fEnableHlsPublish) {
		StartHlsServer(pszConfFile);
	}
	
	if(m_fEnableMpdSvr) {
		StartMpdServer(pszConfFile);
	}

	if(m_fEnableRtpSrv){
	}

	if(m_fEnableUdpSrv){
	}

	if(m_fEnableRtmpPublish){
	}

	return 0;
}


int GetVidMixerConfig(
	const char       *szConfigFile,
	int              nLayoutId,
	DISP_WINDOW_LIST **ppWndList)
{
	int i;
	char szLayoutName[32];
	char szWndName[32];
	int nDefWidth = 720;
	int nDefHeight = 480;
	int nNumWindows = 0;
	DISP_WINDOW_LIST *pListWnd = (DISP_WINDOW_LIST *)malloc(sizeof(DISP_WINDOW_LIST));
	DISP_WINDOW *pWnd;

	sprintf(szLayoutName,"layout%d", nLayoutId);
	nNumWindows = ini_getl(szLayoutName, "windows", 1, szConfigFile); 
	pListWnd->nNumWnd = nNumWindows;
	if(nNumWindows > 0 && nNumWindows < 6) {
		pWnd = (DISP_WINDOW *)malloc(nNumWindows * sizeof(DISP_WINDOW));
		pListWnd->pWndList = pWnd;
		for (i=0; i < nNumWindows; i++) {
			char szWndDetails[256];
			sprintf(szWndName,"window%d", i);
			ini_gets(szLayoutName, szWndName, "",   szWndDetails, 256, szConfigFile);
			if(strlen(szWndDetails)) {
				sscanf(szWndDetails,"%d %d %d %d %d", &pWnd->nStrmSrc, &pWnd->nStartX, &pWnd->nStartY, &pWnd->nWidth, &pWnd->nHeight);
			}
			pWnd++;
		}
	}
	*ppWndList = pListWnd;
	return 0;
}

int GetAudMixerConfig(
	const char       *szConfigFile,
	int              nLayoutId,
	AUD_CHAN_LIST    **ppAChanList)
{
	int i;
	char szLayoutName[32];
	char szAChanName[32];
	int nNumWindows = 0;
	AUD_CHAN_LIST    *pAChanList = (AUD_CHAN_LIST *)malloc(sizeof(AUD_CHAN_LIST));
	AUD_CHAN_T *pAChan;
	pAChanList->nNumChan = 0;
	pAChanList->pAChanList = NULL;
	sprintf(szLayoutName,"layout%d", nLayoutId);
	nNumWindows = ini_getl(szLayoutName, "windows", 1, szConfigFile); 

	if(nNumWindows > 0 && nNumWindows < 6) {
		pAChan = (AUD_CHAN_T *)malloc(nNumWindows * sizeof(AUD_CHAN_T));
		memset(pAChan, 0x00, sizeof(AUD_CHAN_T));
		pAChanList->pAChanList = pAChan;
		for (i=0; i < nNumWindows; i++) {
			char szAChanDetails[256];
			sprintf(szAChanName,"aud%d", i + 1);
			ini_gets(szLayoutName, szAChanName, "",   szAChanDetails, 256, szConfigFile);
			if(strlen(szAChanDetails)) {
				sscanf(szAChanDetails,"%d %d", &pAChan->nStrmSrc, &pAChan->nVolPercent);
			}
			pAChan++;
			pAChanList->nNumChan++;
		}
	}

	*ppAChanList = pAChanList;
	return 0;
}

int COnyxMw::SetupPublishSwiches(const char *pszConfFile)
{
	int res = 0;
	int i;
	char szSwitchIdSection[128];
	char szSwitchInput[128];
	JDBG_LOG(CJdDbg::LVL_TRACE,("Enter"));
	int nNumSwitches =  ini_getl(SECTION_GLOBAL, KEY_MW_SWITCHES,   0, pszConfFile);
	for(int i=0; i < nNumSwitches; i++) {
		int nInputs = 0;
		sprintf(szSwitchIdSection,"%s%d",SWITCH_PREFIX, i);
		int nNumInputs =  ini_getl(szSwitchIdSection, SWITCH_INPUT_COUNT,   0, pszConfFile);
		if(nNumInputs) {
			CMediaSwitch *pPublishSwitch = new CMediaSwitch(szSwitchIdSection);
			m_listPublishSwitches[szSwitchIdSection] = pPublishSwitch;
			SetPublishSwitchSrc(szSwitchIdSection,0, pszConfFile);
		} else {
			JDBG_LOG(CJdDbg::LVL_ERR,("switch_id=%s not defined\n", szSwitchIdSection));
		}
	}

	JDBG_LOG(CJdDbg::LVL_TRACE,("Leave"));	
	return res;
}

void COnyxMw::StartPublishSwiches(const char *pszConfFile)
{
	JDBG_LOG(CJdDbg::LVL_TRACE,("Enter"));
	for(std::map<std::string, CMediaSwitch *>::iterator it = m_listPublishSwitches.begin(); it != m_listPublishSwitches.end(); ++it){
		CMediaSwitch *pPublishSwitch = it->second;
		pPublishSwitch->Run();
	}
	JDBG_LOG(CJdDbg::LVL_TRACE,("Leave"));	
}

int COnyxMw::GetSwitchesStats(switchesStats_t *pStats)
{
	int res = 0;
	int i = 0;
	int j = 0;
	char szSwitchIdSection[128];
	char szSwitchInput[128];
	JDBG_LOG(CJdDbg::LVL_TRACE,("Enter"));
	switchStats_t *pSwitchStat;
	
	pStats->nStartTime = (int)session_start_time;

	for(std::map<std::string, CMediaSwitch *>::iterator it = m_listPublishSwitches.begin(); it != m_listPublishSwitches.end(); ++it){
		CMediaSwitch *pPublishSwitch = it->second;
		pSwitchStat = &pStats->switchesStats[i];
		swicthPortStats_t *pInpuStat = &pSwitchStat->inputStats;
		pInpuStat->nVidStrmTime = pPublishSwitch->m_nVidStrmTime;
		pInpuStat->nAudStrmTime = pPublishSwitch->m_nAudStrmTime;
		pInpuStat->nErrors = 0;
		pInpuStat->nState = 0;
		j=0;
		for(std::vector<CStrmOutBridge *>::iterator it = pPublishSwitch->m_Outputs.begin(); it != pPublishSwitch->m_Outputs.end(); ++it){
			CStrmOutBridge *pOutBridge = *it;
			pOutBridge->UpdateStats();
			swicthPortStats_t *pOutStat = &pSwitchStat->outputStats[j];
			strncpy(pOutStat->szId, pOutBridge->m_szId, MAX_SWITCH_OUT_ID_SIZE - 1);
			pOutStat->nErrors = pOutBridge->m_nErrors;
			pOutStat->nVidStrmTime = pOutBridge->m_nVidStrmTime;
			pOutStat->nAudStrmTime = 0;
			pOutStat->nState = pOutBridge->m_nState; // TODO
			j++;
			if(j >= MAX_SWITCH_OUTPUTS)
				break;
		}
		pSwitchStat->nNumOutputs = j;
		if(pPublishSwitch->m_pSwitchInput) {
			//pPublishSwitch->m_pSwitchInput->
		}
		i++;
		if(i >= SWITCH_STAT_MAX_NUM)
			break;
	}
	pStats->nNumSwitches = i;
	JDBG_LOG(CJdDbg::LVL_TRACE,("Leave"));	
	return res;
}

int COnyxMw::StopPublishSwiches()
{
	int res = 0;
	int i;
	char szSwitchIdSestion[128];
	char szSwitchInput[128];
	JDBG_LOG(CJdDbg::LVL_TRACE,("Enter"));
	for(std::map<std::string, CMediaSwitch *>::iterator it = m_listPublishSwitches.begin(); it != m_listPublishSwitches.end(); ++it){
		CMediaSwitch *pPublishSwitch = it->second;

		CAvmixInputStrm *pSwitchpInput = pPublishSwitch->m_pSwitchInput;
		pPublishSwitch->Stop();
		if(pSwitchpInput){
			CStreamUtil::DeinitInputStrm(pSwitchpInput);
			delete pSwitchpInput;
			pPublishSwitch->m_pSwitchInput = NULL;
		}

		delete pPublishSwitch;
	}
	m_listPublishSwitches.clear();
	JDBG_LOG(CJdDbg::LVL_TRACE,("Leave"));	
	return res;
}

CAvMixer *COnyxMw::GetAvMixer(const char *szId)
{
	CAvMixer *pAvMixer = NULL;
	JDBG_LOG(CJdDbg::LVL_SETUP,("AvMixer ID=%s", szId));
	std::map<std::string, CAvMixer *>::iterator it = m_listAvMixers.find(szId);
	if(it != m_listAvMixers.end()) {
		pAvMixer = it->second;
	}
	return pAvMixer;
}
int COnyxMw::StopAvMixers()
{
	int res = 0;
	int i;
	char szAvMixerIdSestion[128];

	JDBG_LOG(CJdDbg::LVL_TRACE,("Enter"));
	for(std::map<std::string, CAvMixer *>::iterator it = m_listAvMixers.begin(); it != m_listAvMixers.end(); ++it){
		CAvMixer *pAvMixer = it->second;
		pAvMixer->Stop();
		delete pAvMixer;
	}
	m_listAvMixers.clear();
	JDBG_LOG(CJdDbg::LVL_TRACE,("Leave"));	
	return res;
}



int CStreamUtil::InitInputStrm(CInputStrmBase *pInputStream, int nSessionId)
{
	int res = -1;

	JDBG_LOG(CJdDbg::LVL_SETUP,("nSessionId=%d nInputType=%d Input=%s",nSessionId, pInputStream->nInputType, pInputStream->pszInputUri));	
	pInputStream->nCodecSessionId = nSessionId;

	
	// TODO: Make it configurable
	pInputStream->fEnableVid = 1;
	pInputStream->nCmd = SESSION_CMD_RUN;

	if(pInputStream->nInputType == INPUT_TYPE_HLS) {
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
	} else if(pInputStream->nInputType == INPUT_TYPE_RTSP) {
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
	} else if(pInputStream->nInputType == INPUT_TYPE_FILE){
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
	} else if(pInputStream->nInputType == INPUT_TYPE_CAPTURE){
#if (1) // PRODUCT_DVM
		int nResult;
		CSdiCaptureSrc *pCaptureSrc = new CSdiCaptureSrc(pInputStream->pszInputUri, pInputStream->fEnableAud, pInputStream->fEnableVid, 0/*TODO*/, &nResult);
		pInputStream->mpInputBridge =  pCaptureSrc;
#endif
	} else if(pInputStream->nInputType == INPUT_TYPE_STRMCONN) {
		int nResult;
		// TODO: Retrieve the following interface
		ConnCtxT *pVidCon = NULL; 
		ConnCtxT *pAudCon = NULL;
		pInputStream->mpInputBridge =  new CStrmConnWrapper(pAudCon != NULL, pVidCon != NULL,  pAudCon, pVidCon, &nResult);
	} else if(pInputStream->nInputType == INPUT_TYPE_STRMCONN_IPC) {
		int nResult;
		EXT_PARAM_STRMCONN_IPC_T *pExtParam = &pInputStream->ExtParam.strmconn_ipc;
		pInputStream->mpInputBridge =  new CIpcUdpSrc(
												pExtParam->szAudSocketRxName, 
												pExtParam->szAudSocketTxName,
												pExtParam->szVidSocketRxName, 
												pExtParam->szVidSocketTxName, 1, 1,  &nResult);
	} else if(pInputStream->nInputType == INPUT_TYPE_STRMCONN_ZMQ) {
#ifdef ENABLE_ZMQ
		int nResult;
		EXT_PARAM_STRMCONN_IPC_T *pExtParam = &pInputStream->ExtParam.strmconn_ipc;
		pInputStream->mpInputBridge =  new CIpcZmqSrc(
												pExtParam->szAudSocketTxName,
												pExtParam->szVidSocketTxName, 1, 1,  &nResult);
#endif
	} else {
		goto Exit;
	}
	res = 0;
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




int COnyxMw::CloseSession()
{
	JDBG_LOG(CJdDbg::LVL_TRACE,("Enter"));
	//m_pAvMixer->Stop();
	StopAvMixers();
	StopPublishStreams();
	StopPublishSwiches();
	JDBG_LOG(CJdDbg::LVL_TRACE,("Leave"));
	return 0;
}


int COnyxMw::StopPublishStreams()
{
	JDBG_LOG(CJdDbg::LVL_TRACE,("Enter"));
	if(m_pRtspSrvBridge) {
		JDBG_LOG(CJdDbg::LVL_TRACE,("RtspPublishBridge:RemoveMediaDelivery"));
		m_pRtspSrvBridge->RemoveMediaDelivery(m_pOutputStream);
		// TODO: Implete Delete all
		delete m_pRtspSrvBridge;
		m_pRtspSrvBridge = NULL;
	}
#ifdef ENABLE_RTMP
	if(m_pRtmpPublishBridge) {
		delete m_pRtmpPublishBridge;
		m_pRtmpPublishBridge = NULL;
	}
#endif
	if(m_pRtpSrvBridge) {
		delete m_pRtpSrvBridge;
		m_pRtpSrvBridge = NULL;
	}

	if(m_pUdpSrvBridge) {
		delete m_pUdpSrvBridge;
		m_pUdpSrvBridge = NULL;
	}

	if(m_pHlsSrvBridge){
		JDBG_LOG(CJdDbg::LVL_TRACE,("Stopping HlsSrvBridge"));
		delete m_pHlsSrvBridge;
		m_pHlsSrvBridge = NULL;
	}
	if(m_pMpdSrvBridge){
		JDBG_LOG(CJdDbg::LVL_TRACE,("Stopping MpdSrvBridge"));
		delete m_pMpdSrvBridge;
		m_pMpdSrvBridge = NULL;
	}

	if(m_pHlsSvrCfg) {
		delete m_pHlsSvrCfg;
		m_pHlsSvrCfg = NULL;
	}
	if(m_pHlsPublishCfg) {
		delete m_pHlsPublishCfg;
		m_pHlsPublishCfg = NULL;
	}

	if(m_pMpdRoot) {
		delete m_pMpdRoot;
		m_pMpdRoot = NULL;
	}


	if(m_pRtspCommonCfg){
		delete m_pRtspCommonCfg;
	}
	if(m_pRtspSvrCfg){
		delete m_pRtspSvrCfg;
	}

	if(m_pRtspPublishCfg) {
		delete m_pRtspPublishCfg;
		m_pRtspPublishCfg = NULL;
	}

	if(m_pRtpSrvCfg){
		delete m_pRtpSrvCfg;
	}

	if(m_pUdpSrvCfg){
		delete m_pUdpSrvCfg;
	}

	JDBG_LOG(CJdDbg::LVL_TRACE,("Leave"));
	return 0;
}


void COnyxMw::SetModuleDbgLvl(int ModuleId, int nDbgLevel)
{	
	switch(ModuleId)
	{
	case 0:
		modDbgLevel = nDbgLevel;
		break;
	case 1:
		break;
	case 2:
		CMediaSwitch::SetDbgLevel(nDbgLevel);
		break;
	case 3:
		CHlsSrvBridge::SetDbgLevel(nDbgLevel);
		break;
	case 4:
		hlsSetDebugLevel(nDbgLevel);
		break;
	}
}

#define ENABLE_CONSOLE

int COnyxMw::Run(const char *pszConfFile, int nLayoutOption)
{
	int res = 0;
	int nResClentMsg;
	CUiMsg *pUiMsg = CUiMsg::GetSingleton();

	StartSession(pszConfFile, nLayoutOption);

	pUiMsg->Start();

#ifdef ENABLE_CONSOLE
	CConsoleMsg ConsoleMsg;
	ConsoleMsg.Run();
#endif

	while(1) {
		UI_MSG_T Msg = {0};

		if(pUiMsg->IsAbort()) {
			DBG_PRINT("\nExiting due to signal\n");
			CloseSession();
			break;
		} 
		if(!pUiMsg->IsClientConnected()){
			JD_OAL_SLEEP(100);
			continue;
		}
		nResClentMsg = pUiMsg->GetClntMsg(&Msg);
		if(nResClentMsg == CLIENT_MSG_ERROR){
			DBG_PRINT("\nError getting UI Message\n");
			break;
		} else if(nResClentMsg == CLIENT_MSG_READY){
			if(Msg.nMsgId == UI_MSG_EXIT) {
				DBG_PRINT("\nExiting due UI Exit cmd\n");
				CloseSession();
				pUiMsg->Reply(UI_MSG_EXIT, UI_STATUS_OK);
				break;
			} else if(Msg.nMsgId == UI_MSG_SELECT_LAYOUT){
				// Restart
				int nLayoutId;
				DBG_PRINT("\nRestarting due to layout change\n");
				if(Msg.Msg.Layout.nLayoutId > 0) {
					nLayoutId = Msg.Msg.Layout.nLayoutId;
				} else {
					nLayoutId = nLayoutOption;
				}
				CloseSession();
				{
					CAvMixer *pAvMixer = GetAvMixer("avmixer0");
					pAvMixer->StopInputStreams();
					StartSession(pszConfFile, nLayoutId);
					//StartDecodeSubsystem(pszConfFile, nLayoutId);
					pAvMixer->StartInputStreams(pszConfFile, "avmixer0", nLayoutId);
					pUiMsg->Reply(UI_MSG_SELECT_LAYOUT, UI_STATUS_OK);
				}
			} else if(Msg.nMsgId == UI_MSG_SELECT_SWITCH_SRC){
				int nSrcId = 0;
				DBG_PRINT("\nSelecting publish src %d\n", Msg);
				if(Msg.Msg.SwitchSrc.nSrcId >= 0) {
					nSrcId = Msg.Msg.SwitchSrc.nSrcId;
				}
				StopPublishSwitch("switch0");
				SetPublishSwitchSrc("switch0", nSrcId, pszConfFile);
				StartPublishSwitch("switch0");
				pUiMsg->Reply(UI_MSG_SELECT_SWITCH_SRC, UI_STATUS_OK);
			} else if (Msg.nMsgId == UI_MSG_HLS_PUBLISH_STATS) {
				hlspublisgStats_t hlspublisgStats = {0};
				if(m_pHlsSrvBridge) {
					m_pHlsSrvBridge->GetPublishStatistics(&hlspublisgStats.nState, &hlspublisgStats.nStreamInTime, 
						&hlspublisgStats.nLostBufferTime, &hlspublisgStats.nStreamOutTime, &hlspublisgStats.nTotalSegmentTime);
					DBG_PRINT("HLS Publishing Stats nState=%d StreamInTime=%d StreamOutTime=%d TotalSegmentTime\n", hlspublisgStats.nState, hlspublisgStats.nStreamInTime, hlspublisgStats.nStreamOutTime, hlspublisgStats.nTotalSegmentTime);
					pUiMsg->ReplyStats(UI_MSG_HLS_PUBLISH_STATS, &hlspublisgStats, sizeof(hlspublisgStats));
				}
			} else if (Msg.nMsgId == UI_MSG_SWITCHES_STATS) {
				switchesStats_t switchesStats = {0};
				this->GetSwitchesStats(&switchesStats);
				pUiMsg->ReplyStats(UI_MSG_SWITCHES_STATS, &switchesStats, sizeof(switchesStats));
			} else if(Msg.nMsgId == UI_MSG_SET_MOD_DBG_LVL) {
				SetModuleDbgLvl(Msg.Msg.DbgLvl.nModuleId, Msg.Msg.DbgLvl.nDbgLvl);
				pUiMsg->Reply(UI_MSG_SET_MOD_DBG_LVL, UI_STATUS_OK);
			} else {
				DBG_PRINT("\nonyx_app:Unknown Command:%d\n", Msg.nMsgId);
			}
		} else {
			DBG_PRINT("onyx_app:Monitoring App not connected\n");
		}
	}
	return res;
}

#define VERSION_STRING  "version_2_2_0_dvm @(warning: aws publishing and rtmp disabled)"
int main(int argc, char **argv)
{
	COnyxMw *pOnyxMw = new COnyxMw;
	APP_OPTIONS Options = {0};
	ParseAppOptions(argc, argv, &Options);
	
	printf("\n***** onyx_mw %s compiled@%s:%s******\n", VERSION_STRING, __DATE__, __TIME__);
#ifdef WIN32
	{
		WORD wVersionRequested;
		WSADATA wsaData;
		int err;
		wVersionRequested = MAKEWORD(2, 2);
		err = WSAStartup(wVersionRequested, &wsaData);
		if (err != 0) {
			DBG_PRINT("WSAStartup failed with error: %d\n", err);
			exit(0);
		}
	}
#endif
	modDbgLevel =  ini_getl(ONYX_MW_SECTION, "debug_level", 2, ONYX_PUBLISH_FILE_NAME);
	pOnyxMw->Run(Options.conf_file, Options.nLayoutId);
}
