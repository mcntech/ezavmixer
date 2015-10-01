#ifndef __RTSP_PUBLISH_BRIDGE__
#define __RTSP_PUBLISH_BRIDGE__

#include "StrmOutBridgeBase.h"
#include "JdRtspSrv.h"
#include "JdRtspClntRec.h"
#include "JdRfc3984.h"
#include "AccessUnit.h"
#include "SimpleTsMux.h"
#include <JdOsal.h>
#include "RtspConfigure.h"
class CStrmClock
{
public:
	CStrmClock();
	unsigned long long GetStrmPtsForSegmentPts(long long llPts);
	int SetSegmentStartPts(long long llPts);
	int SetFramePts(long long llPts);
	long long SetStartPts(long long llPts);

private:
	long long m_llSgmntStrtPts;
	long long m_llPrvSgmntStrtPts;
	long long m_llCrntPts;
	long long m_llPrevPts;
	long long m_llFrameDuration;
};

class CRtspPublishBridge : public CStrmOutBridge
{
public:
	CRtspPublishBridge();
	~CRtspPublishBridge();

	int Run(COutputStream *pOutputStream)
	{
		return 0;
	}
	int Init(COutputStream *pOutputStream)
	{
		PrepareMediaDelivery(pOutputStream);
		return -1;
	}

//private:
	void PrepareMediaDelivery(COutputStream *pOutputStream);
	void RemoveMediaDelivery(COutputStream *pOutputStream);
	unsigned char *FindStartCode(unsigned char *p, unsigned char *end);

	int SendMp2tAudio(unsigned char *pData, int size, unsigned long lPts);
	int SendMp2tVideo(unsigned char *pData, int size, unsigned long lPts);
	int SendVideo(unsigned char *pData, int size, unsigned long lPts);
	int SendAudio(unsigned char *pData, int size, unsigned long lPts);
	
	int SetStreamCfg(CRtspCommonConfig *pCRtspCommonConfig)
	{
		mpRtspCommonCfg = pCRtspCommonConfig;
		return 0;
	}
	int SetPublishServerCfg(CRtspPublishConfig *pRtspPublishCfg);
	int ConnectToPublishServer();
	int SetRtspServerCfg(CRtspSrvConfig *pRtspSvrCfg);
	int StartRtspServer();

	int SetVideoDiscont();
	int SetAudioDiscont();

public:
	CJdRtspSrv			*mRtspSrv;
	CMediaResMgr        *mMediaResMgr;
	CMediaAggregate		*mAggregate;

	CMediaTrack			*mMp2tTrack;
	CMediaTrack			*mVideoTrack;
	CMediaTrack			*mAudioTrack;

	CJdRtspClntRecSession *mRtspClntRec;

	CRtspCommonConfig     *mpRtspCommonCfg;
	CRtspSrvConfig        *mpRtspSrvCfg;
	CRtspPublishConfig     *mpRtspPublishCfg;

	CTsMux			    m_TsMux;
	COsalMutex          m_Mutex;
	char                *m_pTsBuffer;
	int                 m_fVDiscont;
	int                 m_fADiscont;

public:
	unsigned short   mRtspPort;
	char             m_szRemoteHost[MAX_NAME_SIZE];
	unsigned short   m_usLocalRtpPort;
	unsigned short   m_usRemoteRtpPort;
	int              m_fEnableRtspSrv;
	
	CAvcCfgRecord    mAvcCfgRecord;
	CStrmClock       mStrmClck;
};

#endif
