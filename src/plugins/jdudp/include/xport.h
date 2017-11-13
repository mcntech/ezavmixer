#ifndef __XPORT_H__
#define __XPORT_H__

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

#define DEMUX_CMD_SELECT_PROGRAM	1

#include "strmcomp.h"

StrmCompIf *demuxCreate();
#endif //__XPORT_H__
