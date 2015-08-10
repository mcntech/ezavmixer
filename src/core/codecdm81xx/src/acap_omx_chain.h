#ifndef __ACAP_OMX_CHAIN_H__
#define __ACAP_OMX_CHAIN_H__

#include "strmcomp.h"

#define MAX_AUD_DEV_NAME_SIZE      256

/* Arguments for app */
typedef struct _IL_LOOP_ARGS
{
	char input_device[MAX_AUD_DEV_NAME_SIZE];
	char output_device[MAX_AUD_DEV_NAME_SIZE];
	int  nSampleRate;
	int  buffer_size;			// Parser buffer size (used as jitter buffer)
} IL_ALOOP_ARGS;

#define AUD_LOOP_CMD_SET_PARAMS		1
StrmCompIf *acapchainCreate();

#endif
