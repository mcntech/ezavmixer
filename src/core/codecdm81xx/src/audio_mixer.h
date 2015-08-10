#ifndef __AUDIO_MIXER__
#define __AUDIO_MIXER__

#include <stdint.h>
#include <stdint.h>
#include "strmcomp.h"
#include "layout.h"
#include "dec_clock.h"

#ifdef __cplusplus              /* required for headers that might */
extern "C"
{                               /* be compiled by a C++ compiler */
#endif


#define OUT_ACHAIN_MIX_ENC_SPKR 0
#define OUT_ACHAIN_MIX_ENC      1
#define OUT_ACHAIN_ENC          2
#define OUT_ACHAIN_SPKR         3

#define MAX_INPUT_CHANNELS      6

typedef struct _AUDIO_MIXER_T
{
	StrmCompIf     *pAEncChain;
	int            nNumChannels;
	int            nSampleRate;
	int            nNumOutChannels;
	int	           nSamplesPerFrame;
	int            nEncBitrate;
	int            nOutputBuffers;
	int            nInputBuffers;
	int            nDestType;
	int            fEnableSpkr;
	ACHAN_PARAM_T *listInputChannels;
	ACHAN_PARAM_T AChanOutput;
	ConnCtxT      *listInputConn[MAX_INPUT_CHANNELS];
	ConnCtxT      *pConnMixerToEnc;
	ConnCtxT      *pConnEncOutput;

	char          *pMixBuff;
	int            nMixBuffSize;
	void          *thrdHandle;
	int            m_fStream;

	unsigned long long crnt_input_pts;
	unsigned long long crnt_out_pts;
    unsigned long nMixedFrms;
	unsigned long nEmptyFrms;
	int           nStatPrevFrames;
	int           nStatPrevEmptyFrames;
	CLOCK_T       StatPrevClk;

} AUDIO_MIXER_T;


AUDIO_MIXER_T *amixInit(
	int nDestType, 
	int fEncode,
	int fEnableSpkr,
	int nNumChannels,
	int nSampleRate,
	int nNumOutChannels,
	int nSamplesPerFrame,
	ACHAN_PARAM_T *listInputChannels,
	int nEncBitrate
	);
int amixDeinit(AUDIO_MIXER_T *pAMixer);

AUDIO_MIXER_T *amixGetInstance();

ConnCtxT *amixGetInputPortConnCtx(AUDIO_MIXER_T *pCtx, int nPort);
int amixGetInputPortParam(AUDIO_MIXER_T *pCtx, int nPort, int *pnSampleRate, int *pnSamplesPerFrame, int *pnNumChannels);
int amixSetStreamSink(AUDIO_MIXER_T *pCtx, ConnCtxT *pSink);

int amixStartStream(AUDIO_MIXER_T *pAppDAta);
int amixStopStream(AUDIO_MIXER_T *pAppDAta);


#ifdef __cplusplus              /* matches __cplusplus construct above */
}
#endif

#endif
