#include <stdio.h>

#ifdef WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif
#include "strmcomp.h"
#include "dec_clock.h"
#include "dbglog.h"
#include "audio_mixer.h"
#include "g711_dec_chain.h"
#include "g711u.h"
#include "oal.h"
typedef void *(*thrdStartFcnPtr) (void *);
#define CODEC_TYPE_NONE		0
#define CODEC_TYPE_G711U	1
#define CODEC_TYPE_AAC    	2


typedef struct _G711_DEC_CHAIN_CTX
{
	int        fEoS;

	void       *readThrdId;
	int        m_fRun;
	void       *pClk;
	int        fClkSrc;

	int        buffer;
	int        codingType;
	ConnCtxT   *pConnSrc;
	ConnCtxT   *pConnDest;

	int        nFramesDecoded;
	int        nStatPrevFrames;
	CLOCK_T    StatPrevClk;
	int        nCrntFrameLen;
	CLOCK_T    nSyncWaitTime;
	CLOCK_T    nSyncRunningWaitTime;
	CLOCK_T    nSyncStartupWaitTime;

	OMX_BUFFERHEADERTYPE_M pcmBufferInt;
	OMX_BUFFERHEADERTYPE_M g711BufferInt;

	int  nOutputSampleRate;
	int  nNumOutputChannels;
	int  nPcmSamplesPerFrame;
	int  nG711DecFrameSize;

	int        nMixerPort;
	int        nMixFrameSize;
	void       *pCodecState;
} G711_DEC_CHAIN_CTX;


static int IsNewFrame(unsigned char *pData, int nLen)
{
	if(pData[0] == 0xFF && ((pData[1] & 0xF0) == 0xF0))
		return 1;
	else
		return 0;
}

static void  g711decchainProcess(G711_DEC_CHAIN_CTX * pCtx, OMX_BUFFERHEADERTYPE_M *pBuffer)
{
	int i;
	char szMsg[256];
	int nSpeechType;

	int nPcmByteOffset = 0;
	int nG711ByteOffset = 0;
	OMX_BUFFERHEADERTYPE_M *pPcmBuffer;
	OMX_BUFFERHEADERTYPE_M *pG711Buffer;
	int nPcmFrameSize = pCtx->nPcmSamplesPerFrame * pCtx->nNumOutputChannels * 2;
	//printf("PTS=%d \n", (int)pBuffer->nTimeStamp);
	// TODO: replace with clock
	CLOCK_T nTimeStamp = pBuffer->nTimeStamp;
	CLOCK_T  nStreamTime = ClockGetTime(pCtx->pClk);
	Clock2HMSF(nTimeStamp, szMsg, 255);


	if(pCtx->g711BufferInt.nFilledLen +  pBuffer->nFilledLen < pCtx->g711BufferInt.nAllocLen ) {
#ifdef WIN32
		//myWaveWrite(pBuffer->pBuffer, pBuffer->nFilledLen);
#endif

		memcpy(pCtx->g711BufferInt.pBuffer + pCtx->g711BufferInt.nFilledLen, pBuffer->pBuffer, pBuffer->nFilledLen);
		pCtx->g711BufferInt.nFilledLen +=  pBuffer->nFilledLen;
	} else {
		// TODO: HandleError
		int err=1;
	}
	pG711Buffer = &pCtx->g711BufferInt;

	// Decode in 80 byte chunks
	pPcmBuffer = &pCtx->pcmBufferInt;
	nG711ByteOffset = 0;
	if(pG711Buffer->nFilledLen >= pCtx->nG711DecFrameSize) {
		while(pG711Buffer->nFilledLen - nG711ByteOffset >= pCtx->nG711DecFrameSize){
			int res = g711uDecode(pCtx->pCodecState, (signed short *)(pG711Buffer->pBuffer + nG711ByteOffset), pCtx->nG711DecFrameSize, (signed short *)(pPcmBuffer->pBuffer + pPcmBuffer->nFilledLen));
			pPcmBuffer->nFilledLen += res;
			nG711ByteOffset += pCtx->nG711DecFrameSize;
		}
		if(pG711Buffer->nFilledLen - nG711ByteOffset > 0) {
			memcpy(pG711Buffer->pBuffer, pG711Buffer->pBuffer + nG711ByteOffset, pG711Buffer->nFilledLen - nG711ByteOffset);
		}
		pG711Buffer->nFilledLen = pG711Buffer->nFilledLen - nG711ByteOffset;
	}
	
	nPcmByteOffset = 0;

	if(pPcmBuffer->nFilledLen >= nPcmFrameSize){
		while(pPcmBuffer->nFilledLen - nPcmByteOffset >= nPcmFrameSize){
			if(pCtx->pConnDest) {
				while(pCtx->pConnDest->IsFull(pCtx->pConnDest) && pCtx->m_fRun) 
					OAL_TASK_SLEEP(1)
				if(!pCtx->m_fRun) goto Exit;

				pCtx->pConnDest->Write(pCtx->pConnDest, (char *)pPcmBuffer->pBuffer + nPcmByteOffset, nPcmFrameSize, 0, 0);
			} else {
				// Discard data
			}
			nPcmByteOffset += nPcmFrameSize;
		}
		if(pPcmBuffer->nFilledLen - nPcmByteOffset > 0){
			memcpy(pPcmBuffer->pBuffer, pPcmBuffer->pBuffer + nPcmByteOffset,pPcmBuffer->nFilledLen - nPcmByteOffset);
		}
		pPcmBuffer->nFilledLen =  pPcmBuffer->nFilledLen - nPcmByteOffset;
	} else {
		// Handle error
	}
	// Display Statistics
	{
		char szClck[256];
		char szMClckEnter[256];
		char szMClckLeave[256];
		char szPts[256];

		CLOCK_T pts = pBuffer->nTimeStamp;
		CLOCK_T clk = ClockGetInternalTime(pCtx->pClk);
		CLOCK_T mclk_leave = ClockGetTime(pCtx->pClk);
		CLOCK_T mclk_enter =  nStreamTime;
		if(clk - pCtx->StatPrevClk >= TIME_SECOND) {
			int buffOcupancy;
			Clock2HMSF(clk, szClck, 255);
			Clock2HMSF(pts, szPts, 255);
			Clock2HMSF(mclk_enter, szMClckEnter, 255);
			Clock2HMSF(mclk_leave, szMClckLeave, 255);
			buffOcupancy = pCtx->pConnSrc->BufferFullness(pCtx->pConnSrc);
			DBG_PRINT("<%s:ADec frame=%d rate=%0.2f crnt frame size=%d buff_full=%d pts=%s mclke=%s mclkl=%s>\n", szClck,pCtx->nFramesDecoded, 1.0 * (pCtx->nFramesDecoded - pCtx->nStatPrevFrames) / ((clk - pCtx->StatPrevClk) / TIME_SECOND), pCtx->nCrntFrameLen, buffOcupancy, szPts, szMClckEnter, szMClckLeave);
			pCtx->nStatPrevFrames = pCtx->nFramesDecoded;
			pCtx->StatPrevClk = clk;
		}
	}
	pCtx->nCrntFrameLen = pBuffer->nFilledLen;
	pCtx->nFramesDecoded++;

Exit:
	return;
}

#define MAX_READ_SIZE    (4 * 1024)
#define MAX_PCM_SIZE     (1024 * 2 * 2) // 1024 stereo 16 bit
#define MAX_PCM_INT_SIZE (MAX_PCM_SIZE * 6 * 2) // 3 1024 stereo 16 bit
#define MAX_G711_INT_SIZE (4 * 1024)

static void *threadg711decchainRead(void *threadsArg)
{
	void *res = NULL;
	G711_DEC_CHAIN_CTX *pCtx =  (G711_DEC_CHAIN_CTX *)threadsArg;
	OMX_BUFFERHEADERTYPE_M Buffer = {0};
	OMX_BUFFERHEADERTYPE_M *pBuffer = &Buffer;

	pCtx->pCodecState = g711uCreate(pCtx->nOutputSampleRate, pCtx->nNumOutputChannels == 2);
	pBuffer->pBuffer = (unsigned char *)malloc(MAX_READ_SIZE);
	pBuffer->nAllocLen = MAX_READ_SIZE;
	
	pCtx->pcmBufferInt.pBuffer = (unsigned char *)malloc(MAX_PCM_INT_SIZE);
	pCtx->pcmBufferInt.nAllocLen = MAX_PCM_INT_SIZE;
	pCtx->pcmBufferInt.nFilledLen = 0;

	pCtx->g711BufferInt.pBuffer = (unsigned char *)malloc(MAX_G711_INT_SIZE);
	pCtx->g711BufferInt.nAllocLen = MAX_G711_INT_SIZE;
	pCtx->g711BufferInt.nFilledLen = 0;

	while (pCtx->m_fRun)	{
		unsigned long ulFlags = 0;
		unsigned long long ullPts;

		while(pCtx->pConnSrc->IsEmpty(pCtx->pConnSrc) && pCtx->m_fRun){
					OAL_TASK_SLEEP(5)
		}
		if(!pCtx->m_fRun){
			goto Exit;
		}
		pBuffer->nFilledLen = pCtx->pConnSrc->Read(pCtx->pConnSrc, pBuffer->pBuffer, pBuffer->nAllocLen, &ulFlags, &ullPts);
		pBuffer->nTimeStamp = ullPts;
		pBuffer->nFlags = ulFlags;

		if(pBuffer->nFlags & OMX_BUFFERFLAG_EOS) {
			pCtx->fEoS = 1;
		}
		g711decchainProcess(pCtx, pBuffer);
	}
	free(pBuffer->pBuffer);
	free(pCtx->pcmBufferInt.pBuffer);
	free(pCtx->g711BufferInt.pBuffer);
	g711uDelete(pCtx->pCodecState);
Exit:
	return res;
}


void g711decchainDelete(StrmCompIf *pComp)
{
	G711_DEC_CHAIN_CTX *pCtx = (G711_DEC_CHAIN_CTX *)pComp->pCtx;
	// Do nothing for now
}

int g711decchainStart(StrmCompIf *pComp)
{
	AUDIO_MIXER_T *pAmix = amixGetInstance();
	G711_DEC_CHAIN_CTX *pCtx = (G711_DEC_CHAIN_CTX *)pComp->pCtx;
	pCtx->m_fRun = 1;
	//pCtx->pConnDest = amixGetInputPortConnCtx(pAmix, pCtx->nMixerPort);
	amixGetInputPortParam(pAmix, pCtx->nMixerPort, &pCtx->nOutputSampleRate, &pCtx->nPcmSamplesPerFrame, &pCtx->nNumOutputChannels);

	pCtx->nMixFrameSize =  pCtx->nPcmSamplesPerFrame * pCtx->nNumOutputChannels * 2;
	oalThreadCreate(&pCtx->readThrdId, threadg711decchainRead, pCtx);
	return 0;
}

int g711decchainStop(StrmCompIf *pComp)
{
	int ret_value;
	G711_DEC_CHAIN_CTX *pCtx = (G711_DEC_CHAIN_CTX *)pComp->pCtx;
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	if(pCtx->m_fRun) {
		pCtx->m_fRun = 0;
		oalThreadJoin(pCtx->readThrdId, 1000);
	}

	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return 0;
}

int g711decchainSetOption(
	struct _StrmCompIf *pComp, 
	int                nCmd, 
	char               *pOptionData)
{

	G711_DEC_CHAIN_CTX *pCtx = (G711_DEC_CHAIN_CTX *)pComp->pCtx;
	TRACE_PROGRESS
	if(nCmd == AUD_DEC_CMD_SET_PARAMS) {
		int coding;
		IL_AUD_ARGS *args = (IL_AUD_ARGS *)pOptionData;
		if(strcmp(args->codec_name, "g711u") == 0){
			pCtx->codingType = CODEC_TYPE_G711U;
		}
		pCtx->nMixerPort = args->nSessionId;
	}
	return 0;
}

int g711decchainSetInputConn(StrmCompIf *pComp, int nConnNum, ConnCtxT *pConn)
{
	G711_DEC_CHAIN_CTX *pAppData = (G711_DEC_CHAIN_CTX *)pComp->pCtx;
	pAppData->pConnSrc = pConn;
	return 0;
}

int g711decchainSetOutputConn(StrmCompIf *pComp, int nConnNum, ConnCtxT *pConn)
{
	G711_DEC_CHAIN_CTX *pAppData = (G711_DEC_CHAIN_CTX *)pComp->pCtx;
	pAppData->pConnDest = pConn;
	return 0;
}

int g711decchainSetClkSrc(StrmCompIf *pComp, void *pClk)
{
	G711_DEC_CHAIN_CTX *pAppData = (G711_DEC_CHAIN_CTX *)pComp->pCtx;
	pAppData->pClk = pClk;
	return 0;
}

void *g711decchainGetClkSrc(StrmCompIf *pComp)
{
	G711_DEC_CHAIN_CTX *pAppData = (G711_DEC_CHAIN_CTX *)pComp->pCtx;
	pAppData->fClkSrc= 1;
	pAppData->pClk = clkCreate(1);
	return pAppData->pClk;
}

static int g711decchainOpen(StrmCompIf *pComp, const char *pszResource)
{
	return 0;
}

StrmCompIf *
g711decchainCreate()
{
	G711_DEC_CHAIN_CTX *pAppData;
	StrmCompIf *pComp = (StrmCompIf *)malloc(sizeof(StrmCompIf));
	memset (pComp, 0x0, sizeof (StrmCompIf));
	pAppData = (G711_DEC_CHAIN_CTX *) malloc (sizeof (G711_DEC_CHAIN_CTX));
	memset (pAppData, 0x0, sizeof (G711_DEC_CHAIN_CTX));

	pAppData->nOutputSampleRate = 48000;
	pAppData->nNumOutputChannels = 2;
	pAppData->nPcmSamplesPerFrame = 1024;
	pAppData->nG711DecFrameSize = 80;

	pComp->pCtx = pAppData;

	pComp->Open= g711decchainOpen;
	pComp->SetOption = g711decchainSetOption;
	pComp->SetInputConn= g711decchainSetInputConn;
	pComp->SetOutputConn= g711decchainSetOutputConn;
	pComp->SetClkSrc = g711decchainSetClkSrc;
	pComp->GetClkSrc = g711decchainGetClkSrc;
	pComp->Start = g711decchainStart;
	pComp->Stop = g711decchainStop;
	pComp->Close = NULL;
	pComp->Delete = g711decchainDelete;
	return pComp;
}
