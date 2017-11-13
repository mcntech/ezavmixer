#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <Windows.h>
typedef __int64 int64_t;
#else
#include <sys/time.h>
#include <unistd.h>
#endif

#include "strmclock.h"
#include "JdDbg.h"

static int modDbgLevel = CJdDbg::LVL_TRACE;

typedef void *(*thrdStartFcnPtr) (void *);

#define SINGLE_GOLOBAL_CLOCK

#define MAX_HISTORY_DEPTH 10

typedef struct _CloclCtx
{
	CLOCK_T internal_clock;
	CLOCK_T external_clock;
	CLOCK_T external_clock_at_stream_start;
	CLOCK_T internal_clock_at_stream_start;
	CLOCK_STATE_T clock_state;
	CLOCK_T prevUpdate;
	CLOCK_T internalClkHist[MAX_HISTORY_DEPTH];
	CLOCK_T externalClkHist[MAX_HISTORY_DEPTH];
	int     nIndxClkHist;
} ClockCtx;

ClockCtx gCtx = 
{
	0,
	0,
	0,
	0,
	CLOCK_STOPPED,
	0,
	{0},	// internalClkHist
	{0},
	0
};


static void threadClock(void *threadsArg)
{
}

void *clkCreate(int fGlobal)
{
	int i;
	ClockCtx *pClk = NULL;
	if(fGlobal){
		pClk = &gCtx;
	} else {
		pClk = (ClockCtx *)malloc(sizeof(ClockCtx));
		memset(pClk, 0x00, sizeof(ClockCtx));
	}
	pClk->clock_state = CLOCK_STOPPED;
	pClk->internal_clock = 0;
	pClk->external_clock = 0;
	pClk->external_clock_at_stream_start = 0;
	pClk->internal_clock_at_stream_start = 0;
	pClk->clock_state = CLOCK_STOPPED;
	pClk->prevUpdate = 0;
	
	for(i=0; i < MAX_HISTORY_DEPTH; i++) {
		pClk->internalClkHist[i] = 0;
		pClk->externalClkHist[i] = 0;
	}
	pClk->nIndxClkHist = 0;
	return pClk;
}

void clockDelete(void * pClk)
{
	ClockCtx *pCtx = (ClockCtx *)pClk;
	if(pClk)
		free(pClk);
}

int ClockStart(void * pClk, CLOCK_T start_time)
{
	ClockCtx *pCtx = (ClockCtx *)pClk;
	pCtx->internal_clock = pCtx->internal_clock_at_stream_start = ClockGetSystemTime();
	pCtx->external_clock_at_stream_start = pCtx->external_clock = start_time;
	pCtx->clock_state = CLOCK_RUNNING;
	if(modDbgLevel >= CJdDbg::LVL_TRACE) {
		char szMsg1[128];
		char szMsg2[128];
		Clock2HMSF(pCtx->internal_clock_at_stream_start, szMsg1, 127);
		Clock2HMSF(start_time, szMsg2, 127);
		JdDbg(CJdDbg::DBGLVL_TRACE, ("ClockStart: internal=%s external=%s\n", szMsg1,szMsg2));
	}
	return 0;
}

int ClockStop(void * pClk)
{
	ClockCtx *pCtx = (ClockCtx *)pClk;
	pCtx->clock_state = CLOCK_STOPPED;
	return 0;
}

int IsClockRunning(void * pClk)
{
	ClockCtx *pCtx = (ClockCtx *)pClk;
	return (pCtx->clock_state == CLOCK_RUNNING);
}

CLOCK_T ClockGetSystemTime()
{
#ifdef WIN32
	return (1000 * (int64_t)GetTickCount());
#else
	struct timeval   tv;
	gettimeofday(&tv,NULL);
	CLOCK_T Timestamp =  ((int64_t)tv.tv_sec)*1000000 + tv.tv_usec;
	return (Timestamp);
#endif
}

/*
** ClockGetInternalTime obtains the STC of AV Playback subsystem.
** Use linux sytem time for now.
*/

CLOCK_T ClockGetInternalTime(void * pClk)
{
	ClockCtx *pCtx = (ClockCtx *)pClk;
	CLOCK_T clk =  ClockGetSystemTime();
	return (clk);
}


/*
** ClockGetTime obtains the STC of AV Playback subsystem.
** Use linux sytem clock modified with external clock for now.
*/
CLOCK_T ClockGetTime(void * pClk)
{
	if(pClk) {
		ClockCtx *pCtx = (ClockCtx *)pClk;
		CLOCK_T system_time_diff =  ClockGetInternalTime(pClk) - pCtx->internal_clock;
		// TODO: Scale
		return (pCtx->external_clock + system_time_diff);
	} else {
		return ClockGetSystemTime();
	}
}

#define DECR_INDEX(x, max_indx) (x + max_indx - 1) % max_indx

/*
** ClockScale computes the ratio between extranl clock and internal clock
** by averaging over MAX_HISTORY_DEPTH intervals.
*/
double ClockScale(void * pClk)
{
	ClockCtx *pCtx = (ClockCtx *)pClk;
	double scale = 1.0, diffInternal = 0, diffExternal = 0;
	int i;
	
	int nIndx = DECR_INDEX(pCtx->nIndxClkHist, MAX_HISTORY_DEPTH);
	for(i=0; i < MAX_HISTORY_DEPTH - 1; i++){
		int nIndxPrev = DECR_INDEX(nIndx, MAX_HISTORY_DEPTH);
		if(pCtx->internalClkHist[nIndxPrev] > 0) {
			diffInternal += pCtx->internalClkHist[nIndx] - pCtx->internalClkHist[nIndxPrev];
			diffExternal += pCtx->externalClkHist[nIndx] - pCtx->externalClkHist[nIndxPrev];
		} else {
			break;
		}
		nIndx = nIndxPrev;
	}
	if(i > 0 && diffInternal > 0) {
		scale = diffExternal / diffInternal;
	}

	return scale;
}

/*
** ClockAdjust updates external clock.
*/
void ClockAdjust(void * pClk, CLOCK_T slave, CLOCK_T master)
{
	ClockCtx *pCtx = (ClockCtx *)pClk;
	// TODO: Use table for clock jitter

	pCtx->external_clock = master;
	pCtx->internal_clock = slave;
	pCtx->internalClkHist[pCtx->nIndxClkHist] = slave;
	pCtx->externalClkHist[pCtx->nIndxClkHist] = master;
	pCtx->nIndxClkHist = (pCtx->nIndxClkHist + 1) % MAX_HISTORY_DEPTH;
	if(modDbgLevel >= CJdDbg::LVL_TRACE) {
		if(pCtx->prevUpdate + TIME_SECOND <= pCtx->internal_clock) {
			char szMsg1[256];
			char szMsg2[256];
			char szMsg3[256];

			double scale = ClockScale(pClk);
			Clock2HMSF(pCtx->internal_clock, szMsg1, 255);
			Clock2HMSF(pCtx->external_clock, szMsg2, 255);
			Clock2HMSF(pCtx->external_clock - pCtx->external_clock_at_stream_start, szMsg3, 255);
		
			JdDbg(CJdDbg::LVL_TRACE, ("<%s:ClockAdjust: external=%s strm_clk%s  clock_scale=%f\n", szMsg1, szMsg2, szMsg3,  scale));
			pCtx->prevUpdate = pCtx->internal_clock;
		}
	}
}


void Clock2HMSF( unsigned long long ullClockMHz, char *szTimeHMSF, int nBufSize)
{
	#define  TUNIT_60 60
	long long t_msec;
	int total_time, hours,  minutes, seconds, msec;
	total_time = (int)(ullClockMHz / TIME_SECOND);
	seconds = total_time % TUNIT_60;
	total_time = total_time - seconds;
	minutes = (total_time % (TUNIT_60 * TUNIT_60)) / TUNIT_60;
	total_time = total_time - (minutes * TUNIT_60);
	hours = total_time / (TUNIT_60 * TUNIT_60);

	t_msec = ullClockMHz / TIME_MILLISEC;
	msec = t_msec % 1000;
#ifdef WIN32
	sprintf_s(szTimeHMSF, nBufSize, "%02i:%02i:%02i.%03i",  hours, minutes, seconds, msec);
#else
	snprintf(szTimeHMSF, nBufSize, "%02i:%02i:%02i.%03i",  hours, minutes, seconds, msec);
#endif
}

CLOCK_STATE_T ClockGetState(void * pClk)
{
	ClockCtx *pCtx = (ClockCtx *)pClk;
	return pCtx->clock_state;
}

void WaitForClock(void * pClk, CLOCK_T pts, int nTimeOut)
{
	ClockCtx *pCtx = (ClockCtx *)pClk;
	while(nTimeOut > 0){
		CLOCK_T clock = ClockGetTime(pClk);
		if (pts <=  clock)
			break;
#ifdef WIN32
		Sleep(2);
#else
		usleep(2 * TIME_MILLISEC);
#endif
		nTimeOut -= 2 * TIME_MILLISEC;
	}
}
