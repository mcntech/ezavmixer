#ifdef WIN32
#include <windows.h>
#include <tchar.h>
#else
#include <unistd.h>
#include <stdarg.h>
#endif

#include <assert.h>
#include <queue>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>

using std::queue;
using std::pair;
//#include "strmconn_ipc.h"
#include "strmconn_zmq.h"

#include "Mp4DemuxIf.h"
#include "Mp4StrmSrc.h"

#define SRC_TYPE_FILE		0
#define SRC_TYPE_RTSP		1
#define SRC_TYPE_HTTP		2
#define SRC_TYPE_HTTPLIVE	3

#define DEFUALT_VID_WIDTH	640//1280
#define DEFUALT_VID_HEIGHT	480 //720


#define MAX_DEMUX_BUFF				(4 * 1024 * 1024)
#ifdef EN_TS
#define JB_PKT_COUNT		2048
#define JB_PKT_SIZE			2*1024
#else
#define JB_PKT_COUNT		12
#define JB_PKT_SIZE			512*1024
#endif

namespace Mp4StrmSrc 
{
/**
 * Contains one or more TS packets for audio or video
 */
class CAvSample
{
public:
	CAvSample(long lLen) 
	{
		m_pData = (char *)malloc(lLen);
		m_lMaxLen = lLen; 
		m_llPts = 0;
		m_lUsedLen = 0;
		m_fEoS = 0;
	}
	~CAvSample()
	{
		if(m_pData) free(m_pData);
	}
	char	  *m_pData;
	long	  m_lUsedLen;
	long	  m_lMaxLen;
	long long m_llPts;
	int       m_fEoS;
};

class ITrackAssocInf
{
public:
	virtual ~ITrackAssocInf(){}
	virtual int getTrackId() {return 0;}
	virtual void setTrackId(int nTrackId){}
	virtual void setTrackEoF(){}
	virtual long SendBuffer(CAvSample *pAvSample) = 0;
	virtual bool IsStreaming(){return 1;}
};

enum {
	CMD_STOP
};


/**
 * Implementation source filter
 * Stream is obtained from file, rtsp or http live source.
 */
class CMp4StrmSrc : public CStrmSourceIf
{
public:
    int Load(const char *pszFileName);

	CMp4StrmSrc();
	virtual ~CMp4StrmSrc();
	void Stop();
	void Run(long long llTime);

	long Seek(int TrackId, long long llTime);

	long CreateH264OutputPin();
	long CreateMP4AOutputPin();

	//void SetIpcParam(const char *szAudLocal, const char *szAudPeer, const char *szVidLocal, const char *szVidPeer)
	void SetIpcParam(ConnCtxT *pAudConn, ConnCtxT *pVidConn)
	{
		m_pAudConn = pAudConn;
		m_pVidConn = pVidConn;
		//m_pszAudSockLocal = strdup(szAudLocal);
		//m_pszAudSockPeer = strdup(szAudPeer);
		//m_pszVidSockLocal = strdup(szVidLocal);
		//m_pszVidSockPeer = strdup(szVidPeer);
	}
	static void *DoBufferProcessing(void *pObj);
	long DemuxStream();
	int RateCtrlSyncDelivery(long long nStreamTimeMs);

	void SetStartPts(long long llPtsMs);
	int SetEventCallback(fnEventCallback_t pCallback);

	int     m_nAudCodec;
	int     m_nVidCodec;
	int     m_fRun;
	int     m_nVidWidth;
	int     m_nVidHeight;
private:
		int InitFileSrc(const char *pszFileName);
	long DeinitFileSrc();
	int GetDuration();
public:
	Mp4Demux::CAacCfgRecord	AacCfgRecord;
	int     m_AudSampleRate;
	int     m_AudNumChannels;
	int     m_iPins;
	ITrackAssocInf *m_paStreams[2];
private:
    // It is only allowed to to create these objects with CreateInstance
	long long          m_llSize;
    unsigned char      *m_pbData;
	char               *m_pFileName;

	unsigned long long  m_uliSize;

	int					m_nInitialBuffer;
	int					m_nInitialSgmnts;		// Number of initial segments
	char                m_szSource[256];
	Mp4Demux::CMp4DemuxIf    *m_pFileReader;

	virtual long OnThreadStartPlay(void);
	virtual long OnThreadDestroy(void);
#ifdef WIN32
	HANDLE    m_hThread;
#else
	pthread_t m_hThread;
#endif
	CAvSample *m_pAvSample;
	//char       *m_pszAudSockLocal;
	//char       *m_pszAudSockPeer;
	//char       *m_pszVidSockLocal;
	//char       *m_pszVidSockPeer;
	ConnCtxT       *m_pAudConn;
	ConnCtxT       *m_pVidConn;
    unsigned int m_lTimescale;
	long long   m_llTtrackDurationMs;
	long long   m_llStartPtsMs;
	int         m_fRateControl;
	int         m_nStartSystemTimeMs;
	int         m_nStartStreamTimeMs;
	fnEventCallback_t m_pCallback;
};

class CH264Stream : public ITrackAssocInf
{
public:
    CH264Stream(long *phr,  CMp4StrmSrc *pParent, ConnCtxT *pConn, const char *pszName);
    virtual ~CH264Stream();
    long SendBuffer(CAvSample *pSample);

	int getTrackId(){return m_nTrackId;}
	void setTrackId(int nTrackId){m_nTrackId = nTrackId;}
	void setTrackEoF()
	{
		unsigned long ulFlags = OMX_BUFFERFLAG_EOS;
		m_pVidConn->Write(m_pVidConn, NULL, 0, ulFlags, 0);
	}
public:

	long	       m_lOutputType;

	int                m_nTrackId;
    int                m_fEof;

	int m_iFrameNumber;  // Current frame number that we are rendering.    
	BOOL m_bDiscontinuity; // If true, set the discontinuity flag.    
	ConnCtxT       *m_pVidConn;
};

/**
 * Implementation of TS output pin 
 */
class CAacStream :	public ITrackAssocInf
{
public:
    CAacStream(long *phr,  CMp4StrmSrc *pParent, ConnCtxT *pConn, const char *pszName);
    virtual ~CAacStream();

    // plots a ball into the supplied video frame
    long SendBuffer(CAvSample *pAvSample);
	int getTrackId(){return m_nTrackId;}
	void setTrackId(int nTrackId){m_nTrackId = nTrackId;}
	void setTrackEoF()
	{
		unsigned long ulFlags = OMX_BUFFERFLAG_EOS;
		if(m_pAudConn)
			m_pAudConn->Write(m_pAudConn, NULL, 0, ulFlags, 0);
	}
public:
	int            m_nTrackId;
    int            m_fEof;
	ConnCtxT       *m_pAudConn;
};
}

using namespace Mp4StrmSrc;
#ifdef WIN32
#define	 OSAL_WAIT(x) Sleep(x)
#else
#define	 OSAL_WAIT(x) usleep(x * 1000)
#endif

CMp4StrmSrc::CMp4StrmSrc() :
	m_pFileName(NULL)
{
	m_pFileReader = Mp4Demux::CMp4DemuxIf::CreateInstance();
	m_nVidWidth = DEFUALT_VID_WIDTH;
	m_nVidHeight = DEFUALT_VID_HEIGHT;
	m_iPins = 0;
	m_pAvSample = new CAvSample(1024 * 1024);
	m_llTtrackDurationMs = 0;
	m_llStartPtsMs = 0;
	m_nStartSystemTimeMs = 0;
	m_nStartStreamTimeMs = 0;
	m_fRateControl = 1;
	m_pCallback = NULL;
}

CMp4StrmSrc::~CMp4StrmSrc()
{
	if(m_pFileReader)
		delete m_pFileReader;

	if(m_pAvSample)
		delete m_pAvSample;
	for (int i=0; i < m_iPins; i++)
		delete m_paStreams[i];
	DeinitFileSrc();
}

long CMp4StrmSrc::CreateH264OutputPin()
{
	long hr;
	m_paStreams[m_iPins] = new CH264Stream(&hr, this, m_pVidConn, "H264 AnnexB Source!");
	m_iPins++;
	return hr;
}

long CMp4StrmSrc::CreateMP4AOutputPin()
{
	long hr = -1;
    m_paStreams[m_iPins] = new CAacStream(&hr, this, m_pAudConn, "AAC Source!");
	m_iPins++;
	return hr;
}


int CMp4StrmSrc::Load(const char *pszFileName)
{
	if (InitFileSrc(pszFileName)) {
		if(m_pFileReader) {
			int nStreams = m_pFileReader->GetNmumTracks();
			for (int i=0; i < nStreams; i++) {
				Mp4Demux::CTrackInf *pTrk = m_pFileReader->GetTrackInf(i);
				if(pTrk->m_CodecType == Mp4Demux::CODEC_TYPE_VIDEO) {
					m_nVidWidth = m_pFileReader->GetVidWidth(i);
					m_nVidHeight = m_pFileReader->GetVidHeight(i);
				} else 	if(pTrk->m_CodecType == Mp4Demux::CODEC_TYPE_AUDIO) {
					if(pTrk->m_CodecId == Mp4Demux::CODEC_ID_AAC){
						Mp4Demux::CAacCfgRecord *pAacRec = m_pFileReader->GetAacCfg(i);
						if(pAacRec){
							AacCfgRecord = *pAacRec;
						}
						m_AudSampleRate = m_pFileReader->GetAudSampleRate(i);
						m_AudNumChannels = m_pFileReader->GetAudNumChannels(i);
					}
				}
			}
		}
	}
    m_pFileName = strdup(pszFileName);
    return 0;
}

void CMp4StrmSrc::Stop()
{
	OnThreadDestroy();
}

void CMp4StrmSrc::Run(long long tStart)
{
	OnThreadStartPlay();
}



int CMp4StrmSrc::InitFileSrc(const char *pszFileName)
{
	if(m_pFileReader->OpenFileReader(pszFileName) == 0) {
		int nStreams = m_pFileReader->GetNmumTracks();
		for (int i=0; i < nStreams; i++) {
			Mp4Demux::CTrackInf *pTrk = m_pFileReader->GetTrackInf(i);
			if(pTrk->m_CodecId == Mp4Demux::CODEC_ID_H264) {
				CreateH264OutputPin();
				int nPin = m_iPins - 1;
				ITrackAssocInf *pStrm = dynamic_cast<ITrackAssocInf *>(m_paStreams[nPin]);
				pStrm->setTrackId(i);
				m_llTtrackDurationMs = pTrk ->m_llTtrackDuration * 1000 / pTrk->m_lTimescale;
			} else if (pTrk->m_CodecId == Mp4Demux::CODEC_ID_AAC) {
				CreateMP4AOutputPin();
				int nPin = m_iPins - 1;
				ITrackAssocInf *pStrm = dynamic_cast<ITrackAssocInf *>(m_paStreams[nPin]);
				pStrm->setTrackId(i);
			}
		}
	}
	return 0;
}

long CMp4StrmSrc::DeinitFileSrc()
{
	return 0;
}

long CMp4StrmSrc::OnThreadStartPlay(void)
{
	m_fRun = 1;
#ifdef WIN32
	DWORD dwThreadId;
	m_hThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)DoBufferProcessing, this, 0, &dwThreadId );
#else
	pthread_create(&m_hThread, NULL, thrdVideoStreaming, pStrmCtx);
#endif
    return 0;
}

long CMp4StrmSrc::OnThreadDestroy(void)
{
	void *ret_value;
	m_fRun = 0;
#ifdef WIN32
	WaitForSingleObject(m_hThread,10000);
#else
	pthread_join (m_hThread, (void **) &ret_value);
#endif
	return 0;
}


void *CMp4StrmSrc::DoBufferProcessing(void *pArg)
{
	CMp4StrmSrc *pFilter = (CMp4StrmSrc *)pArg;
	while(pFilter->m_fRun) {
		if(pFilter->DemuxStream() < 0){
			break;
		}
	}

	/* Set EoF on all the streams */
	for (int i=0; i < pFilter->m_iPins; i++) {
		ITrackAssocInf *pStrm = dynamic_cast<ITrackAssocInf *>(pFilter->m_paStreams[i]);
		pStrm->setTrackEoF();
	}

	if(pFilter->m_pCallback) {
		pFilter->m_pCallback(MP4SRC_EVENT_EOS, NULL);
	}
    return 0;
}

int CMp4StrmSrc::RateCtrlSyncDelivery(long long nStreamTimeMs)
{
	int nMaxWaitMs = 1000;
	int nCrtnSystemTime = GetTickCount();

	if(m_nStartSystemTimeMs == 0) {
		m_nStartSystemTimeMs = GetTickCount();
		m_nStartStreamTimeMs = nStreamTimeMs;
	}

	int nWaitTime = (nStreamTimeMs - m_nStartStreamTimeMs) - (nCrtnSystemTime - m_nStartSystemTimeMs);
	if(nWaitTime > 0) {
		if (nWaitTime > nMaxWaitMs)
			nWaitTime = nMaxWaitMs;
		OSAL_WAIT(nWaitTime);
	}
	return 0;
}

long CMp4StrmSrc::DemuxStream()
{
	unsigned long dwBytesRead = 0;
	long lQboxBytes = 0;
	long lResult = 0;
	// Find Stream Type
	long lStrmType = 0;
	long fEof = 0;
	CMp4StrmSrc *pFilter = this;
	int fDone = 0;
	CAvSample *pSample = m_pAvSample; // TODO: Allocate
	ITrackAssocInf *pCrntStrm = NULL;
	Mp4Demux::CPacket AvPkt;
	int nTrackId = m_pFileReader->GetNextPresentationTrackId();

	if(nTrackId < 0) {
		return -1;
	}

	// Get AvSample Queue for the codec
	for (int i=0; i < m_iPins; i++) {
		ITrackAssocInf *pStrm = dynamic_cast<ITrackAssocInf *>(m_paStreams[i]);
		if(pStrm->getTrackId() == nTrackId){
			pCrntStrm = pStrm;
			break;
		}
	}

	if((pCrntStrm ==  NULL) || !pCrntStrm->IsStreaming()){
		// Dicard data
		goto Skip;
	}

	if(pSample) {
		AvPkt.m_pData = (unsigned char *)pSample->m_pData;
		if(m_pFileReader->GetAvSample(nTrackId, &AvPkt) == 0) {
			pSample->m_lUsedLen = AvPkt.m_lSize;
			pSample->m_llPts = AvPkt.m_llPts / 10 + m_llStartPtsMs * 1000; // 100ns to microsec
			RateCtrlSyncDelivery(AvPkt.m_llPts / 10000); // 100ns to millisec
			pCrntStrm->SendBuffer(pSample);
		} else {
			pCrntStrm->setTrackEoF();
			pSample->m_fEoS = 1;
		}
	}
Skip:
	m_pFileReader->AdvanceSample(nTrackId);
	return lResult;
}

long CMp4StrmSrc::Seek(int TrackId, long long llTime)
{
	m_pFileReader->Seek(TrackId, llTime);
	return 0;
}

int CMp4StrmSrc::GetDuration()
{
	return m_llTtrackDurationMs;
}

void CMp4StrmSrc::SetStartPts(long long llPtsMs)
{
	m_llStartPtsMs = llPtsMs;
}

int CMp4StrmSrc::SetEventCallback(fnEventCallback_t pCallback)
{
	m_pCallback = pCallback;
	return 0;
}

CH264Stream::CH264Stream(long *phr, CMp4StrmSrc *pParent, ConnCtxT *pConn, const char *pPinName)
{
	m_fEof = 0;
	m_nTrackId = -1;
	m_pVidConn = pConn;
}


CH264Stream::~CH264Stream()
{
}

long CH264Stream::SendBuffer(CAvSample *pAvSample)
{
	unsigned long ulFlags = 0;
	if(pAvSample->m_fEoS) {
		ulFlags |= OMX_BUFFERFLAG_EOS;
	}

	m_pVidConn->Write(m_pVidConn, pAvSample->m_pData, pAvSample->m_lUsedLen, ulFlags, pAvSample->m_llPts);
	return 0;
}

CAacStream::CAacStream(long *phr,  CMp4StrmSrc *pParent, ConnCtxT *pConn, const char *pszName)
{
	m_fEof = 0;
	m_pAudConn = pConn;
}


CAacStream::~CAacStream()
{
}

long CAacStream::SendBuffer(CAvSample *pAvSample)
{
	unsigned long ulFlags = 0;
	if(pAvSample->m_fEoS) {
		ulFlags |= OMX_BUFFERFLAG_EOS;
	}
	if(m_pAudConn)
		m_pAudConn->Write(m_pAudConn, pAvSample->m_pData, pAvSample->m_lUsedLen, ulFlags, pAvSample->m_llPts);
	return 0;

}

CStrmSourceIf *CreateMp4Source()
{
	return new CMp4StrmSrc();
}

void DeleteMp4Source(CStrmSourceIf *_pSrc)
{
	CMp4StrmSrc *pSrc =  (CMp4StrmSrc *)_pSrc;
	delete pSrc;
}
