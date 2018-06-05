#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif
#include <sys/types.h>
#include <fcntl.h>
#include "filesrc.h"
#include "JdDbg.h"
#include "JdOsal.h"

typedef void *(*thrdStartFcnPtr) (void *);
static int modDbgLevel = CJdDbg::LVL_TRACE;


typedef struct
{
	int        fpIn;
	ConnCtxT   *pConn;
	int        nBlockSize;
#ifdef WIN32
	HANDLE     readThrdId;
#else
	pthread_t  readThrdId;
#endif
	int    pipeEmptyBuff;
	int    pipeFillBuff;
	int        fStreaming;
	int        nUiCmd;
	long long  llTotolRead;
	int        fLoop;
	unsigned long long rate_control_prev_ts;
} FileSrcCtx;


unsigned long long ClockGetSystemTime()
{
#ifdef WIN32
	return (1000 * (int64_t)GetTickCount());
#else
	struct timeval   tv;
	gettimeofday(&tv,NULL);
	unsigned long long Timestamp =  ((int64_t)tv.tv_sec)*1000000 + tv.tv_usec;
	return (Timestamp);
#endif
}

void RateControl(FileSrcCtx  *self, int crntSize)
{
	double time_elapsed;
	double time_estimate;
	double ts_rate = 0;
	int64_t clockDiff;
	unsigned long long current_ts  = ClockGetSystemTime();

	if(self->rate_control_prev_ts == 0 ) {
		self->rate_control_prev_ts = current_ts;
		return;
	}
	//ts_rate = self->current_tsrate;
	if(ts_rate == 0)
		ts_rate = 40000000.0 / 8;

	time_elapsed = (double) (current_ts - self->rate_control_prev_ts) / 1000000;
	time_estimate = (double) crntSize / ts_rate;

	if (time_estimate > time_elapsed){
		clockDiff = (int64_t) ((time_estimate - time_elapsed) * 1000 * 1000);
		//usleep(clockDiff);
	}
	self->rate_control_prev_ts = current_ts;
}

static void threadFileSrcRead(void *threadsArg)
{
	char *pData;
	unsigned long ulFlags = 0;
	int ret = 0;

	FileSrcCtx *pCtx =  (FileSrcCtx *)threadsArg;
	JdDbg(CJdDbg::DBGLVL_TRACE, ("Enter"));

	pData = (char *)malloc(DMA_READ_SIZE);

	while (1)	{
		ret = 0;
		// Wait for buffer availability
		while(pCtx->pConn->IsFull(pCtx->pConn) && pCtx->nUiCmd != STRM_CMD_STOP){
			//JdDbg(CJdDbg::DBGLVL_WARN,("start=%d end=%d",pCtx->pConn->pdpCtx->start, pCtx->pConn->pdpCtx->end))
			JD_OAL_SLEEP(1)
		}

		if(pCtx->nUiCmd == STRM_CMD_STOP) {
			JdDbg(CJdDbg::DBGLVL_SETUP, ("Setting EoS due to User Command."));
			pCtx->fStreaming = 0;
			ulFlags = OMX_BUFFERFLAG_EOS;
		} else {
			ret = read (pCtx->fpIn, pData, DMA_READ_SIZE);
			if(ret <= 0) {
				if(pCtx->fLoop) {
					// Try again
					lseek(pCtx->fpIn, SEEK_SET, 0);
					ret = read (pCtx->fpIn, pData, DMA_READ_SIZE);					
				}
				if(ret <= 0){
					JdDbg(CJdDbg::DBGLVL_SETUP, ("Setting EoS due to source EoS."));
					pCtx->fStreaming = 0;
					ulFlags = OMX_BUFFERFLAG_EOS;
				}
			}
			pCtx->llTotolRead += ret;
		}
		
		pCtx->pConn->Write(pCtx->pConn, pData, ret, ulFlags, 0);

		if(ulFlags & OMX_BUFFERFLAG_EOS || pCtx->nUiCmd == STRM_CMD_STOP){
			JDBG_LOG(CJdDbg::LVL_SETUP, ("Exiting. nFlags=0x%x, nUiCmd=0x%x",ulFlags, pCtx->nUiCmd));
			pCtx->fStreaming = 0;
			break;
		}

		RateControl(pCtx, ret);
	}
	free(pData);
	JdDbg(CJdDbg::DBGLVL_TRACE, ("Leave"));
}

int FileSrcInit(FileSrcCtx *pCtx)
{
	pCtx->fpIn = 0;
	pCtx->pipeEmptyBuff = 0;
	pCtx->pipeFillBuff = 0;
	pCtx->nBlockSize = (21 * 188);
	pCtx->fLoop = 1;
	return 0;
}

int FileSrcSetInputFileHandle(FileSrcCtx *pCtx, int fpIn)
{
	pCtx->fpIn = fpIn;
	return 0;
}

int filesrcOpen(StrmCompIf *pComp, const char *szFile)
{
	FileSrcCtx *pCtx = (FileSrcCtx *)pComp->pCtx;
#ifdef WIN32
	int fIn = open (szFile, O_RDONLY|O_BINARY, 0);
#else
	int fIn = open (szFile, O_RDONLY, 0);
#endif
	if (fIn < 0)	{
		printf ("Error: failed to open the file %s for reading\n",  szFile);
		return -1;
	}
	pCtx->fpIn = fIn;
	return 0;
}

int filesrcSetOption(StrmCompIf *pComp, int nCmd, char *pOptionData)
{
	FileSrcCtx *pCtx = (FileSrcCtx *)pComp->pCtx;
	return 0;
}
int filesrcSetInputConn(struct _StrmCompIf *pComp, int nConnNum, ConnCtxT *pConn)
{
	return 0;
}

int filesrcSetOutputConn(StrmCompIf *pComp, int nCOnnNum, ConnCtxT *pConn)
{
	FileSrcCtx *pCtx = (FileSrcCtx *)pComp->pCtx;
	pCtx->pConn = pConn;
	return 0;
}

void filesrcClose(StrmCompIf *pComp)
{
	FileSrcCtx *pCtx = (FileSrcCtx *)pComp->pCtx;
	if (pCtx->fpIn != NULL)  {
		close (pCtx->fpIn);
		pCtx->fpIn = NULL;
	}
}

int filesrcStart(StrmCompIf *pComp)
{
	int res = 0;
	FileSrcCtx *pCtx = (FileSrcCtx *)pComp->pCtx;
	pCtx->fStreaming = 1;

	if(pCtx->pConn == NULL || pCtx->fpIn == 0) {
		JdDbg(CJdDbg::DBGLVL_ERROR, ("Error starting file src !"));
		res = -1;
		goto Exit;
	}
#ifdef WIN32
	{
		DWORD dwThreadId;
		pCtx->readThrdId = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadFileSrcRead, pCtx, 0, &dwThreadId);
	}
#else
	if (pthread_create (&pCtx->readThrdId, NULL, (thrdStartFcnPtr) threadFileSrcRead, pCtx) != 0)	{
		JdDbg(CJdDbg::DBGLVL_ERROR, ("Create_Task failed !"));
	} else {
		pthread_setname_np(pCtx->readThrdId, "filesrc");
	}
#endif
Exit:
	return res;
}

int filesrcStop(StrmCompIf *pComp)
{
	FileSrcCtx *pCtx = (FileSrcCtx *)pComp->pCtx;
	void *ret_value;
	int nTimeOut = 1000000; // 1000 milli sec
	// Attempt closing the tthread
	JdDbg(CJdDbg::DBGLVL_TRACE, ("Enter fStreaming=%d", pCtx->fStreaming));

	if(pCtx->fStreaming) {
		pCtx->nUiCmd = STRM_CMD_STOP;
		JdDbg(CJdDbg::DBGLVL_TRACE, ("fStreaming=%d", pCtx->fStreaming));
		while(pCtx->fStreaming && nTimeOut > 0) {
			nTimeOut -= 1000;
#ifdef WIN32
			Sleep(1);
#else
			usleep(1000);
#endif
		}
		// If thread did not exit, force close it
		JdDbg(CJdDbg::DBGLVL_TRACE, ("fStreaming=%d nTimeOut rem=%d", pCtx->fStreaming, nTimeOut));

	}

	if(pCtx->fStreaming) {
		// Close the file to force EoS
		JdDbg(CJdDbg::DBGLVL_TRACE, ("Force EoS by closing source"));
		if (pCtx->fpIn != NULL)  {
			close (pCtx->fpIn);
			pCtx->fpIn = NULL;
		}

		nTimeOut = 1000000;
		while(pCtx->fStreaming && nTimeOut > 0) {
			nTimeOut -= 1000;
#ifdef WIN32
			Sleep(1);
#else
			usleep(1000);
#endif
		}
	}

	if(pCtx->fStreaming) {
		JdDbg(CJdDbg::DBGLVL_SETUP, ("[Thread did nit exit. Cancelling the thread..."));
#ifdef WIN32
#else
		//pthread_cancel(pCtx->readThrdId);
		pthread_join (pCtx->readThrdId, (void **) &ret_value);
#endif
		JdDbg(CJdDbg::DBGLVL_SETUP, ("Cancelling the thread: Done]"));
	}

	JdDbg(CJdDbg::DBGLVL_TRACE, ("Leave"));
	return 0;
}

int filesrcSetClkSrc(struct _StrmCompIf *pComp, void *pClk)
{
	return 0;
}
void *filesrcGetClkSrc(struct _StrmCompIf *pComp)
{
	return NULL;
}

void filesrcDelete(struct _StrmCompIf *pComp)
{
	if(pComp->pCtx) {
		free(pComp->pCtx);
	}
	free(pComp);
	//return 0;
}

StrmCompIf *
filesrcCreate()
{
	StrmCompIf *pComp = (StrmCompIf *)malloc(sizeof(StrmCompIf));
	pComp->pCtx = malloc(sizeof(FileSrcCtx));
	memset(pComp->pCtx, 0x00, sizeof(FileSrcCtx));
	
	FileSrcInit((FileSrcCtx *)pComp->pCtx);

	pComp->Open= filesrcOpen;
	pComp->SetOption = filesrcSetOption;
	pComp->SetInputConn= filesrcSetInputConn;
	pComp->SetOutputConn= filesrcSetOutputConn;
	pComp->SetClkSrc = filesrcSetClkSrc;
	pComp->GetClkSrc = filesrcGetClkSrc;
	pComp->Start = filesrcStart;
	pComp->Stop = filesrcStop;
	pComp->Close = filesrcClose;
	pComp->Delete = filesrcDelete;
	return pComp;
}
