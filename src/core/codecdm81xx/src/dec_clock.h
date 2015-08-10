#ifndef __CLOCK_H__
#define __CLOCK_H__
#ifdef __cplusplus
extern "C" {
#endif
/**
 ** Supplies global clock (STC) for the application
 ** Uses the gettimeofdayto obtain the system clock.
 ** Can slave to an exteranl clock such as PCR
 ** Implementation is based on GStreamer gstclock.c
 ** Clock units are in micro seconds
 */
typedef unsigned long long CLOCK_T;

#define TIME_SECOND	   1000000		// micro seconds
#define TIME_MILLISEC  1000		    // micro seconds
typedef enum __CLOCK_STATE_T
{
	CLOCK_STOPPED,
	CLOCK_PAUSED,
	CLOCK_RUNNING
} CLOCK_STATE_T;

void *clkCreate(int fGlobal);
void clockDelete(void * pClk);
int ClockStart(void * pClk, CLOCK_T start_time);
int ClockStop(void * pClk);
int IsClockRunning(void * pClk);

CLOCK_STATE_T ClockGetState(void * pClk);

/**
 ** Returns the current decoder time adjusted with external clock.
 ** This time is used to control presentation.
 ** For audio master, this will is derived from audio PTS, for live streams this 
 ** is derived from PCR.
 ** If HW decoder clock is available, this is same as ClockGetInternalTime
 */
CLOCK_T ClockGetTime(void * pClk);

/**
 ** Adjusts the decoder clock based on the new external clock (PCR or audio PTS).
 */
void ClockAdjust(void * pClk, CLOCK_T slave, CLOCK_T master);

/**
 ** This is system clock. It is internally used by Clock module to get interanl clock.
 ** Application can use this for time stamping debug or event logs.
 */
CLOCK_T ClockGetSystemTime();

/**
 ** This is decoder clock. Currently this is same as ClockGetSystemTime.
 ** Ideally it should be derived from decoder/display sub-system.
 */
CLOCK_T ClockGetInternalTime(void * pClk);

// Clock Utilities
void Clock2HMSF( unsigned long long ullClockMHz, char *szTimeHMSF, int nBufSize);
void WaitForClock(void *pClk, CLOCK_T pts, int nTimeOut);

#ifdef __cplusplus
}
#endif

#endif

