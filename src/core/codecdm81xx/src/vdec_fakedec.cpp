#include <stdio.h>

#ifdef WIN32
#include <Windows.h>
#else
#include <sys/ioctl.h>
#include <pthread.h>
#endif
#include "strmcomp.h"
#include "dec_clock.h"
#include "vdec_fakedec.h"
#include "vdec_es_parser.h"

#include "dbglog.h"
#include "h264parser.h"

#define  VIDEO_CodingAVC	1
#define  VIDEO_CodingMPEG2	2

typedef void *(*thrdStartFcnPtr) (void *);

typedef struct
{
	int nInstanceId;
	int        fEoS;
#ifdef WIN32
	HANDLE     readThrdId;
#else
	pthread_t  readThrdId;
#endif
	int        fRun;
	void       *pClk;
	int        buffer;
	int        codingType;
	ConnCtxT   *pConnSrc;
	int        nUiCmd;
	int        nFramesDecoded;
	int        nStatPrevFrames;
	CLOCK_T    StatPrevClk;
	int        nCrntFrameLen;
	
	CLOCK_T    nJitterLatency;
	CLOCK_T    nSyncMaxWait;
	CLOCK_T    nSyncMaxWaitRunning;
	CLOCK_T    nSyncMaxWaitStartup;
	CLOCK_T    nSyncMaxLateness;
	H264_ParsingCtx H264Parser;
} FakeDecCtx;

static int IsNewFrame(int coding, char *pData, int nLen)
{
	int idx=0;
	int nal_unit_type;
	/*
	**  B.1.1 Byte stream NAL unit syntax
	**	while( next_bits( 24 ) != 0x000001 &&
	**		next_bits( 32 ) != 0x00000001 )
	*/
	if (coding == VIDEO_CodingAVC) {
		if(pData[0] == 0x00 && pData[1] == 0x00) {
			if(pData[2] == 0x01) {
				idx = 4;
			} else if(pData[2] == 0x00 && pData[3] == 0x01) {
				idx = 5;
			}
		
			if(idx) {
				nal_unit_type = pData[idx] && 0x1F;
				if ((nal_unit_type >=1 && nal_unit_type <= 5) || nal_unit_type == 9)
					return 1;
			}
		}
	}
	return 0;
}

static void DoSync(FakeDecCtx *pCtx, CLOCK_T pts, int *fDrop, CLOCK_T nMaxWait, CLOCK_T nMaxLateness)
{
	if(pCtx->pClk) {
		CLOCK_T  clock = ClockGetTime(pCtx->pClk);
		*fDrop = 0;
		if(pts > clock) {
			CLOCK_T wait_time = pts - clock;
			if(wait_time > nMaxWait)
				wait_time = nMaxWait;
			if(wait_time){
				WaitForClock(pCtx->pClk, pts, wait_time);
			}
		} else 	if(pts < clock - nMaxLateness ) {
			*fDrop = 1;
		}
#if 1
		if(*fDrop)	{
			char szMsg1[128] = {0};
			char szMsg2[128] = {0};

			Clock2HMSF(clock, szMsg1, 127);
			Clock2HMSF(pts, szMsg2, 127);
			DBG_LOG(DBGLVL_STAT, ("Drop clk=%s(%lld) pts=%s(%lld)\n", szMsg1, clock, szMsg2, pts));
		}
#endif
	} else {
		Sleep(30);
	}
}
static void  FakeDecConsume(FakeDecCtx *pCtx, OMX_BUFFERHEADERTYPE_M *pBuffer)
{
#define MAX_WAIT_TIME     30000		// 30 milli sec
	char szMsg[256];
	int fDrop;
	//printf("PTS=%d \n", (int)pBuffer->nTimeStamp);
	// TODO: replace with clock
	CLOCK_T nTimeStamp = pBuffer->nTimeStamp;
	CLOCK_T  nStreamTime = ClockGetTime(pCtx->pClk);
	Clock2HMSF(nTimeStamp, szMsg, 255);
	//printf("PTS=%s \n", szMsg);
	if(!IsNewFrame(pCtx->codingType, (char *)pBuffer->pBuffer, pBuffer->nFilledLen)) {
		pCtx->nCrntFrameLen += pBuffer->nFilledLen;
		return;
	}
	if(pCtx->nFramesDecoded) {
		pCtx->nSyncMaxWait = pCtx->nSyncMaxWaitRunning;
	} else {
		pCtx->nSyncMaxWait = pCtx->nSyncMaxWaitStartup;
	}

	DoSync(pCtx, nTimeStamp, &fDrop, pCtx->nSyncMaxWait, pCtx->nSyncMaxLateness);

	// Display Statistics
	{
		char szClck[256];
		char szMClckEnter[256];
		char szMClckLeave[256];
		char szPts[256];
		CLOCK_T mclk_leave = ClockGetTime(pCtx->pClk);
		CLOCK_T mclk_enter =  nStreamTime;

		CLOCK_T pts = pBuffer->nTimeStamp;
		CLOCK_T clk = ClockGetInternalTime(pCtx->pClk);
		if(clk - pCtx->StatPrevClk >= TIME_SECOND) {
			int buffOcupancy;
			Clock2HMSF(clk, szClck, 255);
			Clock2HMSF(pts, szPts, 255);
			Clock2HMSF(mclk_enter, szMClckEnter, 255);
			Clock2HMSF(mclk_leave, szMClckLeave, 255);
			buffOcupancy = pCtx->pConnSrc->BufferFullness(pCtx->pConnSrc);
			printf("<%s:Vid:Strm=%d  frame=%d rate=%0.2f crnt frame size=%d buff_full=%d pts=%s mclke=%s mclkl=%s>\n", szClck, pCtx->nInstanceId, pCtx->nFramesDecoded, 1.0 * (pCtx->nFramesDecoded - pCtx->nStatPrevFrames) / ((clk - pCtx->StatPrevClk) / TIME_SECOND), pCtx->nCrntFrameLen, buffOcupancy, szPts, szMClckEnter, szMClckLeave);
			pCtx->nStatPrevFrames = pCtx->nFramesDecoded;
			pCtx->StatPrevClk = clk;
		}
	}
	pCtx->nFramesDecoded++;
	// Reset frame len
	pCtx->nCrntFrameLen = pBuffer->nFilledLen;
}

#define MAX_READ_SIZE (1024 * 1024)
static void threadFakeDecRead(void *threadsArg)
{
	FakeDecCtx *pAppData =  (FakeDecCtx *)threadsArg;
	int frameSize = 0;
	OMX_BUFFERHEADERTYPE_M Buffer = {0};
	OMX_BUFFERHEADERTYPE_M *pBuffer = &Buffer;
	pBuffer->pBuffer = (unsigned char *)malloc(MAX_READ_SIZE);
	pBuffer->nAllocLen = MAX_READ_SIZE;
	H264::cParser *pParser= new H264::cParser;

	Decode_H264ParserInit2(&pAppData->H264Parser, pAppData->pConnSrc);
	
	while (pAppData->fRun)	{
		unsigned long ulFlags;
		unsigned long long ullPts;

		pAppData->H264Parser.outBuf.ptr = pBuffer->pBuffer;
		pAppData->H264Parser.outBuf.bufsize = pBuffer->nAllocLen;
		pAppData->H264Parser.outBuf.bufused = 0;
		pAppData->H264Parser.outBuf.nFlags = 0;
		frameSize = Decode_GetNextH264FrameSize (&pAppData->H264Parser);

		pBuffer->nTimeStamp = pAppData->H264Parser.outBuf.nTimeStamp;
		pBuffer->nFlags = pAppData->H264Parser.outBuf.nFlags;
		if(pBuffer->nFlags & OMX_BUFFERFLAG_EOS) {
			pAppData->fEoS = 1;
		}
		{
			int i = 0;
			unsigned char *pData = pBuffer->pBuffer;
			while(i < frameSize - 4) {
				if(pData[i] == 0x00 && pData[i+1] == 0x00 &&  pData[i+2] == 0x01) {
					pParser->PutNalUnitData(pBuffer->pBuffer + i, frameSize);
					pParser->Process();
				}
				i++;
			}
		}
		FakeDecConsume(pAppData, pBuffer);
		if(pAppData->fEoS) {
			pAppData->fRun = 0;
			printf("Exiting Fake Decode thread \n");
		}
	}
	delete pParser;
	free(pBuffer->pBuffer);
}


void fakedecDelete(StrmCompIf *pComp)
{
	FakeDecCtx *pCtx = (FakeDecCtx *)pComp->pCtx;
	// Do nothing for now
}

int fakedecStart(StrmCompIf *pComp)
{
	FakeDecCtx *pCtx = (FakeDecCtx *)pComp->pCtx;
	pCtx->fRun = 1;
#ifdef WIN32
	{
		DWORD dwThreadId;
		pCtx->readThrdId = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)&threadFakeDecRead,(LPVOID) pCtx,0,&dwThreadId);
	}
#else
	if (pthread_create (&pCtx->readThrdId, NULL, (thrdStartFcnPtr *) threadFakeDecRead, pCtx) != 0)	{
		DBG_LOG(DBGLVL_ERROR, ("Create_Task failed !"));
	}
#endif
	return 0;
}

int fakedecStop(StrmCompIf *pComp)
{
	int ret_value;
	FakeDecCtx *pCtx = (FakeDecCtx *)pComp->pCtx;
	if(pCtx->fRun) {
		pCtx->fRun = 0;
		//pthread_cancel(pCtx->readThrdId);
#ifdef WIN32
		WaitForSingleObject(pCtx->readThrdId, 3000);
#else
		pthread_join (pCtx->readThrdId, (void **) &ret_value);
#endif
	}
	return 0;
}

int fakedecSetOption(
	struct _StrmCompIf *pComp, 
	int                nCmd, 
	char               *pOptionData)
{

	FakeDecCtx *pCtx = (FakeDecCtx *)pComp->pCtx;
	TRACE_PROGRESS
	if(nCmd == DEC_CMD_SET_PARAMS) {
		int coding;
		IL_VID_ARGS *args = (IL_VID_ARGS *)pOptionData;
		pCtx->nInstanceId = args->nInstanceId;
		pCtx->buffer = args->buffer;
		if(!strcmp(args->codec_name,"h264")){
			coding = VIDEO_CodingAVC;
		} else if(!strcmp(args->codec_name,"mpeg2")) {
			coding = VIDEO_CodingMPEG2;
		}
		pCtx->codingType = coding; 
	}
	return 0;
}
int fakedecSetInputConn(StrmCompIf *pComp, int nConnNum, ConnCtxT *pConn)
{
	FakeDecCtx *pAppData = (FakeDecCtx *)pComp->pCtx;
	pAppData->pConnSrc = pConn;
	return 0;
}

int fakedecSetClkSrc(StrmCompIf *pComp, void *pClk)
{
	FakeDecCtx *pAppData = (FakeDecCtx *)pComp->pCtx;
	pAppData->pClk = pClk;
	return 0;
}

int vidchainOpen(StrmCompIf *pComp, const char *szResource)
{
	return 0;
}
StrmCompIf *
fakedecCreate()
{
	FakeDecCtx *pAppData;
	StrmCompIf *pComp = (StrmCompIf *)malloc(sizeof(StrmCompIf));
	memset (pComp, 0x0, sizeof (StrmCompIf));
	pAppData = (FakeDecCtx *) malloc (sizeof (FakeDecCtx));
	memset (pAppData, 0x0, sizeof (FakeDecCtx));

	pAppData->nSyncMaxWaitRunning = 30000;  // 30ms
	pAppData->nSyncMaxWaitStartup = 500000; // 500ms
	pAppData->nSyncMaxLateness = 200000;    // 200ms

	pComp->pCtx = pAppData;

	pComp->Open= vidchainOpen;
	pComp->SetOption = fakedecSetOption;
	pComp->SetInputConn= fakedecSetInputConn;
	pComp->SetOutputConn= NULL;
	pComp->SetClkSrc = fakedecSetClkSrc;
	pComp->GetClkSrc = NULL;
	pComp->Start = fakedecStart;
	pComp->Stop = fakedecStop;
	pComp->Close = NULL;
	pComp->Delete = fakedecDelete;
	return pComp;
}
