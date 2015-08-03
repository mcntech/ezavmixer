#ifndef __ENC_OUT_H__
#define __ENC_OUT_H__
#ifdef WIN32
//#include <Windows.h>
#include <winsock2.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#endif
#include <assert.h>
#include <vector>
#include "StrmOutBridgeBase.h"
#include "OmxIf.h"

typedef std::vector<CStrmOutBridge *> StrmOutArray_T;
class CMediaSwitch
{
public:
	CMediaSwitch(const char *pszName);
	~CMediaSwitch();
	static void *threadStreamingVideo(void *threadsArg);
	static void *threadStreamingAudio(void *threadsArg);
	void *ProcessVideo();
	void *ProcessAudio();
	int Run();
	int Stop();
	int SetSource(ConnCtxT *pVidConnSrc, ConnCtxT *pAudConnSrc);
	int AddOutput(CStrmOutBridge *pOut);
	int GetInputParams(int *pnWith, int *pnHeight, int *pnFrameRate, int *pnBandwidth);
	int DeleteOutput(CStrmOutBridge *pOut);
	void ShowStats();
	static void SetDbgLevel(int nLevel);

public:
	//int  nSessionId;
	int nCmd;
	int nPort;

	StrmOutArray_T        m_Outputs;

	int            m_fRun;
	ConnCtxT       *m_pVidConnSrc;
	ConnCtxT       *m_pAudConnSrc;

#ifdef WIN32
	HANDLE         m_thrdVidHandle;
#else
	pthread_t      m_thrdVidHandle;
#endif
#ifdef WIN32
	HANDLE         m_thrdAudHandle;
#else
	pthread_t      m_thrdAudHandle;
#endif

	int        fEoS;
	unsigned long  PrevClk;
	long       aud_frames;
	long       vid_frames;
	long       aud_pts;
	long       vid_pts;
	long       vid_start_pts;
	long       aud_start_pts;
	int        m_fVidInputChanged;
	int        m_fAudInputChanged;
	char       *m_pszName;
	// Statistics
	int        m_nVidStrmTime;
	int        m_nAudStrmTime;
	CAvmixInputStrm *m_pSwitchInput;
};
#endif // __ENC_OUT_H__