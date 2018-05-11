#ifndef __XPORT_H__
#define __XPORT_H__

class CtlXportParser
{

};

class CallbackXportParser
{

};

typedef struct _DemuxSelectProgramT
{
	int                     detect_program_pids;
	unsigned int	        program;
	unsigned int	        video_channel;
	unsigned int	        audio_channel;

	unsigned short	        pcr_pid;
	unsigned short	        video_pid;
	unsigned short	        audio_pid;
	unsigned char	        audio_stream_type;
	unsigned char	        video_stream_type;
} DemuxSelectProgramT;

typedef struct AudParam
{
    int nSampleRate;
    int nNumChannels;
} AudParam;

typedef void (*pat_callback_t)(void *pCtx, const char *pPsiData, int len);
typedef void (*pmt_callback_t)(void *pCtx, int nPid, const char *pPmtData, int len);
typedef void (*fmt_callback_t)(void *pCtx, int nPid, int codecType, const char *pFmtData, int len);

typedef struct _DemuxSubscribeProgramPidT
{
	int nPid;
	pmt_callback_t pmt_callback;
} DemuxSubscribeProgramPidT;


typedef struct _PgmPcrT
{
	int nPid;
	unsigned long long	clk;
} PgmPcrT;

typedef struct _PmtDataT
{
	int nPid;
	char *pData;
	int  nLen;
} PmtDataT;

typedef struct _PidStatT
{
	int nPid;
	unsigned long long countDrops;
	unsigned long long sizeLost;
	unsigned long long sizeTotal;
	unsigned long long bitrate;
	unsigned long long nsJitter; // nano seconds
} PidStatT;

#define DEMUX_CMD_SELECT_PROGRAM	     1
#define DEMUX_CMD_SET_PAT_CALLBACK       2
#define DEMUX_CMD_SET_PMT_CALLBACK       3
#define DEMUX_CMD_SET_PSI_CALLBACK_CTX   4
#define DEMUX_CMD_SUBSCRIBE_PROGRAM_PID  5
#define DEMUX_CMD_SUBSCRIBE_ES_PID       6
#define DEMUX_CMD_SET_FMT_CALLBACK		 7
#define DEMUX_CMD_GET_PCR        		 8
#define DEMUX_CMD_GET_PID_STAT           9
#define DEMUX_CMD_GET_PMT_DATA           10

#include "strmcomp.h"

StrmCompIf *demuxCreate();
#endif //__XPORT_H__
