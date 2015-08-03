#include "RtspPublishBridge.h"
#include "JdDbg.h"

static int  modDbgLevel = CJdDbg::LVL_SETUP;

#define RTSP_FLAG_HAS_SPS                0x00000001
#define RTSP_FLAG_DISCONT                0x00010000
#define MAX_TS_BUFFER_SIZE				(512 * 1024)

static int oalGetTimeMs()
{
#ifdef WIN32
	return (GetTickCount());
#else
	struct timeval   tv;
	gettimeofday(&tv,NULL);
	int nTimesMs =  (tv.tv_sec)*1000 + tv.tv_usec / 1000;
	return (nTimesMs);
#endif
}


CStrmClock::CStrmClock()
{
	m_llSgmntStrtPts = 0;
	m_llPrvSgmntStrtPts = 0;
	m_llCrntPts = 0;
	m_llPrevPts = 0;
	m_llFrameDuration = 3000; //Hz
}
unsigned long long CStrmClock::GetStrmPtsForSegmentPts(long long llPts)
{
	return 0;
}
	
int CStrmClock::SetSegmentStartPts(long long llPts)
{
	m_llPrvSgmntStrtPts = m_llSgmntStrtPts;
	m_llSgmntStrtPts = llPts;
	return 0;
}

int CStrmClock::SetFramePts(long long llPts)
{
	long long llDiff;

	JDBG_LOG(CJdDbg::LVL_STRM,("%d: Pts=%d", oalGetTimeMs(), llPts / 90));
	if(llPts > m_llPrevPts)
		llDiff = llPts - m_llPrevPts; 
	else
		llDiff = m_llPrevPts - llPts; 

	if(llDiff > (6 * m_llFrameDuration)) {
		int fDisCont = 1;
	}
	m_llPrevPts = m_llCrntPts;
	m_llCrntPts = llPts;
	return 0;
}

long long CStrmClock::SetStartPts(long long llPts)
{
	return 0;
}

CRtspPublishBridge::CRtspPublishBridge() : CStrmOutBridge("RTSP")
{
	mMediaResMgr = new CMediaResMgr();
	mRtspSrv = new CJdRtspSrv(mMediaResMgr);
	mRtspClntRec = new CJdRtspClntRecSession(mMediaResMgr);
	m_pOutputStream = NULL;
	mAggregate = NULL;

	mMp2tTrack = NULL;
	mVideoTrack = NULL;
	mAudioTrack = NULL;
	mRtspPort = 59400;
	m_fEnableRtspSrv = 0;
	m_pTsBuffer = (char *)malloc(MAX_TS_BUFFER_SIZE);
	mAvcCfgRecord.mSpsFound = 0;
	m_fVDiscont = 1;
}

CRtspPublishBridge::~CRtspPublishBridge()
{
	if(mRtspClntRec) delete mRtspClntRec;
	if(mRtspSrv) delete mRtspSrv;
	if(mMediaResMgr) delete mMediaResMgr;
}

void CRtspPublishBridge::PrepareMediaDelivery(COutputStream *pOutputStream)
{
	//m_pRtspSrv;
	mAggregate = new CMediaAggregate;
	m_pOutputStream = pOutputStream;
	if(mpRtspCommonCfg->m_fEnableMux) {
		mMp2tTrack = new CMediaTrack(TRACK_ID_STREAM, CODEC_NAME_MP2T, CMediaTrack::CODEC_TYPE_STREAM, CMediaTrack::CODEC_ID_MPT2, 59426);
		mAggregate->AddTrack(mMp2tTrack);
	} else {
		if(mpRtspCommonCfg->m_fEnableVid) {
			mVideoTrack = new CMediaTrack(TRACK_ID_VIDEO, CODEC_NAME_H264, CMediaTrack::CODEC_TYPE_VIDEO, CMediaTrack::CODEC_ID_H264, 59426);
			mVideoTrack->mpAvcCfgRecord = &mAvcCfgRecord;
			mAggregate->AddTrack(mVideoTrack);
		}
		if(mpRtspCommonCfg->m_fEnableAud) {
			mAudioTrack = new CMediaTrack(TRACK_ID_AUDIO, CODEC_NAME_AAC, CMediaTrack::CODEC_TYPE_AUDIO, CMediaTrack::CODEC_ID_AAC, 59428);
			mAggregate->AddTrack(mAudioTrack);
		}
	}
	mMediaResMgr->AddMediaAggregate(pOutputStream->m_szStreamName, mAggregate);
}

int CRtspPublishBridge::SetRtspServerCfg(CRtspSrvConfig *pRtspSvrCfg)
{
	mpRtspSrvCfg = pRtspSvrCfg;
	return 0;
}

int CRtspPublishBridge::StartRtspServer()
{
	m_fEnableRtspSrv = 1;
	mRtspSrv->Run(1, mpRtspSrvCfg->usServerRtspPort);
	return 0;
}

void CRtspPublishBridge::RemoveMediaDelivery(COutputStream *pOutputStream)
{
	if(mRtspClntRec) {
		mRtspClntRec->Close();
	}
	if(mRtspSrv) {
		mRtspSrv->Stop(1);
	}
	if(mMediaResMgr) {
		if(mAggregate) {
			if(mVideoTrack) {
				mAggregate->RemoveTrack(mVideoTrack);
				delete mVideoTrack;
			}
			if(mAudioTrack) {
				mAggregate->RemoveTrack(mAudioTrack);
				delete mAudioTrack;
			}
			if(mMp2tTrack) {
				mAggregate->RemoveTrack(mMp2tTrack);
				delete mMp2tTrack;
			}
			
			mMediaResMgr->RemoveAggregate(pOutputStream->m_szStreamName);
			delete mAggregate;
		}
	}
}

unsigned char *CRtspPublishBridge::FindStartCode(unsigned char *p, unsigned char *end)
{
    while( p < end - 3) {
        if( p[0] == 0 && p[1] == 0 && ((p[2] == 1) || (p[2] == 0 && p[3] == 1)) )
            return p;
		p++;
    }
    return end;
}

int CRtspPublishBridge::SetVideoDiscont()
{
	m_fVDiscont = 1; // Used only for MP2T
	mAvcCfgRecord.mSpsFound = 0;
	return 0;
}

int CRtspPublishBridge::SetAudioDiscont()
{
	return 0;
}

int CRtspPublishBridge::SendVideo(unsigned char *pData, int size, unsigned long lPts)
{
	unsigned char *p = pData;
	unsigned char *end = p + size;
	unsigned char *pNalStart, *pNalEnd;
	int           lNalSize = 0;
	long          lBytesUsed = 0;

	if(mpRtspCommonCfg->m_fEnableMux)
		return SendMp2tVideo(pData, size, lPts);

	JDBG_LOG(CJdDbg::LVL_STRM,("size=%d", size));
	pNalStart = FindStartCode(p, end);

	while (pNalStart < end) {
		/* Skip Start code */
		while(!*(pNalStart++)) lBytesUsed++;
        lBytesUsed++;

		pNalEnd = FindStartCode(pNalStart, end);
		lNalSize = pNalEnd - pNalStart;
		assert(lNalSize > 0);
		/* Update SPS and PPS */
		if((pNalStart[0] & 0x1F) == 7 /*SPS*/) {
			mAvcCfgRecord.UpdateAvccSps(pNalStart, lNalSize);
			if(m_fVDiscont) {
				m_fVDiscont = 0;
				mStrmClck.SetSegmentStartPts(lPts);
			}
		} else if((pNalStart[0] & 0x1F) == 8 /*PPS*/) {
			mAvcCfgRecord.UpdateAvccPps((char *)pNalStart, lNalSize);
		}
		if((pNalStart[0] & 0x1F) >= 20) {
			int InvalidNalType  = 1;
		}

		if(mAvcCfgRecord.mSpsFound) {
			//if((pNalStart[0] & 0x1f) == 5){
			//	mVideoTrack->Deliver((char *)mAvcCfgRecord.mSps, mAvcCfgRecord.mSpsSize, lPts);
			//	mVideoTrack->Deliver((char *)mAvcCfgRecord.mPps, mAvcCfgRecord.mPpsSize, lPts);
			//}
			mStrmClck.SetFramePts(lPts);
			mVideoTrack->Deliver((char *)pNalStart, lNalSize, lPts);
			lBytesUsed += lNalSize;
		} else {
			// Discard data
			int Discard = 1;
		}
        pNalStart = pNalEnd;
    }
    return lBytesUsed;
}

int CRtspPublishBridge::SendAudio(unsigned char *pData, int sizeData, unsigned long lPts)
{
	int res = 0;
	unsigned char *pFrameStart =  pData;
	unsigned char *pDataEnd = pData + sizeData;
	int nFrameSize = 0;

	if(mpRtspCommonCfg->m_fEnableMux)
		return SendMp2tAudio(pData, sizeData, lPts);

	if(mAudioTrack == NULL){
		goto Exit;
	}

	mAudioTrack->Deliver((char *)pData, sizeData, lPts);
	res = sizeData;
Exit:
    return res;
}

static int HasSps(unsigned char *pData, int nLen)
{
	int i = 0;
	while(i < nLen - 4) {
		if ((pData[i+4] & 0x1F) == 0x07 && pData[i+3] == 0x01 && pData[i+2] == 0x00 && pData[i+1] == 0x00 && pData[i+0] == 0x00)
			return 1;
		i++;
	}
	return 0;
}

int CRtspPublishBridge::SendMp2tVideo(unsigned char *pData, int sizeData, unsigned long lPts)
{
	long hr = 0;
	int nSearchLen;
	unsigned long ulFlags = 0;
	CAccessUnit Au;
	Au.m_TSStreamID = 0;
	Au.m_SampleStreamType = SAMPLE_TYPE_H264;
	Au.m_pRawData = (char *)pData;
	Au.m_RawSize = sizeData;
	Au.m_pTsData = m_pTsBuffer;
	Au.m_PTS = lPts;
	Au.m_DTS = Au.m_PTS;
	Au.m_Flags = 0;

	JDBG_LOG(CJdDbg::LVL_STRM,("size=%d", sizeData));
	nSearchLen = sizeData < 32 ? sizeData : 32;				// Limit SPS search tp first 32 bytes
	if(HasSps(pData, nSearchLen)) {
		ulFlags |= RTSP_FLAG_HAS_SPS;
		Au.m_Flags = FORCE_SEND_PSI;
	}
	//DumpHex(pData, 32);
	//DBG_PRINT("CHlsSrvBridge::SendVideo size=%d pts=%d(ms)\n", size, lPts / 1000);
	if(m_fVDiscont) {
		ulFlags |= RTSP_FLAG_DISCONT;
		m_fVDiscont = 0;
	}
	m_TsMux.Mux(&Au);
	m_Mutex.Acquire();
	mMp2tTrack->Deliver((char *)m_pTsBuffer, Au.m_TsSize, lPts);

	m_Mutex.Release();
	return hr;
}

int CRtspPublishBridge::SendMp2tAudio(unsigned char *pData, int sizeData, unsigned long lPts)
{
	long hr = 0;
	unsigned long ulFlags = 0;
	CAccessUnit Au;
	Au.m_TSStreamID = 0;
	Au.m_SampleStreamType = SAMPLE_TYPE_AAC;
	Au.m_pRawData = (char *)pData;
	Au.m_RawSize = sizeData;
	Au.m_pTsData = m_pTsBuffer;
	Au.m_PTS = lPts;
	Au.m_DTS = Au.m_PTS;
	
	m_TsMux.Mux(&Au);
	m_Mutex.Acquire();
	mMp2tTrack->Deliver((char *)m_pTsBuffer, Au.m_TsSize, lPts);
	m_Mutex.Release();
	return hr;

}

int CRtspPublishBridge::SetPublishServerCfg(CRtspPublishConfig *pRtspPublishCfg)
{
	mpRtspPublishCfg = pRtspPublishCfg;
	return 0;
}

int CRtspPublishBridge::ConnectToPublishServer()
{
	int res = -1;
	CRtspPublishConfig *pRtspPublishCfg = mpRtspPublishCfg;
	if(strlen(pRtspPublishCfg->szRtspServerAddr)) {
		if(mRtspClntRec->Open(pRtspPublishCfg->szRtspServerAddr, pRtspPublishCfg->szApplicationName, pRtspPublishCfg->usServerRtspPort) == 0) {
			CMediaAggregate *pMediaAggregate = mRtspClntRec->mpMediaResMgr->GetMediaAggregate(m_pOutputStream->m_szStreamName);
			if(mRtspClntRec->SendAnnounce(m_pOutputStream->m_szStreamName) == 0) {
				if(mRtspClntRec->SendSetup("video",pRtspPublishCfg->usRtpLocalPort,pRtspPublishCfg->usRtpLocalPort+1) == 0){
					mRtspClntRec->SendRecord(m_pOutputStream->m_szStreamName);
					mRtspClntRec->StartPublish(m_pOutputStream->m_szStreamName, TRACK_ID_VIDEO);
					res = 0;
				} else {
					fprintf(stderr, "Failed to setup video port=%d\n",pRtspPublishCfg->usRtpLocalPort );
				}
			} else {
				fprintf(stderr, "Failed to Announce application=%s stream=%s\n", pRtspPublishCfg->szApplicationName,m_pOutputStream->m_szStreamName);
			}
		} else {
			fprintf(stderr, "Failed to connect to %s:%d\n", pRtspPublishCfg->szRtspServerAddr,pRtspPublishCfg->usServerRtspPort);
		}
	}
	return res;
}
