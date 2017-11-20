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


typedef void (*pat_callback_t)(void *pCtx, const char *pPsiData, int len);
typedef void (*pmt_callback_t)(void *pCtx, int nPid, const char *pPmtData, int len);

typedef struct _DemuxSubscribeProgramPidT
{
	int nPid;
	pmt_callback_t pmt_callback;
} DemuxSubscribeProgramPidT;


#define DEMUX_CMD_SELECT_PROGRAM	     1
#define DEMUX_CMD_SET_PAT_CALLBACK       2
#define DEMUX_CMD_SET_PMT_CALLBACK       3
#define DEMUX_CMD_SET_PSI_CALLBACK_CTX   4
#define DEMUX_CMD_SUBSCRIBE_PROGRAM_PID  5
#define DEMUX_CMD_SUBSCRIBE_ES_PID       6

#include "strmcomp.h"

StrmCompIf *demuxCreate();
#endif //__XPORT_H__
