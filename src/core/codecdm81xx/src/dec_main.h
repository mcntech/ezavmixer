#ifndef __DEC_MAIN_H__
#define __DEC_MAIN_H__

typedef enum
{
	DEC_SRC_URI,
	DEC_SRC_STRM_CONN
} DEC_SRC_TYPE;

int decmainAcquireResource(
	void *pCtx,
	int  nSessionId,
	int  nVidSrcType,
	void *szVidInput, 
	char *szVidCodecName, 
	int  nAudSrcType,
	void *szAudInput, 
	char *szAudCodecName, 
	int  nDecWidth, int  nDecHeight, 
	int  nDispWidth, int nDispHeight, 
	int  nAudSampleRate, 
	int  nLatency, 
	int  nDeinterlace,
	int  nFrameRate,
	int nDetectProgramPids,
	int nPcrPidOrProg, int nAudPidOrChan, int nVidPidOrChan,
	void (*strmCallback)(void *, int),	
	void *pContext);
int decmainStartStreaming(void *pCtx);
int decmainStopStreaming(void *pCtx);

void *decmainCreateStream();
void decmainDeleteStream(void *);
#endif