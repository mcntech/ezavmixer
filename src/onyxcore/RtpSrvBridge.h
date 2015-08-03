#ifndef __RTP_SRV_BRIDGE_H__
#define __RTP_SRV_BRIDGE_H__

#include "StrmOutBridgeBase.h"
#include "JdRtp.h"
#include "h264parser.h"
#include "TsDemuxIf.h"
#include "TsPsiIf.h"
#include "tsfilter.h"
#include "AccessUnit.h"
#include "SimpleTsMux.h"
#include "uimsg.h"
#include <JdOsal.h>

#define MAX_TS_BUFFER_SIZE		(512 * 1024)

class CRtpSrvBridge : public CStrmOutBridge
{
public:
	CRtpSrvBridge() : CStrmOutBridge("RTSP")
	{
		m_pTsBuffer = (char *)malloc(MAX_TS_BUFFER_SIZE);
	}
	
	~CRtpSrvBridge()
	{
		if(m_pRtpSrv) {
			m_pRtpSrv->Stop();
			delete m_pRtpSrv;
		}
		if(m_pTsBuffer)
			free(m_pTsBuffer);
	}
	int Run(COutputStream *pOutputStream);
	int SendVideo(unsigned char *pData, int size, unsigned long lPts);
	int SendAudio(unsigned char *pData, int size, unsigned long lPts);
	int SetVideoDiscont();

	int SetServerConfig(CRtpServerConfig *pRtpSvrCfg)
	{ 
		sprintf(m_szResourceFolder, "/%s", pRtpSvrCfg->m_szApplicationName);
		m_pRtpServerConfig = pRtpSvrCfg;
		return 0;
	}
	
	int GetPublishStatistics(int *pnState, int *pnStreamInTime, int *pnLostBufferTime,  int *pnStreamOutTime, int *pnSegmentTime)
	{
		int res = 0;
		return res;
	}

public:
	void            *m_pSegmenter;
	CRtp            *m_pRtpSrv;
	CTsMux			m_TsMux;

	char            *m_pTsBuffer;
	char            m_szFilePrefix[256];
	char            m_szResourceFolder[256];

	CRtpServerConfig *m_pRtpServerConfig;

	COsalMutex      m_Mutex;
	int             m_fDiscont;
};
#endif // __RTP_SRV_BRIDGE_H__