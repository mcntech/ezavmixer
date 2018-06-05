#ifndef __UDP_CLNT_BRIDGE_H__
#define __UDP_CLNT_BRIDGE_H__

//#include "StrmInBridgeBase.h"
#include <map>
#include "JdOsal.h"
#include "UdpCallback.h"
#include "strmcomp.h"
#include "TsPsi.h"
#include "JdWebsocket.h"

#define  UDP_SRC_PREFIX   "udp://"

#define MAX_NAME_SIZE	256

class CUdpClntBridge : public  CJdWsService//: public CStrmInBridgeBase
{
public:
	CUdpClntBridge(const char *lpszRspServer, int fEnableAud, int fEnableVid, int *pResult , CUdpServerCallback *pCallback=NULL);
	virtual ~CUdpClntBridge();

    virtual int StartStreaming(void);
    virtual int StopStreaming(void);
    virtual int ConnectStreamForPid(int nPid, ConnCtxT *pConn);
	void SubscribeProgram(int strmId);
	void UnsubscribeProgram(int strmId);
	void UpdateJitter(int nPktTime);

	void UpdateStat();

	void UpdatePmt(int nPid, const char *pData, int len );
	void UpdatePat(const char *pData, int len);
	void UpdateFormat(int nPid, int nCodecType, const char *pData, int len);

	//void psiJson(std::string &psiString);
	void psiPatJson(std::string &psiString);
	void psiPmtJson(MPEG2_PMT_SECTION *pmt, std::string &psiString);
	//void statPmtJson(PgmStatT *stat, std::string &statString);

	void strmH264FmtJson(const char *pFmtData, int len, std::string &psiString);
	void strmH265FmtJson(const char *pFmtData, int len, std::string &psiString);
    void strmMP2VidFmtJson(const char *pFmtData, int len, std::string &psiString);
	void strmAudFmtJson(const char *pFmtData, int len, std::string &psiString);
	unsigned long long GetPcrClock(int nPid);

    // CJdWsService
    std::string ProcessWsRequest(std::string request);
	int UpdatePmtData(int nPid, struct MPEG2_PMT_SECTION &pmt);

public:
	//CJdRtspClntSession	*m_pRtspClnt;
	CUdpServerCallback  *m_pCallback;
    CJdWs               *m_pJdWs;

	int					m_nAudCodec;
	int					m_nVidCodec;
	int					m_nSrcType;


	char				*m_pData;
	long				m_lUsedLen;
	long				m_lMaxLen;
	long long			m_lPts;
	long long			m_lDts;
	unsigned short      m_usVidSeqNum;
	unsigned short      m_usAudSeqNum;
	long                m_lPktLoss;
	long                m_fDisCont;

	char				*m_pAudData;
	long				m_lAudUsedLen;
	long				m_lAudMaxLen;
	long long			m_lAudPts;

#ifdef WIN32
	HANDLE              m_thrdHandle;
#else
	pthread_t			m_thrdHandle;
#endif

public:
	int                   m_nUiCmd;
	int                   m_fRun;
	int                   m_fEnableAud;
	int                   m_fEnableVid;

	long                  m_lWidth;
	long                  m_lHeight;

public:
	bool             m_fEnableRtcp;
	int              fEoS;
	int              frameCounter;
	unsigned short   mRtspPort;
	char             m_szRemoteHost[MAX_NAME_SIZE];
	unsigned short   m_usLocalRtpPort;
	unsigned short   m_usRemoteRtpPort;
	//CAvcCfgRecord    mAvcCfgRecord;

	long long        mDbgPrevTime;
	int              mDbgTotalAudPrev;
	int              mDbgTotalVidPrev;
	int              mJitterUpdateTime;

	UDP_SERVER_STATS m_UdpServerStats;
	int              mTotalAud;
	int              mTotalVid;
	StrmCompIf       *m_srcComp;
	StrmCompIf       *m_demuxComp;

	ConnCtxT          *m_pConnCtxSrcToDemux;
	ConnCtxT          *m_pConnVidChainSrc;
	struct MPEG2_PAT_SECTION                *m_pat;
	std::map<int, struct MPEG2_PMT_SECTION *> m_pmts;
};
#endif //__UDP_CLNT_BRIDGE_H__
