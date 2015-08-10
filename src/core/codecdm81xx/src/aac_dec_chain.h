#ifndef __IL_ADEC_CHAIN_H__
#define __IL_ADEC_CHAIN_H__

#define ADEC_CHAIN_CMD_SET_PARAMS		1

/* Arguments for app */
typedef struct _ADEC_CHAIN_ARGS
{
	int  buffer_size;			// Parser buffer size (used as jitter buffer)
} ADEC_CHAIN_ARGS;

StrmCompIf *aacdecchainCreate();

#endif
