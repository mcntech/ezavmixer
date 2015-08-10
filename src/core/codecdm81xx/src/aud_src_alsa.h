#ifndef __AUD_SRC_ALSA__
#define __AUD_SRC_ALSA__
#include "strmcomp.h"

#define ALSA_AC3_READ_SIZE (8 * 1536)

#define AUD_SRC_ALSA_CMD_SET_PARAMS		1

/* Arguments for app */
typedef struct _IL_AUD_SRC_ALSA_ARGS
{
	char device[MAX_AUD_DEV_NAME_SIZE];
	char output_device[MAX_AUD_DEV_NAME_SIZE];
	int  nSampleRate;
	int  buffer_size;			// Parser buffer size (used as jitter buffer)
	int fDetectFormat;
} IL_AUD_SRC_ALSA_ARGS;

StrmCompIf *audsrcalsaCreate();

#endif // __AUD_SRC_ALSA__