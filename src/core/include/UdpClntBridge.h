#ifndef __RTSP_CLNT_BRIDGE_H__
#define __RTSP_CLNT_BRIDGE_H__

#include "StrmInBridgeBase.h"
#include "UdpCallback.h"

#define MAX_NAME_SIZE	256

class CUdpClntBridge : public CStrmInBridgeBase
{
public:
	CUdpClntBridge(const char *lpszRspServer, int fEnableAud, int fEnableVid, int *pResult , CUdpServerCallback *pCallback=NULL);
	virtual ~CUdpClntBridge();

	int StartClient(const char *lpszRspServer);
    virtual int StartStreaming(void);
    virtual int StopStreaming(void);

	static void *DoVideoBufferProcessing(void *pObj);
	static void *DoAudioBufferProcessing(void *pObj);

	static void *DoAudioRtcpProcessing(void *pArg);
	static void *DoVideoRtcpProcessing(void *pArg);

	long ProcessVideoFrame();
	long ProcessAudioFrame();
	int InitAudioStreaming();
	int InitVideoStreaming();
	void UpdateJitter(int nPktTime);

	void UpdateStat();
public:
	//CJdRtspClntSession	*m_pRtspClnt;
	CUdpServerCallback  *m_pCallback;
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
	HANDLE              m_thrdHandleVideo;
#else
	pthread_t			m_thrdHandleVideo;
#endif
#ifdef WIN32
	HANDLE              m_thrdHandleAudio;
#else
	pthread_t			m_thrdHandleAudio;
#endif

#ifdef WIN32
	HANDLE              m_thrdHandleVideoRtcp;
#else
	pthread_t			m_thrdHandleVideoRtcp;
#endif
#ifdef WIN32
	HANDLE              m_thrdHandleAudioRtcp;
#else
	pthread_t			m_thrdHandleAudioRtcp;
#endif
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
};
#endif //__RTSP_CLNT_BRIDGE_H__
