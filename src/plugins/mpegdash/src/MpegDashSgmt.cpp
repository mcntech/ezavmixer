#include <assert.h>
#include <string.h>
#ifdef WIN32
#include <winsock2.h>
#include <fcntl.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#define	 OSAL_WAIT(x) Sleep(x)
#define read	_read
#define pthread_t void *
#else

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <memory.h>
#include <signal.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#define	 OSAL_WAIT(x) usleep(x * 1000)
#include <pthread.h>
#define  O_BINARY     0
#endif
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <map>
#include <list>
#include "JdDbg.h"

using namespace std;

#include "MpegDashSgmt.h"
#include "JdHttpClnt.h"
#include "TsParse.h"
//#include "MpdOutBase.h"
#include "SegmentWriteS3.h"
#include "MpdPublishBase.h"
#include "Mpd.h"
#include "Mp4MuxIf.h"
#include "tsfilter.h"
#include "AccessUnit.h"
#include "SimpleTsMux.h"
#include "h264parser.h"
#include "JdMpdDefs.h"
#define EN_S3_UPLOAD

#ifdef EN_S3_UPLOAD
#include "SegmentWriteBase.h"
#include "JdAwsRest.h"
#include "JdAwsConfig.h"
#endif
#define  MEM_FILE_DEFAULT_FRONT_CACHE   0
#define  MEM_FILE_DEFAULT_BACK_CACHE    0

#define  S3_DEFAULT_FRONT_CACHE         1
#define  S3_DEFAULT_BACK_CACHE          1

#define MAX_UPLOAD_TIME			(3600 * 1000)	// 1 hour
#define DEF_SEGMENT_DURATION	4000

#define MAX_SPS_SIZE    128
#define MAX_PPS_SIZE    64
#define EMSZ_MAX_SIZE	1024
//#define TESTING_ROKU

static int               modDbgLevel = CJdDbg::LVL_STRM;

/* Delay updating S3 Playlist by S3_CACHE_DELAY segments */
#define S3_CACHE_DELAY				2
#define DEFUALT_NUM_TIMESHIFT_SEGS	        3

#define PLAYLIST_BUFFER_SIZE		(4 * 1024)
#define PLAYLIST_MAX_SIZE			(4 * 1024 * 1024)
#define WAIT_TIME_FOR_FILLED_BUFF	4000 //100000
#define WAIT_TIME_FOR_EMPTY_BUFF	100000

#define MAX_FILE_NAME                260

static void GetSegmentNameFromIndex(char *pszSegmentFileName, const char *pszBaseName, int nSegmentIdx, int nMuxType)
{
	char szSegmentExt[8] = {0};
	if(nMuxType == MPD_MUX_TYPE_TS)
		strncpy(szSegmentExt, TS_SEGEMNT_FILE_EXT, 8);
	else
		strncpy(szSegmentExt, MP4_SEGEMNT_FILE_EXT, 8);

	sprintf(pszSegmentFileName,"%s_%d.%s", pszBaseName, nSegmentIdx, szSegmentExt);
}

class CMpdEmsg
{
public:
	CMpdEmsg(CMpdRoot  *pMpd)
	{
		m_pBuffer = (char *)malloc(EMSZ_MAX_SIZE);
		m_presentation_time_delta = 0;
		m_fActive = 0;
		m_pMpd = pMpd;
		m_nLen = 0;
	}

	void GenerateMsg()
	{
		const char *pPublishDate = m_pMpd->GetAvailabilityStartTime();
		m_nLen =  CreateEmsg(m_pBuffer, EMSZ_MAX_SIZE, ATTRIB_NAME_SCHEME_ID_URI, ATTRIB_VAL_SCHEME_ID_URI_1, 
					0/* debug:m_presentation_time_delta*/, 0/*debug 0XFFFF*/, 1, (unsigned char *)pPublishDate, strlen(pPublishDate));
	}
	
	void Start(int presentation_time_delta)
	{
		m_presentation_time_delta = presentation_time_delta;
		m_fActive = 1;
	}
	void DecrementPresentationTimeDelta(int nSegmentDuration)
	{
		m_presentation_time_delta -= nSegmentDuration;
		if(m_presentation_time_delta <= 0) {
			m_fActive = 0;
			m_presentation_time_delta = 0;
		}
	}

	char      *m_pBuffer;
	int       m_nLen;
	int       m_presentation_time_delta;
	CMpdRoot  *m_pMpd;
	int       m_fActive;
};

class CMpdPublishBase
{
public:
	CMpdPublishBase();
	virtual int ReceiveInitSegment(const char *pData, int nLen) = 0;
	virtual int ReceiveGop(int nGopNum, const char *pData, int nLen, int nStartPtsMs, int nDurarionMs, CMpdEmsg *pEmsg) = 0;
	virtual void Run() = 0;
	virtual void Stop() = 0;
	virtual int GetStats(MPD_PUBLISH_STATS *pStats);
	virtual void SetDisContinuity() {}

protected:
	int                 m_nError;
	int                 m_nLostBufferTime;
	int                 m_nSegmentTime;
	int                 m_nInStreamTime;
	int                 m_nOutStreamTime;
	int                 m_nStreamStartTime;
	int                 m_fSupportDiscont;
	int                 m_nBcastFrontCache;      // Delays deletion of playlist to accommodate delayed requests from client  
	int                 m_nBcastBackCache;       // Delays updation of playlist to accommodate latency of segment update
	int                 m_nTimeshiftSegments;
	int                 m_nMuxType;
};

class CSegmentInf
{
public:
	CSegmentInf(int nSuffix, int nStartTime, int nDuration, int fDiscont)
	{
	  m_nSuffix = nSuffix; 
	  m_nDuration = nDuration; 
	  m_fDiscont = fDiscont; 
	  m_nStartTime = nStartTime;
	}
	int m_nSuffix;
	int m_nDuration;
	int m_nStartTime;
	int m_fDiscont;
};

#define MAX_PL_LINE_SIZE 256

void UpdateSegmentList(CMpdRepresentation *pMdpRep, const char *pszRepId, std::list<CSegmentInf *> *pSegmentList, int nSkipSegments, int nReqSegments, int nMuxType)
{
	char szTemp[MAX_PL_LINE_SIZE];
	int  i = 0;
	int  fSeqStart = 0;
	int nSeqStart = 0;
	int nDuration = 0;
	int nStartTime = 0;
	int nSegmentaAdded = 0;
	char szSegmentExt[8] = {0};
	CSegmentInf        *pSegmentInf;
	std::list<std::string> listUrl;

	for (std::list<CSegmentInf *>::iterator it = pSegmentList->begin(); it != pSegmentList->end(); ++it) {
		CSegmentInf *pSegmentInf = *it;
		if(i < nSkipSegments){
			// Skip it
		} else if(nSegmentaAdded >= nReqSegments){
			break;
		} else {
			char szUrl[256] = {0};
			pSegmentInf = *it;
			GetSegmentNameFromIndex(szUrl, pszRepId, pSegmentInf->m_nSuffix, nMuxType);
			listUrl.push_back(szUrl);
			nSegmentaAdded++;
			if(!fSeqStart) {
				fSeqStart = 1;
				nSeqStart = pSegmentInf->m_nSuffix;
				nDuration = pSegmentInf->m_nDuration;
				nStartTime = pSegmentInf->m_nStartTime;
			}
		}
		i++;
	}
	pMdpRep->UpdateSegmentList(nStartTime, nDuration, nSeqStart, &listUrl);
}

static char *GetMpdBaseName(const char *pszFile)
{
	const char *pIdx  = strstr(pszFile,".mpd");
	if(pIdx) {
		int nLen = pIdx - pszFile;
		if (nLen > 0) {
			char *pszBase = (char *)malloc(nLen + 1);
			memcpy(pszBase, pszFile, nLen);
			pszBase[nLen] = 0x00;
			return pszBase;
		}
	}
	return NULL;
}
class CMpdOutFs : public CSegmentWriteBase
{
public:
	int Start(const char *pszParentFolder, const char *pszFile, int nTotalLen, char *pData, int nLen, const char *pContentType)
	{
		char szFile[256];
		sprintf(szFile, "%s/%s",pszParentFolder, pszFile);
		int fd  =  open(szFile,  O_CREAT|O_RDWR|O_TRUNC|O_BINARY ,
#ifdef WIN32
	_S_IREAD | _S_IWRITE);
#else
	 S_IRWXU | S_IRWXG | S_IRWXO);
#endif

		if(fd > 0) {
			m_pCtx = (void *)fd;
			if(pData && nLen> 0) {
				write(fd, pData, nLen);
			}
		} else {
			m_pCtx = (void *)-1;
			JDBG_LOG(CJdDbg::LVL_ERR,("!!! Failed to open %s!!!\n",pszParentFolder));
		}
		return 0;
	}

	int Continue(char *pData, int nLen)
	{
		int res = 0;
		int fd = (int)m_pCtx;
		if(fd != -1) {
			if(pData && nLen > 0) {
				res = write(fd, pData, nLen);
			}
		}
		return res;
	}

	int End(char *pData, int nLen)
	{
		int res = -1;
		int fd = (int)m_pCtx;
		if(fd != -1)  {
			if(pData && nLen > 0) {
				res = write(fd, pData, nLen);
			}
			close(fd);
		}
		return res;
	}

	int Send(const char *pszParentFolder, const char *pszFile, std::time_t req_time, char *pData, int nLen, const char *pContentType, int nTimeOut)
	{
		int res = 0;
		char szFile[256];
		sprintf(szFile, "%s/%s",pszParentFolder, pszFile);
		int fd  =  open(szFile,  O_CREAT|O_RDWR|O_TRUNC|O_BINARY ,
#ifdef WIN32
	_S_IREAD | _S_IWRITE);
#else
	 S_IRWXU | S_IRWXG | S_IRWXO);
#endif
		if(fd > 0) {
			res = write(fd, pData, nLen);
			close(fd);
		} else {
#ifdef WIN32
			res = GetLastError();
#else
#endif
		}
		return res;
	}

	int Delete(const char *pszParentFolder, const char *pszFile)
	{
		int res = 0;
		char szFile[256];
		sprintf(szFile, "%s/%s",pszParentFolder, pszFile);
		unlink(szFile);
		return 0;
	}
};


static void DumpHex(unsigned char *pData, int nSize)
{
	int i;
	for (i=0; i < nSize; i++)
		printf("%02x ", pData[i]);
	printf("\n");
}

class CGop : public COsalMutex
{
public:
	unsigned char *m_pBuff;
	int           m_nLen;
	int           m_nMaxLen;
	int           m_nGopNum;
	int           m_DurationMs;

	CGop( int nMaxLen)
	{
		m_pBuff = (unsigned char *)malloc(nMaxLen);
		m_nMaxLen = nMaxLen;
		m_nLen = 0;
		m_DurationMs = 500;
	}
	~CGop()
	{
		if(m_pBuff) {
			free(m_pBuff);
			m_pBuff = NULL;
		}
	}

	void Clear()
	{
		m_nLen = 0;
	}
	int AddData(unsigned char *pData, int nLen)
	{
		int nRes  = 0;
		if(m_pBuff && m_nLen + nLen < m_nMaxLen) {
			memcpy(m_pBuff + m_nLen, pData, nLen);
			m_nLen += nLen;
			nRes = nLen;
		}
		return nRes;
	}
};

#define MAX_TS_BUFFER_SIZE		(512 * 1024)
#define MAX_MP4_BUFFER_SIZE		(8 * 1024 * 1024)
#define MAX_INIT_SEGMENT_SIZE 1024

/**
 * CMpdSegmnter: Manages segment list
 */
class CMpdSegmnter
{
public:
	CMpdSegmnter(CMpdRepresentation *pMpdRep)
	{
		int nMuxType = pMpdRep->GetMimeType();
		int nMoofDurationMs = pMpdRep->GetCutomAttribMoofLength();
		int fEnableVid = 1;
		int fEnableAud = 0;
		m_nWr = 0;
		m_nSeqStart = 0;
		m_fHasSps = 0;
		m_nGopSize = 2 * 1024 * 1024; // TODO
		m_nCrntGopNum = 0;
		m_pMp4Mux = NULL;
		m_pMp4moof = NULL;
		m_pMuxBuffer = NULL;
		m_pCrntGop = new CGop(m_nGopSize);
		if(nMuxType == MPD_MUX_TYPE_TS) {
			m_pMuxBuffer = (char *)malloc(MAX_TS_BUFFER_SIZE);
		} else {
			fEnableVid = nMuxType == MPD_MUX_TYPE_VIDEO_MP4;
			fEnableAud = nMuxType == MPD_MUX_TYPE_AUDIO_MP4;
			m_pMp4Mux = CreateMp4Segmenter(fEnableAud, fEnableVid);
			m_pMuxBuffer = (char *)malloc(MAX_MP4_BUFFER_SIZE);
			m_nSegmentInitLen = 0;
			m_pSegmentInitData = (char *)malloc(MAX_INIT_SEGMENT_SIZE);
		}
		m_nMuxType = nMuxType;
		m_fMp4InitSegment = 0;
		m_fMp4VidInit = 0;
		m_fMp4AudInit = 0;
		m_fVidEnable = 1; // TODO
		m_fAudEnable = 0;
		m_nMoofDurationMs = nMoofDurationMs;
		m_nMoofCrntTime = 0;
		m_nMp4SStrmPts = 0;
		m_nMp4SStrmStartPts = 0;
		m_fSegmentSelfInit = 1;
		m_pSpsData = (char *)malloc(MAX_SPS_SIZE);
		m_pPpsData = (char *)malloc(MAX_PPS_SIZE);
		m_nSpsSize = 0;//MAX_SPS_SIZE;
		m_nPpsSize = 0;//MAX_PPS_SIZE;
		m_pMpdEmsg = new CMpdEmsg(pMpdRep->GetMpd());
	}
	~CMpdSegmnter()
	{
		delete m_pCrntGop;
		if(m_pMuxBuffer)
			free(m_pMuxBuffer);

		if(m_pMp4Mux){
			delete	m_pMp4Mux;
		}
		free(m_pSpsData);
		free(m_pPpsData);
	}

	int IsFilling( int nFileIdx)
	{
		return (nFileIdx >= m_nSeqStart);
	}

	int ProcessAVFrame(char *pData, int nLen, int fVideo, int fDisCont, unsigned long ulPtsMs);
	int ProcessEndOfSeq();

	int MuxTs(char *pInData, int nInLen, char *pOutData, int *pnOutLen, int fVideo, unsigned long *pulFlags, unsigned long ulPtsMs, int fDisCont);
	int MuxMp4(char *pInData, int nInLen, char *pOutData, int *pnOutLen, int fVideo, unsigned long ulFlags, unsigned long ulPtsMs, int fDisCont);
	int GenerateMp4InitSegment(char *pInData, int nInLen, int fVideo);
	int GetSpsPpsData(char *pData, int nLen);

	int WriteFrameData(char *pData, int nLen, int fVideo, unsigned long ulFlags, unsigned long ulPtsMs)
	{
		long lTimeout = WAIT_TIME_FOR_EMPTY_BUFF; // milli seconds
		unsigned long ulPtsHi = 0;
		unsigned long ulPtsLo = 0;
		char *pPkt = pData;
		int fHasSps = ulFlags & MPD_FLAG_HAS_SPS;
		int fDisCont = ulFlags & MPD_FLAG_DISCONT;

		if(ulFlags & MPD_FLAG_DISCONT ){
			if(m_pCrntGop->m_nLen > 0){
				UpdateOnNewGop();
			}
			NotifyDiscontinuity();
			// Wait again for SPS
			m_fHasSps = 0;
		}

		// Waiting for SPS and discard data, if no SPS yet
		if(!m_fHasSps && !fHasSps) {
			return 0;
		}

		if(fVideo && fHasSps ) {
			// Send the Current Gop
			m_ulNextGopStartPtsMs = ulPtsMs;
			UpdateOnNewGop();
			
			// Prepare for next GOP
			m_fHasSps = fHasSps;
			m_GopStartPtsMs = ulPtsMs;
		}
		if(m_pCrntGop){
			m_pCrntGop->AddData((unsigned char *)pData, nLen);
		}
		return 0;
	}

	void UpdateOnNewGop()
	{
		/* Save the current gop */
		if(m_pCrntGop && m_pCrntGop->m_nLen > 0) {
			m_pCrntGop->m_DurationMs = m_ulNextGopStartPtsMs - m_GopStartPtsMs;
			PublishGop(m_nCrntGopNum, (const char *)m_pCrntGop->m_pBuff, m_pCrntGop->m_nLen, m_GopStartPtsMs, m_pCrntGop->m_DurationMs);
			m_pCrntGop->Clear();
			m_nCrntGopNum++;
		} 
	}
	void NotifyDiscontinuity()
	{
		m_MutexPublish.Acquire();
		for (std::list<CMpdPublishBase *>::iterator it = m_PublishList.begin(); it != m_PublishList.end(); ++it) {
			CMpdPublishBase *pPublish = *it;
			pPublish->SetDisContinuity();
		}
		m_MutexPublish.Release();
	}


	void AddPublisher(CMpdPublishBase *pPublish)
	{
		m_MutexPublish.Acquire();
		m_PublishList.push_back(pPublish);
		m_MutexPublish.Release();
	}

	void RemovePublisher(CMpdPublishBase *pPublish)
	{
		m_MutexPublish.Acquire();
		for (std::list<CMpdPublishBase *>::iterator it = m_PublishList.begin(); it != m_PublishList.end(); ++it) {
			CMpdPublishBase *pTmp = *it;
			if(pTmp == pPublish) {
				m_PublishList.erase(it);
				break;
			}
		}
		m_MutexPublish.Release();
	}

	void PublishGop(int nGopNum, const char *pData, int nLen, int nStartPtsMs, int nDurarionMs)
	{
		m_MutexPublish.Acquire();
		for (std::list<CMpdPublishBase *>::iterator it = m_PublishList.begin(); it != m_PublishList.end(); ++it) {
			CMpdPublishBase *pPublish = *it;
			pPublish->ReceiveGop(nGopNum, pData, nLen, nStartPtsMs, nDurarionMs, m_pMpdEmsg);
		}
		m_MutexPublish.Release();
	}

	void PublishInitSegment(const char *pData, int nLen)
	{
		m_MutexPublish.Acquire();
		for (std::list<CMpdPublishBase *>::iterator it = m_PublishList.begin(); it != m_PublishList.end(); ++it) {
			CMpdPublishBase *pPublish = *it;
			pPublish->ReceiveInitSegment(pData, nLen);
		}
		m_MutexPublish.Release();
	}

	void PublishMoof(int nGopNum, const char *pData, int nLen, int nStartPtsMs, int nDurarionMs)
	{
		m_MutexPublish.Acquire();
		for (std::list<CMpdPublishBase *>::iterator it = m_PublishList.begin(); it != m_PublishList.end(); ++it) {
			CMpdPublishBase *pPublish = *it;
			pPublish->ReceiveGop(nGopNum, pData, nLen, nStartPtsMs, nDurarionMs, m_pMpdEmsg);
		}
		m_MutexPublish.Release();
	}

	std::list <CMpdPublishBase *>  m_PublishList;
	COsalMutex                     m_MutexPublish;

	int m_nWr;
	int m_fHasSps;
	int m_nSeqStart;

	CGop *m_pCrntGop;
	int  m_nCrntGopNum;
	int  m_nGopSize;
	int  m_GopStartPtsMs;
	int  m_ulNextGopStartPtsMs;

	int  m_nMuxType;
	char     *m_pMuxBuffer;
	CTsMux	  m_TsMux;
	CMp4MuxIf *m_pMp4Mux;
	void      *m_pMp4moof;
	char      *m_pEmsgBuff;
	char       *m_pSpsData;
	char       *m_pPpsData;
	int        m_nSpsSize;
	int        m_nPpsSize;

	int        m_fMp4InitSegment; // Flag to indicate if MP4 init segment is generated
	int        m_fMp4VidInit;     // Flag to indicate if MP4 video is initialized
	int        m_fMp4AudInit;     // Flag to indicate if MP4 video is initialized
	int        m_fVidEnable;
	int        m_fAudEnable;

	char       *m_pSegmentInitData;
	int        m_nSegmentInitLen;
	int        m_fSegmentSelfInit;

	int        m_nMoofDurationMs;
	int        m_nMoofCrntTime;
	int        m_nMp4SStrmPts;
	int        m_nMp4SStrmStartPts;
	CMpdRepresentation *m_pMpdRep;
	CMpdEmsg   *m_pMpdEmsg;
};

static void GetAacConfigFromAdts(char *pConfig, char *pAdts)
{
        unsigned char profileVal      = pAdts[2] >> 6;
        unsigned char sampleRateId    = (pAdts[2] >> 2 ) & 0x0f;
        unsigned char channelsVal     = ((pAdts[2] & 0x01) << 2) | ((pAdts[3] >> 6) & 0x03);
        pConfig[0]  = (profileVal + 1) << 3 | (sampleRateId >> 1);
        pConfig[1]  = ((sampleRateId & 0x01) << 7) | (channelsVal <<3);
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

static unsigned char *FindStartCode(
	 unsigned char *p, 
	 unsigned char *end)
{
    while( p < end - 3) {
        if( p[0] == 0 && p[1] == 0 && ((p[2] == 1) || (p[2] == 0 && p[3] == 1)) )
            return p;
		p++;
    }

    return NULL;
}

int CMpdSegmnter::GetSpsPpsData(char *pData, int nInLen)
{
	char *pSpsData = m_pSpsData; 
	char *pPpsData = m_pPpsData;

	unsigned char *pScPos = (unsigned char *)pData;
	int fDone = 0;
	while(!fDone && pScPos && pScPos < (unsigned char *)pData + nInLen - 4) {
		pScPos = FindStartCode(pScPos, (unsigned char *)pData + nInLen - 1);
		if(pScPos) {
			while (*pScPos == 0) pScPos++;
			pScPos++;
			int nal_unit_type = *pScPos & 0x1F;
			switch (nal_unit_type) {
				case 7:
				{
					unsigned char *pSpsEnd = FindStartCode(pScPos, (unsigned char *)pData + nInLen - 1);
					if(pSpsEnd > pScPos) {
						m_nSpsSize = pSpsEnd - pScPos;
						if(m_nSpsSize > MAX_SPS_SIZE) 
							m_nSpsSize =  MAX_SPS_SIZE;
						memcpy(pSpsData, pScPos, pSpsEnd - pScPos);
						
					}
					pScPos = pSpsEnd;
				}
				break;
				case 8:
				{
					unsigned char *pPpsEnd = FindStartCode(pScPos, (unsigned char *)pData + nInLen - 1);
					if(!pPpsEnd)
						pPpsEnd = (unsigned char *)pData + nInLen;
					if(pPpsEnd > pScPos) {
						m_nPpsSize = pPpsEnd - pScPos;
						if(m_nPpsSize > MAX_PPS_SIZE)
							m_nPpsSize = MAX_PPS_SIZE;
						memcpy(pPpsData, pScPos, pPpsEnd - pScPos);
					}
					fDone = 1;
				}
				break;
				default:
					break;
			}
		} else
			break;
	}
	return 0;
}

int CMpdSegmnter::GenerateMp4InitSegment(char *pInData, int nInLen, int fVideo)
{
	unsigned long dwWritten;
	CPacket Packet;
	//TODO: Add support of IBO
	if(!m_fMp4InitSegment) {
		// TODO
		if(fVideo && !m_fMp4VidInit) {
			char *pIndex = NULL;
			GetSpsPpsData(pInData,nInLen);
			if(m_nSpsSize && m_nPpsSize) {
				m_pMp4Mux->InitVideoTrack((unsigned char *)m_pSpsData, m_nSpsSize, (unsigned char *)m_pPpsData, m_nPpsSize);
				m_fMp4VidInit = 1;
			}
		} else if(!fVideo && !m_fMp4AudInit) {
			char ConfigData[2];
			GetAacConfigFromAdts(pInData, ConfigData);
			m_pMp4Mux->InitAudioTrack((unsigned char *)ConfigData, 2);
			m_fMp4AudInit = 1;
		}
		if((m_fMp4AudInit || !m_fAudEnable) && (!m_fVidEnable || m_fMp4VidInit)) {
			m_nSegmentInitLen = m_pMp4Mux->GenerateInitSegment(m_pSegmentInitData, MAX_INIT_SEGMENT_SIZE);
			m_fMp4InitSegment = 1;
			PublishInitSegment(m_pSegmentInitData, m_nSegmentInitLen);
		}
	}
	return 0;
}

int CMpdSegmnter::MuxTs(
		char           *pInData, 
		int            nInLen, 
		char           *pOutData, 
		int            *pnOutLen, 
		int            fVideo, 
		unsigned long  *pulFlags, 
		unsigned long  ulPtsMs, 
		int            fDisCont)
{
	int hr = -1;
	long ulFlags = 0;
	int nSearchLen;
	nSearchLen = nInLen < 32 ? nInLen : 32;				// Limit SPS search tp first 32 bytes
	if(HasSps((unsigned char *)pInData, nSearchLen)) {
		ulFlags |= MPD_FLAG_HAS_SPS;
	}
	if(fDisCont) {
		ulFlags |= MPD_FLAG_DISCONT;
	}
	*pulFlags = ulFlags;

	CAccessUnit Au;
	Au.m_TSStreamID = 0;
	Au.m_SampleStreamType = SAMPLE_TYPE_H264;
	Au.m_pRawData = (char *)pInData;
	Au.m_RawSize = nInLen;
	Au.m_pTsData = pOutData;
	Au.m_PTS = ulPtsMs;
	Au.m_DTS = Au.m_PTS;
	Au.m_Flags = 0;

	if(ulFlags & MPD_FLAG_HAS_SPS){
		Au.m_Flags = FORCE_SEND_PSI;
	}

	m_TsMux.Mux(&Au);
	*pnOutLen = Au.m_TsSize;
	hr = 0;

	return hr;
}

int CMpdSegmnter::MuxMp4(
		char           *pInData, 
		int            nInLen, 
		char           *pOutData, 
		int            *pnOutLen, 
		int            fVideo, 
		unsigned long  ulFlags, 
		unsigned long  ulPtsMs, 
		int            fDisCont)
{
	int hr = -1;

	CPacket Packet;

	if(!m_fMp4InitSegment) {
		GenerateMp4InitSegment(pInData, nInLen, fVideo);
	}

	// Do not wait for m_fMp4InitSegment which requires both sps and pps found.
	// SPS is already found, if we are here which is sufficient for proceeding with muxing
	//if(m_fMp4InitSegment) 
	{
		Packet.m_llPts = ulPtsMs;
		Packet.m_llDts = ulPtsMs; //TODO
		Packet.m_lDuration = 3003;//TODO tStop - tStart;
		Packet.m_pData = (unsigned char *)pInData;
		Packet.m_lSize = nInLen;
		Packet.m_StreamIndex = CMp4MuxIf::TRACK_INDEX_VIDEO;
		m_pMp4Mux->WritePacket(&Packet);
		hr = 0;
		// TODO
	}
	return hr;
}

int CMpdSegmnter::ProcessAVFrame(char *pData, int nLen, int fVideo, int fDisCont, unsigned long ulPtsMs)
{
	int res = 0;
	unsigned long ulFlags = 0;

	if(m_nMuxType == MPD_MUX_TYPE_TS) {
		int nTsLen = MAX_TS_BUFFER_SIZE;
		if(MuxTs(pData, nLen, m_pMuxBuffer, &nTsLen, fVideo, &ulFlags, ulPtsMs, fDisCont) == 0) {
			WriteFrameData(m_pMuxBuffer, nTsLen, 1, ulFlags, ulPtsMs / 90);
		}
	} else {
		if(fVideo) {
			int nOutLen = MAX_MP4_BUFFER_SIZE;

			long ulFlags = 0;
			int nSearchLen;
			nSearchLen = nLen < 32 ? nLen : 32;				// Limit SPS search tp first 32 bytes
			if(HasSps((unsigned char *)pData, nSearchLen) && (m_nMoofCrntTime == 0 || m_nMoofCrntTime >= m_nMoofDurationMs)) {
				ulFlags |= MPD_FLAG_HAS_SPS;
				if(m_pMp4moof) {
					CMoofParam MoofParam;
					nOutLen = m_pMp4Mux->CloseMoof(m_pMp4moof, &MoofParam);
					PublishMoof(m_nCrntGopNum, (const char *)m_pMuxBuffer, nOutLen, MoofParam.m_nStartPtsMs, m_nMoofCrntTime);
					m_nCrntGopNum++;
					m_nMoofCrntTime = 0;
				}
				m_pMp4moof = m_pMp4Mux->OpenMoof(m_pMuxBuffer, MAX_MP4_BUFFER_SIZE, m_fVidEnable + m_fAudEnable);
				m_nMp4SStrmStartPts = ulPtsMs;
			}
			if(fDisCont) {
				ulFlags |= MPD_FLAG_DISCONT;
				m_pMpdEmsg->Start(1200); // TODO tie with Segment duration
			}
			if(m_pMp4moof) {
				MuxMp4(pData, nLen, m_pMuxBuffer, &nOutLen, fVideo, ulFlags, ulPtsMs, fDisCont);
				if(ulPtsMs - m_nMp4SStrmPts < 200/*check*/)
					m_nMoofCrntTime += (ulPtsMs - m_nMp4SStrmPts);
				m_nMp4SStrmPts = ulPtsMs;
			}
		} else {
			int nErrAudNotsupported = 1;
		}
	}
	return res;
}

int CMpdSegmnter::ProcessEndOfSeq()
{
	if(m_nMuxType == MPD_MUX_TYPE_TS) {
		// Do nothing for now
	} else {
		if(m_pMp4moof) {
			CMoofParam MoofParam;
			int nOutLen;
			nOutLen = m_pMp4Mux->CloseMoof(m_pMp4moof, &MoofParam);
			PublishMoof(m_nCrntGopNum, (const char *)m_pMuxBuffer, nOutLen, m_nMp4SStrmStartPts, m_nMoofCrntTime);
			m_nCrntGopNum++;
			m_nMoofCrntTime = 0;
			m_pMp4moof = NULL;
		}
	}
	return 0;
}
//===============================================================================================

int CMpdPublishBase::GetStats(MPD_PUBLISH_STATS *pStats)
{
	pStats->nState = m_nError;
	pStats->nLostBufferTime = m_nLostBufferTime / 1000;
	pStats->nSegmentTime = m_nSegmentTime / 1000;
	pStats->nStreamInTime = m_nInStreamTime / 1000;
	pStats->nStreamOutTime = m_nOutStreamTime / 1000;
	return 0;
}

CMpdPublishBase::CMpdPublishBase()
{
	m_nInStreamTime = 0;
	m_nOutStreamTime = 0;
	m_nError = 0;

	m_nLostBufferTime = 0;
	m_nSegmentTime = 0;
	m_fSupportDiscont = 0;
	m_nStreamStartTime = time(NULL);
}


//
// Circualr buffer for storing H.264 Gops
// 
class CGopCircBuff
{
public:
	CGopCircBuff(int nBufSize) 
	{
		m_pData = (unsigned char *)malloc(nBufSize);
		m_nSize  = nBufSize; /* include empty elem */
		m_nRd = 0;
		m_nWr   = 0;
	}
 
	~CGopCircBuff() 
	{
		free(m_pData);
	}
 
	/*
	** Allocate memory for Gop data and advance the write ptr
	*/
	unsigned char *Alloc(int nSize) 
	{
		unsigned char *pData = 0;
		int nTailSize = 0;
		int nHeadSize = 0;
		
		if(m_nWr < m_nRd) {
			if(m_nRd - m_nWr > nSize) {
				pData = m_pData + m_nWr;
				m_nWr += nSize;
			}
		} else {
			int nTailSize = m_nSize - m_nWr - 1;
			if(nTailSize >= nSize) {
				pData = m_pData + m_nWr;
				m_nWr = m_nWr + nSize;
			} else if (m_nRd > nSize) {
				pData = m_pData;
				m_nWr = nSize;
			}
		}
		return pData;
	}

	/*
	** Free the Gop data AND advancE the read ptr
	*/
	void Free(unsigned char *pData, int nSize) 
	{
		int nTailSize = 0;
		int nHeadSize = 0;
		if(m_nRd  < m_nWr) {
			assert(pData ==  m_pData + m_nRd);
			m_nRd += nSize;
		} else {
			int nTailSize = m_nSize - m_nRd - 1;
			if(nTailSize >= nSize) {
				assert(pData ==  m_pData + m_nRd);
				m_nRd = m_nRd + nSize;
			} else {
				assert(pData == m_pData);
				m_nRd = nSize;
			}
		}
	}

	int         m_nSize;
	int         m_nRd;
	int         m_nWr;

	unsigned char *m_pData;
};

class CGopCb
{
public:
	unsigned char *m_pBuff;
	int           m_nLen;
	int           m_nGopNum;
	int           m_DurationMs;

	CGopCb(unsigned char *pData, int nLen)
	{
		m_pBuff = pData;
		m_nLen = nLen;
		m_DurationMs = 500;
	}
	~CGopCb()
	{
	}

	int AddData(unsigned char *pData, int nLen)
	{
		int nRes  = 0;
		memcpy(m_pBuff, pData, nLen);
		nRes = nLen;
		return nRes;
	}
};

/******************************************************************************
 * Uploads data from segmenter to http site or local file system
 */
#define TRANSFER_TYPE_PARTIAL 1
#define TRANSFER_TYPE_FULL    2

class CMpdPublishS3 : public CMpdPublishBase
{
public:
	CMpdPublishS3(
		int			nTotalTimeMs,
		void		*pSegmenter, 
		CMpdRepresentation        *pCfgRepresenation,
		const char	*pszSegmentPrefix,
		const char	*pszParentFolder,
		const char	*pszBucket,
		CJdAwsContext *pAwsContext,
		/*
		const char	*pszHost,
		const char	*szAccessId,
		const char	*szSecKey,
		*/
		int         fLiveOnly,
		int         nStartIndex,
		int         nSegmentDuration,
		int         nDestType
		):
			m_nTotalTimeMs(nTotalTimeMs),
			m_fLiveOnly(fLiveOnly)
	{
		// TODO: Pass as parameter
		m_nSegmentDurationMs = nSegmentDuration;
		if (nDestType == MPD_UPLOADER_TYPE_S3) {
			m_TransferType = TRANSFER_TYPE_PARTIAL;
			m_pHlsOut = new CSegmentWriteS3(pAwsContext,pszBucket/*, pszHost, szAccessId, szSecKey, 2*/);
			
		} else if (nDestType == MPD_UPLOADER_TYPE_DISC) {
			m_pHlsOut = new CMpdOutFs;
		}
		m_pszParentFolder = strdup(pszParentFolder);

		m_nGopDuration = 500;
		m_nGopSize = 1 * 1024 * 1024;
		m_pCb = new CGopCircBuff(10 * 1024 * 1024);
		
		m_fRunState = 0;

		m_pSegmenter = (CMpdSegmnter *)pSegmenter;
		m_nSegmentStartIndex = nStartIndex;//time(NULL);//1;
		m_nSegmentIndex = m_nSegmentStartIndex;
		m_nBcastFrontCache = S3_DEFAULT_FRONT_CACHE;
		m_nBcastBackCache = S3_DEFAULT_BACK_CACHE;
		m_fSupportDiscont = 1;
		m_nTimeshiftSegments = DEFUALT_NUM_TIMESHIFT_SEGS;
		m_fDiscont = 0;
		m_pszMpdBaseUrl = NULL;
		m_pszMpdFilePrefix = strdup(pszSegmentPrefix);
		m_pMpdRepresentation = pCfgRepresenation;
	}
	
	~CMpdPublishS3()
	{
		if(m_pszParentFolder) free(m_pszParentFolder);
		if(!m_SegmentList.empty()){
			for (std::list<CSegmentInf *>::iterator it=m_SegmentList.begin(); it!=m_SegmentList.end(); ++it)
			delete(*it);
			m_SegmentList.clear();
		}
		if(m_pszMpdFilePrefix)
			free(m_pszMpdFilePrefix);
	}

	int GetAvailBuffDuration()
	{
		int nDuration = 0;
		std::list<CGopCb *>::iterator it;
		m_Mutex.Acquire();
		for (it = m_pGopFilledList.begin(); it != m_pGopFilledList.end(); it++){
			nDuration += (*it)->m_DurationMs;
		}
		m_Mutex.Release();
		return nDuration;
	}
	int GetSegmentLen()
	{
		int nSegmentLen = 0;
		int nDuration = 0;
		std::list<CGopCb *>::iterator it;
		m_Mutex.Acquire();
		for (it = m_pGopFilledList.begin(); it != m_pGopFilledList.end(); it++){
			nSegmentLen += (*it)->m_nLen;
			nDuration += (*it)->m_DurationMs;
			if(nDuration >= m_nSegmentDurationMs)
				break;
		}
		m_Mutex.Release();
		return nSegmentLen;
	}
	int SegmentCount() { return m_nSegmentIndex - m_nSegmentStartIndex;}
	int ReceiveGop(int nGopNum, const char *pData, int nLen, int nStartPtsMs, int nDurarionMs, CMpdEmsg *pMpdEmsg);
	int ReceiveInitSegment(const char *pData, int nLen);
	DWORD Process();
	void Run();
	void Stop();
	void UpdateSlidingWindow();

#ifdef WIN32
static DWORD WINAPI thrdStreamHttpLiveUpload(	void *pArg);
#else
static void *thrdStreamHttpLiveUpload(void *pArg);
#endif
	void UpLoadBroadcastPlayList();
	void UpLoadFullPlayList(
		char			*pSegmentBaseName,
		int				nStartSegment,
		int				nCountSegments);

	int					m_nTotalTimeMs;

	char				*m_pszParentFolder;
	char                *m_pszMpdFilePrefix;
	char                *m_pszMpdBaseUrl;

	CSegmentWriteBase         *m_pHlsOut;
	pthread_t			thrdHandle;
	int                 m_fLiveOnly;
	int                 m_nSegmentDurationMs;
	int                 m_nCrntSegmentStart;

	std::list <CGopCb *>       m_pGopFilledList;
	std::list <CSegmentInf *>  m_SegmentList;

	CGopCircBuff        *m_pCb;
	COsalMutex          m_Mutex;

	int                 m_nGopSize;
	int                 m_nGopDuration;
	int                 m_fRunState;

    int                 m_nSegmentStartIndex;
	int                 m_nSegmentIndex;
	CMpdSegmnter		*m_pSegmenter;
	int                 m_fDiscont;
	CMpdRepresentation  *m_pMpdRepresentation;
	int                 m_TransferType;
};

int CMpdPublishS3::ReceiveInitSegment(const char *pData, int nLen)
{
	return 0;
}
int CMpdPublishS3::ReceiveGop(int nGopNum, const char *pData, int nLen, int nStartPtsMs, int nDurarionMs, CMpdEmsg *pMpdEmsg)
{
	unsigned char *pCbData;
	CGopCb *pGop;
	JDBG_LOG(CJdDbg::LVL_STRM,("%s:Enter", __FUNCTION__));
	m_nInStreamTime += nDurarionMs;
	m_Mutex.Acquire();
	JDBG_LOG(CJdDbg::LVL_MSG,("Enter:Receiving Gop %d:  nLen=%d nDurarionMs=%d", nGopNum, nLen, nDurarionMs));
	pCbData = m_pCb->Alloc(nLen);
	if(pCbData == NULL) {
		JDBG_LOG(CJdDbg::LVL_ERR,("No Buffer"));
		m_nLostBufferTime += nDurarionMs;
		goto Exit;
	}

	pGop = new  CGopCb(pCbData, nLen);
	memcpy(pGop->m_pBuff, pData, nLen);
	m_pGopFilledList.push_back(pGop);
Exit:
	m_Mutex.Release();
	JDBG_LOG(CJdDbg::LVL_STRM,("%s:Leave", __FUNCTION__));
	return 0;
}
//=================================================================================
// CSegmentFile is helper class used by CMpdPublishMemFile to write a local file
class CSegmentFile
{
public:
	CSegmentFile() 
	{
		m_fd = -1;
	}

	int Open(const char *pszParentFolder, const char *pszFile)
	{
		char szFile[MAX_FILE_NAME];
		sprintf(szFile, "%s/%s",pszParentFolder, pszFile);
		strncpy(m_szFileName, szFile, MAX_FILE_NAME);
		strcat (szFile, "_");
		m_fd  =  open(szFile,  O_CREAT|O_RDWR|O_TRUNC|O_BINARY ,
#ifdef WIN32
	_S_IREAD | _S_IWRITE);
#else
	 S_IRWXU | S_IRWXG | S_IROTH);
#endif
		if(m_fd < 0) {
			JDBG_LOG(CJdDbg::LVL_ERR,("!!! Failed to open %s!!!\n",pszParentFolder));
			return -1;
		}
		return 0;
	}

	int Write(const char *pData, int nLen)
	{
		int res =  0;
		if(m_fd > 0) {
			res = write(m_fd, pData, nLen);
		}
		return res;
	}

	void Close()
	{
		char szFile[MAX_FILE_NAME];
		close(m_fd);

		strcpy(szFile, m_szFileName);
		strcat(szFile, "_");
		Rename(szFile, m_szFileName);
		m_fd = -1;
	}

	int Send(const char *pszParentFolder, const char *pszFile, char *pData, int nLen)
	{
		int res = 0;
		if(Open(pszParentFolder, pszFile) ==0) {
			res = write(m_fd, pData, nLen);
			close(m_fd);
		} else {
#ifdef WIN32
			res = GetLastError();
#else
			res = -1;
#endif
		}
		return res;
	}

	static int Delete(const char *pszParentFolder, const char *pszFile)
	{
		int res = 0;
		char szFile[256];
		sprintf(szFile, "%s/%s",pszParentFolder, pszFile);
		unlink(szFile);
		return 0;
	}
	
	int IsValid()
	{
		return (m_fd == -1);
	}
private:
	void Rename(const char *oldName, const char *newName)
	{
		remove(newName);
		rename(oldName, newName);
	}
public:
	char m_szFileName[MAX_FILE_NAME];
	int  m_fd;
};

/******************************************************************************
 * Uploads data from segmenter to http site or local file system
 */
class CMpdPublishMemFile : public CMpdPublishBase
{
public:
	CMpdPublishMemFile(
		void		*pSegmenter, 
		CMpdRepresentation        *pMpdRepresentation,
		const char	*pszMpdSegmentPrefix,
		const char	*pszFolder,
		const char	*pszServerRoot,
		int          nStartIndex
		)
	{
		CMpdRoot         *m_pMpd = pMpdRepresentation->GetMpd();
		CMpdAdaptaionSet *pAdapSet= pMpdRepresentation->GetAdaptationSet();

		int          nSegmentDuration = m_pMpd->GetMaxSegmentDuration();
		int          nTimeShiftBuffer = m_pMpd->GetTimeShiftBuffer();
		char szHlsFolderPath[256];
		int  nTimeShiftSegments = nTimeShiftBuffer / nSegmentDuration;

		// TODO: Pass as parameter
		m_nSegmentDurationMs = nSegmentDuration;
		m_pSegmenter = (CMpdSegmnter *)pSegmenter;
		sprintf(szHlsFolderPath, "%s/%s", pszServerRoot, pszFolder);
		m_pszParentFolder = strdup(szHlsFolderPath);
		m_pszMpdFilePrefix = strdup(pszMpdSegmentPrefix);
		m_nSegmentStartIndex = nStartIndex;//time(NULL);//1;
		m_nSegmentIndex = m_nSegmentStartIndex;
		m_nSegmentCount = 0;
		m_CrntSegmentDuration = 0;
		m_nError = 0;
		m_fSupportDiscont = 1;
		m_nBcastFrontCache = MEM_FILE_DEFAULT_FRONT_CACHE;
		m_nBcastBackCache = MEM_FILE_DEFAULT_BACK_CACHE;

		if(nTimeShiftSegments > 1)
			m_nTimeshiftSegments = nTimeShiftSegments;
		else
			m_nTimeshiftSegments = DEFUALT_NUM_TIMESHIFT_SEGS;
		m_pszMpdBaseUrl = NULL;
		m_pMpdRepresentation = pMpdRepresentation;
		if (m_pMpdRepresentation->m_SegmentType == CMpdRepresentation::TYPE_SEGMENT_LIST) {
			if(m_pMpdRepresentation->GetInitializationSegment() == NULL)
				m_fSegmentSelfInit = 1;
			else
				m_fSegmentSelfInit = 0;
		} else if (m_pMpdRepresentation->m_SegmentType == CMpdRepresentation::TYPE_SEGMENT_TEMPLATE) {
			if(pAdapSet->GetInitializationSegmentUrl() == NULL)
				m_fSegmentSelfInit = 1;
			else
				m_fSegmentSelfInit = 0;
		}
		m_pSegmentInitData = NULL;
		m_nSegmentInitLen = 0;
		m_fLiveOnly = m_pMpd->IsDynamic();
	}
	
	~CMpdPublishMemFile()
	{
		if(m_pszParentFolder) free(m_pszParentFolder);
		if(m_pszMpdFilePrefix)
			free(m_pszMpdFilePrefix);
		if(!m_SegmentList.empty()){
			 for (std::list<CSegmentInf *>::iterator it=m_SegmentList.begin(); it!=m_SegmentList.end(); ++it)
				delete(*it);
			 m_SegmentList.clear();
		}
		if(m_pSegmentInitData)
			free(m_pSegmentInitData);
	}
	int ReceiveInitSegment(const char *pData, int nLen);
	int ReceiveGop(int nGopNum, const char *pData, int nLen, int nStartPtsMs, int nDurarionMs, CMpdEmsg *pMpdEmsg);
	void Run()
	{ 
		m_pSegmenter->AddPublisher(this);
		m_fRunState = 1;
		m_nError = MPD_UPLOAD_STATE_RUN;
	}
	void Stop() 
	{ 
		m_pSegmenter->RemovePublisher(this);
		m_fRunState = 0;
		m_nError = MPD_UPLOAD_STATE_STOP;
	}
	
	void SetDisContinuity()
	{
		if(m_MemFile.IsValid() && m_CrntSegmentDuration >  0) {
			m_MemFile.Close();
			m_nSegmentTime += m_CrntSegmentDuration;
			m_nSegmentCount++;
			UpdateSlidingWindow();
			m_nSegmentIndex++;
			m_CrntSegmentDuration = 0;
		}
		m_fDiscont = 1;
	}

	void UpLoadBroadcastPlayList();
	void UpdateSlidingWindow();

	CSegmentFile     m_MemFile;
	CMpdSegmnter		*m_pSegmenter;
	char				*m_pszParentFolder;

	char                *m_pszMpdBaseUrl;
	char                *m_pszMpdFilePrefix;
	int                 m_nSegmentDurationMs;
	int                 m_nSegmentIndex;
	int                 m_nSegmentStartIndex;
	int                 m_nSegmentCount;
	int                 m_CrntSegmentDuration;
	int                 m_nCrntSegmentStart;
	int                 m_fRunState;
	int                 m_nError;
	int                 m_fLiveOnly;
	std::list <CSegmentInf *>  m_SegmentList;
	int                 m_fDiscont;
	CMpdRepresentation  *m_pMpdRepresentation;

	char       *m_pSegmentInitData;
	int        m_nSegmentInitLen;
	int        m_fSegmentSelfInit;
};


void CMpdPublishMemFile::UpLoadBroadcastPlayList()
{
	int nSegmentCount;
	int nStartTime = 0;
	int nLen = 0;
	JDBG_LOG(CJdDbg::LVL_STRM,("%s:Enter", __FUNCTION__));

	CMpdRepresentation *pMdpRep = m_pMpdRepresentation;
	char *buffer = (char *)malloc(PLAYLIST_BUFFER_SIZE);
	UpdateSegmentList(pMdpRep, m_pszMpdFilePrefix, &m_SegmentList, m_nBcastFrontCache, m_nTimeshiftSegments, m_pSegmenter->m_nMuxType);
	JDBG_LOG(CJdDbg::LVL_STRM,("%s:Leave", __FUNCTION__));
}

void CMpdPublishMemFile::UpdateSlidingWindow()
{
	JDBG_LOG(CJdDbg::LVL_STRM,("%s:Enter", __FUNCTION__));
	int nDelSegment = 0;

	// Add segment to the tail
	CSegmentInf *pSegmentInf = new CSegmentInf(m_nSegmentIndex, m_nCrntSegmentStart, m_CrntSegmentDuration, m_fDiscont);
	m_SegmentList.push_back(pSegmentInf);
	m_fDiscont = 0;

	// Remove head segment
	if(m_SegmentList.size() > m_nTimeshiftSegments +  m_nBcastFrontCache + m_nBcastBackCache) {
		CSegmentInf *pSegmentInf = m_SegmentList.front();
		m_SegmentList.pop_front();
		nDelSegment = pSegmentInf->m_nSuffix;
		delete pSegmentInf;
	}

	if (m_pMpdRepresentation->m_SegmentType == CMpdRepresentation::TYPE_SEGMENT_LIST) {
		if(m_SegmentList.size()  >=  m_nTimeshiftSegments + m_nBcastBackCache + m_nBcastFrontCache) {
			UpLoadBroadcastPlayList();
		}
	}
	// delete unused segment
	if(nDelSegment) {
		char szTsFile[256];
		char szTsFilePath[256];
		GetSegmentNameFromIndex(szTsFile, m_pszMpdFilePrefix, nDelSegment, m_pSegmenter->m_nMuxType);
		sprintf(szTsFilePath, "%s/%s",m_pszParentFolder, szTsFile);
		unlink(szTsFilePath);
	}

	JDBG_LOG(CJdDbg::LVL_STRM,("%s:Leave", __FUNCTION__));
}

int CMpdPublishMemFile::ReceiveInitSegment(const char *pData, int nLen)
{
	int res = 0;
	JDBG_LOG(CJdDbg::LVL_STRM,("%s:Enter", __FUNCTION__));

	if(m_fSegmentSelfInit)	{
		m_pSegmentInitData = (char *)malloc(nLen);
		memcpy(m_pSegmentInitData, pData, nLen);
		m_nSegmentInitLen = nLen;
	} else {
		char szTsFile[256];
		GetSegmentNameFromIndex(szTsFile, m_pszMpdFilePrefix, 0/*m_nSegmentIndex*/, m_pSegmenter->m_nMuxType);
		if(m_MemFile.Open(m_pszParentFolder, szTsFile) != 0){
			m_nError = MPD_UPLOAD_ERROR_XFR_FAIL;
			JDBG_LOG(CJdDbg::LVL_ERR,("Failed to open %s", szTsFile));
			goto Exit;
		}
		m_MemFile.Write(pData, nLen);
		m_MemFile.Close();

		//std::string segemtnInitUrl = szTsFile;
		//m_pMpdRepresentation->SetInitializationSegment(&segemtnInitUrl);
	}

Exit:
	JDBG_LOG(CJdDbg::LVL_STRM,("%s:Leave", __FUNCTION__));
	return 0;
}
int CMpdPublishMemFile::ReceiveGop(int nGopNum, const char *pData, int nLen, int nStartPtsMs, int nDurarionMs, CMpdEmsg *pMpdEmsg)
{
	int res = 0;
	JDBG_LOG(CJdDbg::LVL_STRM,("%s:Enter", __FUNCTION__));
	m_nInStreamTime += nDurarionMs;
	
	if(m_CrntSegmentDuration ==  0) {
		char szTsFile[256];
		GetSegmentNameFromIndex(szTsFile, m_pszMpdFilePrefix, m_nSegmentIndex, m_pSegmenter->m_nMuxType);

		{
			int nTimeOffset = (m_nSegmentIndex - m_nSegmentStartIndex) * m_nSegmentDurationMs / 1000;
			int nTime = time(NULL);
			m_nCrntSegmentStart = nStartPtsMs;
			JDBG_LOG(CJdDbg::LVL_MSG,("%d: Segmentoffset=%d SegmentTime=%d File=%s", nTime, m_nSegmentIndex - m_nSegmentStartIndex, m_nSegmentStartIndex + nTimeOffset, szTsFile));
		}
		if(m_MemFile.Open(m_pszParentFolder, szTsFile) != 0){
			m_nError = MPD_UPLOAD_ERROR_XFR_FAIL;
			JDBG_LOG(CJdDbg::LVL_ERR,("Failed to open %s", szTsFile));
			goto Exit;
		}
		
		if(pMpdEmsg && pMpdEmsg->m_fActive) {
			pMpdEmsg->GenerateMsg();
			if(pMpdEmsg->m_nLen) {
				m_MemFile.Write(pMpdEmsg->m_pBuffer, pMpdEmsg->m_nLen);
			}
			pMpdEmsg->DecrementPresentationTimeDelta(m_nSegmentDurationMs / 1000);
		}

		if(m_fSegmentSelfInit) {
			m_MemFile.Write(m_pSegmentInitData, m_nSegmentInitLen);
		}
	}

	if(m_MemFile.Write(pData, nLen) == nLen) {
		m_CrntSegmentDuration += nDurarionMs;
		m_nOutStreamTime += nDurarionMs;
	} else {
		m_nError = MPD_UPLOAD_ERROR_XFR_FAIL;
		JDBG_LOG(CJdDbg::LVL_ERR,("Failed to write"));
		m_nLostBufferTime += nDurarionMs;
		goto Exit;
	}

	//if(m_CrntSegmentDuration >=  m_nSegmentDurationMs)
	{
		int nCrntStrmTime = (time(NULL) - m_nStreamStartTime);
		int nNextSegmentTIme = (m_nSegmentIndex - m_nSegmentStartIndex + 1) * m_nSegmentDurationMs / 1000;
		if(nCrntStrmTime >= nNextSegmentTIme) {
			m_MemFile.Close();
			m_nSegmentTime += m_CrntSegmentDuration;
			if(m_fLiveOnly)
				UpdateSlidingWindow();
			m_nSegmentIndex++;
			m_CrntSegmentDuration = 0;
		}
	}
Exit:
	JDBG_LOG(CJdDbg::LVL_STRM,("%s:Leave", __FUNCTION__));
	return 0;
}



#define	MAX_FILE_NAME	 256

void CMpdPublishS3::UpLoadFullPlayList(
		char			*pSegmentBaseName,
		int				nStartSegment,
		int				nCountSegments)
{
	
	JDBG_LOG(CJdDbg::LVL_TRACE,("%s:Enter", __FUNCTION__));
	//TODO
	JDBG_LOG(CJdDbg::LVL_TRACE,("%s:Leave", __FUNCTION__));
}


void CMpdPublishS3::UpLoadBroadcastPlayList()
{
	int nSegmentCount;
	int nLen = 0;
	CMpdRepresentation *pMdpRep = m_pMpdRepresentation;

	JDBG_LOG(CJdDbg::LVL_TRACE,("%s:Enter", __FUNCTION__));

	int nSize = m_SegmentList.size();
	if(nSize > m_nTimeshiftSegments) {
		nSegmentCount = m_nTimeshiftSegments;
	} else {
		nSegmentCount = nSize;
	}

	int nBcastFrontCache = 0;
	if(nSize > nSegmentCount + m_nBcastBackCache ) {
		// Apply back cache, if there are more elements than nSegmentCount + m_nBcastBackCache
		nBcastFrontCache = nSize - (nSegmentCount + m_nBcastBackCache) >  m_nBcastFrontCache ? m_nBcastFrontCache : nSize - (nSegmentCount + m_nBcastBackCache);
	} 
	UpdateSegmentList(pMdpRep, m_pszMpdFilePrefix, &m_SegmentList, m_nBcastFrontCache, m_nTimeshiftSegments, m_pSegmenter->m_nMuxType);

Exit:
	JDBG_LOG(CJdDbg::LVL_STRM,("%s:Leave", __FUNCTION__));
}

/**
 * HttpLiveUploadCameraHandler : Stores the data in the given folder with http put request for each segment
 * Saves the html file and playlist at the end of streaming.
 * TODO: implement termination
 */
#ifdef WIN32
DWORD WINAPI CMpdPublishS3::thrdStreamHttpLiveUpload(	void *pArg)
#else
void *CMpdPublishS3::thrdStreamHttpLiveUpload(	void *pArg)
#endif
{
	CMpdPublishS3 *pUpload = (CMpdPublishS3 *)pArg;
	JDBG_LOG(CJdDbg::LVL_TRACE,("%s:Enter", __FUNCTION__));
	
	pUpload->Process();

	JDBG_LOG(CJdDbg::LVL_STRM,("%s:Leave", __FUNCTION__));
#ifdef WIN32
	return 0;
#else
	return (void *)0;
#endif
}

void CMpdPublishS3::Run()
{
	JDBG_LOG(CJdDbg::LVL_TRACE,("%s:Enter", __FUNCTION__));
	m_pSegmenter->AddPublisher(this);
	m_fRunState = 1;
	m_nError = MPD_UPLOAD_STATE_RUN;
#ifdef WIN32
	DWORD dwThreadId;
    thrdHandle = CreateThread(NULL, 0, CMpdPublishS3::thrdStreamHttpLiveUpload, this, 0, &dwThreadId);
#else
	pthread_create(&thrdHandle, NULL, CMpdPublishS3::thrdStreamHttpLiveUpload, this);
#endif
	thrdHandle = thrdHandle;
	JDBG_LOG(CJdDbg::LVL_STRM,("%s:Leave", __FUNCTION__));
}

void CMpdPublishS3::Stop()
{
	JDBG_LOG(CJdDbg::LVL_TRACE,("%s:Enter", __FUNCTION__));
	m_pSegmenter->RemovePublisher(this);
	m_fRunState = 0;
	m_nError = MPD_UPLOAD_STATE_STOP;
#ifdef WIN32
	WaitForSingleObject(thrdHandle, 3000);
#endif
	JDBG_LOG(CJdDbg::LVL_TRACE,("%s:Leave", __FUNCTION__));
}

void CMpdPublishS3::UpdateSlidingWindow()
{
	JDBG_LOG(CJdDbg::LVL_TRACE,("%s:Enter", __FUNCTION__));
	int nDeleteIdx = 0;
	
	/* Save the new segment */
	CSegmentInf *pSegmentInf = new CSegmentInf(m_nSegmentIndex, m_nCrntSegmentStart, m_nSegmentDurationMs, m_fDiscont);
	m_SegmentList.push_back(pSegmentInf);

	/* Delete old segment falling out of sliding window */
	if(m_SegmentList.size() > m_nTimeshiftSegments + m_nBcastFrontCache + m_nBcastBackCache) {
		CSegmentInf *pSegmentInf = m_SegmentList.front();
		m_SegmentList.pop_front();
		nDeleteIdx = pSegmentInf->m_nSuffix;
		delete pSegmentInf;
	}

	// Upload updated m3u8
	if(m_SegmentList.size()  >=  m_nTimeshiftSegments + m_nBcastBackCache + m_nBcastFrontCache) {
		JDBG_LOG(CJdDbg::LVL_MSG,("Upload playlist: url=%s playlist=%s\n", m_pszParentFolder, m_pszMpdFilePrefix));
		UpLoadBroadcastPlayList();
	}
	// Delete prev head segment for LiveOnly upload
	if(m_fLiveOnly && nDeleteIdx) {
		char szOldSegmentFileName[256];
		GetSegmentNameFromIndex(szOldSegmentFileName, m_pszMpdFilePrefix, nDeleteIdx,  m_pSegmenter->m_nMuxType);
		m_pHlsOut->Delete(m_pszParentFolder, szOldSegmentFileName);
	}

	JDBG_LOG(CJdDbg::LVL_STRM,("%s:Leave", __FUNCTION__));
}

#if 0
DWORD CMpdPublishS3::Process()
{
	int len, lfd;
	char *ChunkStart;
	char *buffer;
	int nContentLen = 0;
	int fChunked = 0;
	int fDone = 0;

	JDBG_LOG(CJdDbg::LVL_TRACE,("%s:Enter", __FUNCTION__));
	CSegmentWriteBase *pHlsOut = m_pHlsOut;
	int nSegmentDurationMs;

	JDBG_LOG(CJdDbg::LVL_MSG,("HttpLive Request. parent url=%s\n", m_pszParentFolder));

	char szTsFileName[MAX_FILE_NAME];

	while(!fDone){
		int lTimeout = m_nSegmentDurationMs * 2;
		int nCrntDuration = 0;
		while(GetAvailBuffDuration() < m_nSegmentDurationMs && /*lTimeout > 0 &&*/ m_fRunState) {
			lTimeout -= 100;
			OSAL_WAIT(100);
		}

		if(/*lTimeout <= 0 ||*/ !m_fRunState){
			JDBG_LOG(CJdDbg::LVL_MSG,("Timeout(%d) while waiting for filled buffer", m_nSegmentDurationMs * 2));
			fDone = true;
			continue;
		}
		
		/* Segment Available */
		GetSegmentNameFromIndex(szTsFileName, m_pszMpdFilePrefix, m_nSegmentIndex,  m_pSegmenter->m_nMuxType);
		int nTotlaLen = GetSegmentLen();
		int nBytesSent = 0;
		JDBG_LOG(CJdDbg::LVL_MSG,("m_nSegmentIndex=%d nTotlaLen=%d: numGops=%d SegDurationMs=%d nCrntDuration=%d",m_nSegmentIndex, nTotlaLen, m_pGopFilledList.size()));
		if( m_pHlsOut->Start(m_pszParentFolder,szTsFileName,nTotlaLen, NULL,0,CONTENT_STR_MP2T) == JD_ERROR){
			JDBG_LOG(CJdDbg::LVL_MSG,(":Start: Exiting due to error writing: %s", szTsFileName));
			m_nError = MPD_UPLOAD_ERROR_CONN_FAIL;
			goto Exit;
		}

		nSegmentDurationMs = m_nSegmentDurationMs;
		// TODO: Handle discont
		while(nCrntDuration < nSegmentDurationMs) {
			m_Mutex.Acquire();
			JDBG_LOG(CJdDbg::LVL_MSG,(":Uploading Segment %d: numGops=%d SegDurationMs=%d nCrntDuration=%d",  m_nSegmentIndex, m_pGopFilledList.size(), nSegmentDurationMs, nCrntDuration));
			
			if(m_pGopFilledList.size() <= 0) {
				JDBG_LOG(CJdDbg::LVL_MSG,("Error:Unexpected Size %d!!!",m_pGopFilledList.size()));
			}
			CGopCb *pGop = m_pGopFilledList.front();
			nBytesSent += pGop->m_nLen;
			m_pGopFilledList.pop_front();
			m_Mutex.Release();
			
			if(m_pHlsOut->Continue((char *)pGop->m_pBuff, pGop->m_nLen) == JD_ERROR) {
				JDBG_LOG(CJdDbg::LVL_MSG,(":Continue: Exiting due to error writing: %s", szTsFileName));
				m_nError = MPD_UPLOAD_ERROR_XFR_FAIL;
				goto Exit;
			}
			nCrntDuration += pGop->m_DurationMs;
			m_nOutStreamTime += pGop->m_DurationMs;
			m_Mutex.Acquire();
			m_pCb->Free(pGop->m_pBuff, pGop->m_nLen);
			m_Mutex.Release();
			delete pGop;;
		}
		m_nSegmentTime += nSegmentDurationMs;

		if(m_pHlsOut->End(NULL, 0) == JD_ERROR){
			JDBG_LOG(CJdDbg::LVL_MSG,(":End: Exiting due to error writing: %s", szTsFileName));
			m_nError = MPD_UPLOAD_ERROR_XFR_FAIL;
			goto Exit;
		}
		
		UpdateSlidingWindow();

		m_nSegmentIndex++;
	}
	if(SegmentCount() > 0 && !m_fLiveOnly){
		//TODO:
		//UpLoadFullPlayList(m_pszMpdFilePrefix, m_nSegmentStartIndex, SegmentCount());
	}
Exit:
	m_fRunState = 0;
	JDBG_LOG(CJdDbg::LVL_STRM,("%s:Leave Exiting...", __FUNCTION__));
	return 0;
}
#endif


#define MAX_SEGMENT_SIZE 8*1024*1024
DWORD CMpdPublishS3::Process()
{
	int len, lfd;
	char *ChunkStart;
	char *buffer;
	int offset;
	int nContentLen = 0;
	int fChunked = 0;
	int fDone = 0;

	JDBG_LOG(CJdDbg::LVL_TRACE,("%s:Enter", __FUNCTION__));
	CSegmentWriteBase *pHlsOut = m_pHlsOut;
	int nSegmentDurationMs;

	JDBG_LOG(CJdDbg::LVL_MSG,("HttpLive Request. parent url=%s\n", m_pszParentFolder));

	char szTsFileName[MAX_FILE_NAME];
	buffer = (char *)malloc(MAX_SEGMENT_SIZE);
	while(!fDone){
		int lTimeout = m_nSegmentDurationMs * 2;
		int nCrntDuration = 0;
		while(GetAvailBuffDuration() < m_nSegmentDurationMs && /*lTimeout > 0 &&*/ m_fRunState) {
			lTimeout -= 100;
			OSAL_WAIT(100);
		}

		if(/*lTimeout <= 0 ||*/ !m_fRunState){
			JDBG_LOG(CJdDbg::LVL_MSG,("Timeout(%d) while waiting for filled buffer", m_nSegmentDurationMs * 2));
			fDone = true;
			continue;
		}

		/* Segment Available */
		GetSegmentNameFromIndex(szTsFileName, m_pszMpdFilePrefix, m_nSegmentIndex,  m_pSegmenter->m_nMuxType);
		int nTotlaLen = GetSegmentLen();
		int nBytesSent = 0;
		JDBG_LOG(CJdDbg::LVL_MSG,("m_nSegmentIndex=%d nTotlaLen=%d: numGops=%d SegDurationMs=%d nCrntDuration=%d",m_nSegmentIndex, nTotlaLen, m_pGopFilledList.size()));
		if(m_TransferType == TRANSFER_TYPE_PARTIAL) {
			if( m_pHlsOut->Start(m_pszParentFolder,szTsFileName,nTotlaLen, NULL,0,CONTENT_STR_MP2T) == JD_ERROR){
				JDBG_LOG(CJdDbg::LVL_MSG,(":Start: Exiting due to error writing: %s", szTsFileName));
				m_nError = MPD_UPLOAD_ERROR_CONN_FAIL;
				goto Exit;
			}
		} else {
			offset = 0;
		}
		nSegmentDurationMs = m_nSegmentDurationMs;
		// TODO: Handle discont
		while(nCrntDuration < nSegmentDurationMs) {
			m_Mutex.Acquire();
			JDBG_LOG(CJdDbg::LVL_MSG,(":Uploading Segment %d: numGops=%d SegDurationMs=%d nCrntDuration=%d",  m_nSegmentIndex, m_pGopFilledList.size(), nSegmentDurationMs, nCrntDuration));

			if(m_pGopFilledList.size() <= 0) {
				JDBG_LOG(CJdDbg::LVL_MSG,("Error:Unexpected Size %d!!!",m_pGopFilledList.size()));
			}
			CGopCb *pGop = m_pGopFilledList.front();
			nBytesSent += pGop->m_nLen;
			m_pGopFilledList.pop_front();
			m_Mutex.Release();
			if(m_TransferType == TRANSFER_TYPE_PARTIAL) {
				if(m_pHlsOut->Continue((char *)pGop->m_pBuff, pGop->m_nLen) == JD_ERROR) {
					JDBG_LOG(CJdDbg::LVL_MSG,(":Continue: Exiting due to error writing: %s", szTsFileName));
					m_nError = MPD_UPLOAD_ERROR_XFR_FAIL;
					goto Exit;
				}
			} else {
				if(offset + pGop->m_nLen < MAX_SEGMENT_SIZE ) {
					memcpy(buffer+offset, (char *)pGop->m_pBuff, pGop->m_nLen);
					offset += pGop->m_nLen;
				} else {
					// Buffer overrun
				}
			}
			nCrntDuration += pGop->m_DurationMs;
			m_nOutStreamTime += pGop->m_DurationMs;
			m_Mutex.Acquire();
			m_pCb->Free(pGop->m_pBuff, pGop->m_nLen);
			m_Mutex.Release();
			delete pGop;;
		}
		m_nSegmentTime += nSegmentDurationMs;
		if(m_TransferType == TRANSFER_TYPE_PARTIAL) {
			if(m_pHlsOut->End(NULL, 0) == JD_ERROR){
				JDBG_LOG(CJdDbg::LVL_MSG,(":End: Exiting due to error writing: %s", szTsFileName));
				m_nError = MPD_UPLOAD_ERROR_XFR_FAIL;
				goto Exit;
			}
		}
		std::time_t req_time = std::time(NULL);
		m_pHlsOut->Send(m_pszParentFolder,szTsFileName, req_time, buffer, nTotlaLen, CONTENT_STR_MP2T, 30);
		UpdateSlidingWindow();

		m_nSegmentIndex++;
	}
	if(SegmentCount() > 0 && !m_fLiveOnly){
		//TODO:
		//UpLoadFullPlayList(m_pszMpdFilePrefix, m_nSegmentStartIndex, SegmentCount());
	}
Exit:
	if(buffer)
		free(buffer);
	m_fRunState = 0;
	JDBG_LOG(CJdDbg::LVL_STRM,("%s:Leave Exiting...", __FUNCTION__));
	return 0;
}

void *mpdPublishStart(
			int		    nTotalTimeMs, 
			void	    *pSegmenter, 
			CMpdRepresentation  *pMpdRep,
			const char	*pszSegmentPrefix,
			const char	*pszParentFolder,
 			const char	*pszBucketOrSvrRoot, 
			/*const char	*pszHost,
 			const char	*szAccessId,
			const char	*szSecKey,*/
			CJdAwsContext  *pServerCtxt,
			int         fLiveOnly,
			int         nStartIndex,
			int         nDestType
			)
{
	JDBG_LOG(CJdDbg::LVL_TRACE,("%s:Enter", __FUNCTION__));
	CMpdPublishBase *pPublisher = NULL;
#if 1
	int nSegmentDurationMs = 1000;
	if(nDestType == MPD_UPLOADER_TYPE_S3 || nDestType == MPD_UPLOADER_TYPE_DISC) {
		if(nTotalTimeMs == -1)
			nTotalTimeMs = MAX_UPLOAD_TIME;
		pPublisher = new CMpdPublishS3(nTotalTimeMs, pSegmenter, pMpdRep, pszSegmentPrefix, pszParentFolder,
			pszBucketOrSvrRoot, /*pszHost, szAccessId, szSecKey,*/ pServerCtxt, fLiveOnly, nStartIndex, nSegmentDurationMs, nDestType);
	} else 
#endif
	{
		pPublisher = new CMpdPublishMemFile(pSegmenter, pMpdRep, pszSegmentPrefix, pszParentFolder, pszBucketOrSvrRoot, nStartIndex);
	}
	pPublisher->Run();
	
	JDBG_LOG(CJdDbg::LVL_STRM,("%s:Leave", __FUNCTION__));
	return pPublisher;
}

void mpdPublishStop(void *pUploadCtx)
{
	JDBG_LOG(CJdDbg::LVL_TRACE,("%s:Enter", __FUNCTION__));
	CMpdPublishBase *pHlsUpload = (CMpdPublishBase *)pUploadCtx;
	pHlsUpload->Stop();
	JDBG_LOG(CJdDbg::LVL_STRM,("%s:Leave", __FUNCTION__));
}

int mpdPublishGetStats(void *pUploadCtx, int *pnState, int *pnStreamInTime, int *pnLostBufferTime,  int *pnStreamOutTime, int *pnSegmentTime)
{
	JDBG_LOG(CJdDbg::LVL_TRACE,("%s:Enter", __FUNCTION__));
	MPD_PUBLISH_STATS Stats;
	CMpdPublishBase *pHlsUpload = (CMpdPublishBase *)pUploadCtx;
	pHlsUpload->GetStats(&Stats);
	*pnState = Stats.nState;
	*pnSegmentTime = Stats.nSegmentTime;
	*pnLostBufferTime = Stats.nLostBufferTime;
	*pnStreamInTime = Stats.nStreamInTime;
	*pnStreamOutTime = Stats.nStreamOutTime;

	JDBG_LOG(CJdDbg::LVL_STRM,("%s:Leave", __FUNCTION__));
	return 0;
}


void *mpdCreateSegmenter(CMpdRepresentation *pMpdRep)
{
	JDBG_LOG(CJdDbg::LVL_TRACE,("%s:Enter", __FUNCTION__));
	CMpdSegmnter *pMpdSegmenter = new CMpdSegmnter(pMpdRep);
	JDBG_LOG(CJdDbg::LVL_STRM,("%s:Leave", __FUNCTION__));
	return pMpdSegmenter;
}

void mpdDeleteSegmenter(void *pSegmenter)
{
	JDBG_LOG(CJdDbg::LVL_TRACE,("%s:Enter", __FUNCTION__));
	
	CMpdSegmnter *pHlsSegmenter = (CMpdSegmnter *)pSegmenter;
	delete pHlsSegmenter;
	
	JDBG_LOG(CJdDbg::LVL_STRM,("%s:Leave", __FUNCTION__));
}


/**
 * mpdWriteFrameData is expored to encoder to supply the data
 */
int mpdWriteFrameData(void *pCtx, char *pData, int nLen, int fVideo, int fDiscont,  unsigned long ulPtsMs)
{
	JDBG_LOG(CJdDbg::LVL_TRACE,("%s:Enter", __FUNCTION__));
	int res = 0;
	CMpdSegmnter *pMpdLiveSgmnt = (CMpdSegmnter *)pCtx;
	if(pMpdLiveSgmnt) {
		res =  pMpdLiveSgmnt->ProcessAVFrame(pData, nLen,  fVideo, fDiscont, ulPtsMs);
	} else {
		JDBG_LOG(CJdDbg::LVL_ERR,("mpdWriteFrameData: Invalid Segmenter"));
		res = -1;
	}
	JDBG_LOG(CJdDbg::LVL_STRM,("%s:Leave", __FUNCTION__));
	return res;
}

int mpdEndOfSeq(void *pCtx)
{
	int res = 0;
	CMpdSegmnter *pMpdLiveSgmnt = (CMpdSegmnter *)pCtx;
	if(pMpdLiveSgmnt) {
		res =  pMpdLiveSgmnt->ProcessEndOfSeq();
	}
	return res;
}

void mpdSetDebugLevel(int nLevel)
{
	modDbgLevel = nLevel;
}
