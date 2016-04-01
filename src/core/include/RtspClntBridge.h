#ifndef __RTSP_CLNT_BRIDGE_H__
#define __RTSP_CLNT_BRIDGE_H__

#include "StrmInBridgeBase.h"
#include "JdRfc3640.h"
#include "JdRfc3984.h"

#define MAX_NAME_SIZE	256
class CRtspClntBridge : public CStrmInBridgeBase
{
public:
	CRtspClntBridge(const char *lpszRspServer, int fEnableAud, int fEnableVid, int *pResult);
	virtual ~CRtspClntBridge();

	int StartClient(const char *lpszRspServer);
    virtual int StartStreaming(void);
    virtual int StopStreaming(void);

	static void *DoVideoBufferProcessing(void *pObj);
	static void *DoAudioBufferProcessing(void *pObj);

	long ProcessVideoFrame();
	long ProcessAudioFrame();
	int InitAudioStreaming();
	int InitVideoStreaming();
	void DumpStat();

public:
	CJdRtspClntSession	*m_pRtspClnt;
	int					m_nAudCodec;
	int					m_nVidCodec;
	int					m_nSrcType;
	CJdRfc3984Rx        *m_pRfcRtp;
	CJdRfcBase			*m_pAudRfcRtp;
	char				*m_pData;
	long				m_lUsedLen;
	long				m_lMaxLen;
	long long			m_lPts;
	long long			m_lDts;
	unsigned short      m_usSeqNum;
	long                m_lPktLoss;
	long                m_fDisCont;

	char				*m_pAudData;
	long				m_lAudUsedLen;
	long				m_lAudMaxLen;
	long long			m_lAudPts;

#ifdef WIN32
	HANDLE              m_thrdHandleVideo;
#else
	pthread_t			m_thrdHandleVideo;
#endif
#ifdef WIN32
	HANDLE              m_thrdHandleAudio;
#else
	pthread_t			m_thrdHandleAudio;
#endif

public:

	int        fEoS;
	int        frameCounter;
	unsigned short   mRtspPort;
	char             m_szRemoteHost[MAX_NAME_SIZE];
	unsigned short   m_usLocalRtpPort;
	unsigned short   m_usRemoteRtpPort;
	CAvcCfgRecord    mAvcCfgRecord;

	int              mDbgPrevTime;
	int              mDbgTotalAudPrev;
	int              mDbgTotalVidPrev;

	int              mTotalAud;
	int              mTotalVid;
};
#endif //__RTSP_CLNT_BRIDGE_H__
