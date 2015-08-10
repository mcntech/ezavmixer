#ifndef __ADEC_OMX_CHAIN_H__
#define __ADEC_OMX_CHAIN_H__

#include "strmcomp.h"

#define MAX_CODEC_NAME_SIZE     16
#define MAX_RAW_FORMAT_SIZE     16
#define MAX_SAMPLE_RATE_SIZE    16
#define MAX_FILE_NAME_SIZE      256

/* Arguments for app */
typedef struct _IL_AUD_ARGS
{
	char codec_name[MAX_CODEC_NAME_SIZE];
	char out_device[MAX_FILE_NAME_SIZE];
	int  nRrawFormat;
	int  nSampleRate;
	int  buffer_size;			// Parser buffer size (used as jitter buffer)
	int  max_input_pkt_size;	// Max input size. 188 for TS Demux, (8*1536) for SDI AC3
	int  dec_input_buffer_size;
	int  dec_output_buffer_size;
	int  alsa_output_buffer_size;
	
	int  sync;
	int  latency;
	int  nSessionId;
} IL_AUD_ARGS;

#define AUD_DEC_CMD_SET_PARAMS		1
StrmCompIf *audchainCreate();

#endif
