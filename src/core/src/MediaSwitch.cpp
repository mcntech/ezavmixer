#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef WIN32
//#include <Windows.h>
#include <winsock2.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#endif
#include "MediaSwitch.h"
#include "JdDbg.h"
#include "JdOsal.h"
#include <time.h>


#ifdef NO_DEBUG_LOG
#define DBG_PRINT(...)
#else
#define DBG_PRINT(...)   if (1) { printf(__VA_ARGS__); }
#endif

static int  modDbgLevel = CJdDbg::LVL_TRACE;
CMediaSwitch::CMediaSwitch(const char *pszName)
{
	m_pVidConnSrc = NULL;
	m_pAudConnSrc = NULL;
	PrevClk = 0;
	aud_frames = 0;
	vid_frames = 0;
	aud_pts =  0;
	vid_pts =  0;
	vid_start_pts = 0;
	aud_start_pts = 0;
	m_pSwitchInput = NULL;
	m_fVidInputChanged = 0;
	m_fAudInputChanged = 0;
	m_pszName = strdup(pszName);
	m_nVidStrmTime = 0;
	m_nAudStrmTime = 0;
}

CMediaSwitch::~CMediaSwitch()
{
	if(m_pszName)
		free(m_pszName);
}

int CMediaSwitch::AddOutput(CStrmOutBridge *pOut)
{
	m_Outputs.push_back(pOut);
	return 0;
}

int CMediaSwitch::DeleteOutput(CStrmOutBridge *pOut)
{
	for(StrmOutArray_T::iterator it = m_Outputs.begin(); it != m_Outputs.end(); ++it) {
		if(pOut == *it) {
			m_Outputs.erase(it);
			break;
		}
	}
	return 0;
}

void *CMediaSwitch::threadStreamingVideo(void *threadsArg)
{
	CMediaSwitch *pCtx =  (CMediaSwitch *)threadsArg;
	return pCtx->ProcessVideo();
}

void *CMediaSwitch::threadStreamingAudio(void *threadsArg)
{
	CMediaSwitch *pCtx =  (CMediaSwitch *)threadsArg;
	return pCtx->ProcessAudio();
}

void *CMediaSwitch::ProcessVideo()
{
	char *pData = (char *)malloc(MAX_VID_FRAME_SIZE);
	long long ullPts;
	unsigned long ulFlags = 0;
	
	JDBG_LOG(CJdDbg::LVL_SETUP,("Start"));

	if(m_pVidConnSrc == NULL){
		JDBG_LOG(CJdDbg::LVL_ERR,("!!! Video Connnection is not set !!!"));
	}
	while (m_fRun)	{
		int length = 0;
		while(m_pVidConnSrc->IsEmpty(m_pVidConnSrc) && m_fRun){
			JD_OAL_SLEEP(1)
			ShowStats();
		}
		if(!m_fRun)
			break;
		
		JDBG_LOG(CJdDbg::LVL_STRM,("Read m_pVidConnSrc"));
		ShowStats();
		length = m_pVidConnSrc->Read(m_pVidConnSrc, pData, MAX_VID_FRAME_SIZE, &ulFlags, &ullPts);
		if(length  < 0) {
				JDBG_LOG(CJdDbg::LVL_SETUP,("Exiting due to read error or connection close"));
				goto Exit;
		}		
		JDBG_LOG(CJdDbg::LVL_STRM,("Read length=%d %lld", length, ullPts));

		for(StrmOutArray_T::iterator it = m_Outputs.begin(); it != m_Outputs.end(); ++it) {
			CStrmOutBridge *pOut = *it;
			if(length >= 0) {
				JDBG_LOG(CJdDbg::LVL_STRM,("pOut=%p", pOut));
				if(ulFlags & OMX_EXT_BUFFERFLAG_DISCONT || m_fVidInputChanged) {
					pOut->SetVideoDiscont();
					m_fVidInputChanged = 0;
				} else if (ulFlags & OMX_BUFFERFLAG_EOS) {
					pOut->SetVideoEos();
				}
				if(length > 0) {
					pOut->SendVideo((unsigned char *)pData, length, ullPts * 90 / 1000);
					vid_frames++;
					vid_pts =  ullPts / 1000;
					if(vid_start_pts == 0)
						vid_start_pts = vid_pts;
					m_nVidStrmTime = (vid_pts - vid_start_pts) / 1000;
				}
			}
		}
	}
Exit:
	JDBG_LOG(CJdDbg::LVL_SETUP,("Exit"));
	return NULL;
}

void *CMediaSwitch::ProcessAudio()
{
	char *pData = (char *)malloc(MAX_AUD_FRAME_SIZE);
	long long ullPts;
	unsigned long ulFlags = 0;

	JDBG_LOG(CJdDbg::LVL_SETUP,("Start"));

	if(m_pAudConnSrc == NULL){
		JDBG_LOG(CJdDbg::LVL_ERR,("!!! Audio Connnection is not set !!!"));
	}

	while (m_fRun)	{
		int length = 0;
		while(m_pAudConnSrc->IsEmpty(m_pAudConnSrc) && m_fRun){
			JD_OAL_SLEEP(1)
			ShowStats();
		}
		if(!m_fRun)
			break;
		ShowStats();
		JDBG_LOG(CJdDbg::LVL_STRM,("Read m_pAudConnSrc"));
		
		length = m_pAudConnSrc->Read(m_pAudConnSrc, pData, MAX_AUD_FRAME_SIZE, &ulFlags, &ullPts);

		if(length  <= 0) {
			JDBG_LOG(CJdDbg::LVL_SETUP,("Exiting due to read error or connection close"));
			goto Exit;
		}		

		JDBG_LOG(CJdDbg::LVL_STRM,("Read length=%d pts=%lld", length, ullPts));
		//DumpHex((unsigned char *)pData, 16);
		for(StrmOutArray_T::iterator it = m_Outputs.begin(); it != m_Outputs.end(); ++it) {
			CStrmOutBridge *pOut = *it;
			if(length > 0) {
				JDBG_LOG(CJdDbg::LVL_STRM,("pOut=%p", pOut));
				if(ulFlags & OMX_EXT_BUFFERFLAG_DISCONT || m_fAudInputChanged) {
					pOut->SetAudioDiscont();
					m_fAudInputChanged = 0;
				}
				pOut->SendAudio((unsigned char *)pData, length, ullPts * 90 / 1000);
				aud_frames++;
				aud_pts =  ullPts / 1000;
				if(aud_start_pts == 0)
					aud_start_pts = aud_pts;
				m_nAudStrmTime = (aud_pts - aud_start_pts) / 1000;
			} 
		}
	}
Exit:
	JDBG_LOG(CJdDbg::LVL_SETUP,("Exit"));
	return NULL;
}

int CMediaSwitch::Run()
{
	m_fRun = 1;
	
	if(m_pVidConnSrc) {
		jdoalThreadCreate((void **)&m_thrdVidHandle, threadStreamingVideo, this);
	}
	if(m_pAudConnSrc){
		jdoalThreadCreate((void **)&m_thrdAudHandle, threadStreamingAudio, this);
	}
	return 0;
}

int CMediaSwitch::Stop()
{
	void *res;
	m_fRun =  0;

	if(m_pVidConnSrc) {
		jdoalThreadJoin((void *)m_thrdVidHandle, 3000);
	}
	if(m_pAudConnSrc) {
		jdoalThreadJoin((void *)m_thrdAudHandle, 3000);
	}
    return 0;
}

int CMediaSwitch::SetSource(ConnCtxT *pVidConnSrc, ConnCtxT *pAudConnSrc)
{
	if(m_pVidConnSrc) {
		m_fVidInputChanged = 1;
	}
	m_pVidConnSrc = pVidConnSrc;

	if(m_pAudConnSrc) {
		m_fAudInputChanged = 1;
	}
	m_pAudConnSrc = pAudConnSrc;
	return 0;
}

int CMediaSwitch::GetInputParams(int *pnWith, int *pnHeight, int *pnFrameRate, int *pnBandwidth)
{
	if(m_pSwitchInput) {
		*pnWith = m_pSwitchInput->nWidth;
		*pnHeight = m_pSwitchInput->nHeight;
		*pnFrameRate = 30;
		*pnBandwidth = 2000000;
		return 0;
	}
	return -1;
}

unsigned long ClockGet()
{
#ifdef WIN32
	return GetTickCount();
#else
	struct timeval   tv;
	gettimeofday(&tv,NULL);
	long long Timestamp =  ((long long)tv.tv_sec)*1000 + tv.tv_usec / 1000;
	return (Timestamp);
#endif
}

#define TIME_SECOND 1000
void CMediaSwitch::ShowStats()
{
	unsigned long clk = ClockGet();
	if(clk > PrevClk + TIME_SECOND) {
		DBG_PRINT("EncOut %s: vid=%ld aud=%ld NumOutputs=%d\n", m_pszName, vid_frames, aud_frames, m_Outputs.size());
		PrevClk = clk;
	}
}

void CMediaSwitch::SetDbgLevel(int nLevel)
{
	modDbgLevel = nLevel;
}
