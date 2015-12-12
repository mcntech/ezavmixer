#ifndef __MPD_SRV_BRIDGE_H__
#define __MPD_SRV_BRIDGE_H__

#include "StrmOutBridgeBase.h"
#include "JdHttpSrv.h"
#include "MpegDashSgmt.h"
#include "h264parser.h"
#include "TsPsiIf.h"
#include "tsfilter.h"
#include "AccessUnit.h"
#include "SimpleTsMux.h"
#include "uimsg.h"
#include <JdOsal.h>
#include "Mpd.h"
#include "Mp4MuxIf.h"
#include <string>
#include <map>
#include "JdAwsContext.h"

class CMpdSrvBridgeChan : public CStrmOutBridge
{
public:
	CMpdSrvBridgeChan(int fMuxType, int nSegmentStart, int nSegmentTime, int nTimeShiftBuffer);
	~CMpdSrvBridgeChan();

	int Run(COutputStream *pOutputStream);
	int SendVideo(unsigned char *pData, int size, unsigned long lPts90kHz);
	int SendAudio(unsigned char *pData, int size, unsigned long lPts90kHz);
	int SetVideoDiscont();
	int SetVideoEos();

	int SetServerConfig(
		CMpdRepresentation *pMpdRepresentation,  
		const char *pszFilePrefix, 
		const char *pszParentFolder, 
		const char *pszBucketOrServerRoot,
		CJdAwsContext *pAwsContext);
	
	int GetPublishStatistics(int *pnState, int *pnStreamInTime, int *pnLostBufferTime,  int *pnStreamOutTime, int *pnSegmentTime)
	{
		int res = 0;
		if(m_fEnablePublish && m_pUploader){
			mpdPublishGetStats(m_pUploader, pnState, pnStreamInTime, pnLostBufferTime, pnStreamOutTime, pnSegmentTime);
		} else {
			*pnState = HLS_PUBLISH_STOPPED;
		}
		return res;
	}
	int UpdateStats();

public:
	void            *m_pSegmenter;
	CJdHttpSrv      *m_pHttpSrv;

	void            *m_pUploader;
	void            *m_pUploaderForExtHttpSrv;
	CMpdRepresentation            *m_pMpdRepresentation;
	std::string            m_szFilePrefix;
	std::string            m_szParentFolder;
	std::string            m_szBucketOrServerRoot;
/*
	char	        m_szHost[256];
	char	        m_szAccessId[256];
	char	        m_szSecKey[256];
*/
	CJdAwsContext   m_AwsContext;
	int             m_fEnableServer;
	int             m_fEnablePublish;
	COsalMutex      m_Mutex;
	int             m_fDiscont;
	int             m_nSegmentStart;
	int             m_nSegmentTime;
	int             m_nTimeShiftBuffer;
	int             m_nMuxType;
};

class CMpdSrvBridge
{
public:
	CMpdSrvBridge();
	~CMpdSrvBridge();
	CMpdSrvBridgeChan *CreateChannel(
			std::string         szId,
			CMpdRepresentation *pCfgRepresenation,
			int        nSegmentStart,
			int        nSegemtTimeMs,
			int        nTimeShiftBuffer,
			const char *pszFilePrefix,
			const char *pszParentFolder,
			const char *pszBucketOrServerRoot,
			CJdAwsContext       *pJdAwsContext,
			int        nMimeType);

	CMpdSrvBridgeChan *getChannel(std::string szId);

	std::map<std::string, CMpdSrvBridgeChan *> m_listChan;
};

#endif
