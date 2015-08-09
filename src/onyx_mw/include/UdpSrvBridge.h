#ifndef __UDP_SRV_BRIDGE_H__
#define __UDP_SRV_BRIDGE_H__

#include "StrmOutBridgeBase.h"
#include "JdUdp.h"
#include "h264parser.h"
#include "TsDemuxIf.h"
#include "TsPsiIf.h"
#include "tsfilter.h"
#include "AccessUnit.h"
#include "SimpleTsMux.h"
#include "uimsg.h"
#include <JdOsal.h>

#define MAX_TS_BUFFER_SIZE		(512 * 1024)

class CUdpSrvBridge : public CStrmOutBridge
{
public:
	CUdpSrvBridge() : CStrmOutBridge("UDP")
	{
		m_pTsBuffer = (char *)malloc(MAX_TS_BUFFER_SIZE);
	}
	
	~CUdpSrvBridge()
	{
		if(m_pUdpSrv) {
			m_pUdpSrv->Stop();
			delete m_pUdpSrv;
		}
		if(m_pTsBuffer)
			free(m_pTsBuffer);
	}
	int Run(COutputStream *pOutputStream);
	int SendVideo(unsigned char *pData, int size, unsigned long lPts);
	int SendAudio(unsigned char *pData, int size, unsigned long lPts);
	int SetVideoDiscont();

	int SetServerConfig(CUdpServerConfig *pUdpSvrCfg)
	{ 
		sprintf(m_szResourceFolder, "/%s", pUdpSvrCfg->m_szApplicationName);
		m_pUdpServerConfig = pUdpSvrCfg;
		return 0;
	}
	
	int GetPublishStatistics(int *pnState, int *pnStreamInTime, int *pnLostBufferTime,  int *pnStreamOutTime, int *pnSegmentTime)
	{
		int res = 0;
		return res;
	}

public:
	void            *m_pSegmenter;
	CUdp            *m_pUdpSrv;
	CTsMux			m_TsMux;

	char            *m_pTsBuffer;
	char            m_szFilePrefix[256];
	char            m_szResourceFolder[256];


	CUdpServerConfig *m_pUdpServerConfig;

	COsalMutex      m_Mutex;
	int             m_fDiscont;
};
#endif // __UDP_SRV_BRIDGE_H__