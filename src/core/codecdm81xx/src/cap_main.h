#ifndef __CAP_MAIN_H__
#define __CAP_MAIN_H__

#define  DVM_SDI_SRC      "/dev/dvm_sdi"

int capmainStartStreaming(
	char *szInput, 
	char *szVidCodecName, 
	char *szAudCodecName, 
	int  nSrcWidth, int  nSrcHeight, 
	int  nDispWidth, int nDispHeight, 
	int  nEncWidth, int nEncHeight, 
	int  nAudSampleRate, 
	int  nLatency, 
	int  neinterlace,
	int  nFrameRate,
	void (*strmCallback)(void *, int),	
	void *pContext);

int capmainStopStreaming();

#endif