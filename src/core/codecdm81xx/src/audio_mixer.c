#include <stdlib.h>
#include <stdio.h>
#ifdef WIN32
#include <winsock2.h>
#else //LINUX
#include <pthread.h>
#include <unistd.h>
#include <xdc/std.h>
#endif
#include "audio_mixer.h"
#include "dbglog.h"
#include "oal.h"
#include "dbglog.h"

#define AUD_BYTES_PER_SAMPLE     2

static AUDIO_MIXER_T *pamixCtx = NULL;


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
	)
{
	StrmCompIf *pEncChain = NULL;
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	
	if(pamixCtx == NULL) {
		int i;
		pamixCtx = (AUDIO_MIXER_T *)malloc(sizeof(AUDIO_MIXER_T));
		memset(pamixCtx, 0x00, sizeof(AUDIO_MIXER_T));

		pamixCtx->nDestType = nDestType;
		pamixCtx->nNumChannels = nNumChannels;
		pamixCtx->nEncBitrate = nEncBitrate;
		pamixCtx->nSampleRate = nSampleRate;
		pamixCtx->nNumOutChannels = nNumOutChannels;
		pamixCtx->nSamplesPerFrame = nSamplesPerFrame;
		pamixCtx->nMixBuffSize = AUD_BYTES_PER_SAMPLE * pamixCtx->nNumOutChannels * pamixCtx->nSamplesPerFrame; // 1024 stereo 16bit smaples
		pamixCtx->pMixBuff =  (char *)malloc(pamixCtx->nMixBuffSize);
		
		pamixCtx->pConnMixerToEnc = CreateStrmConn(pamixCtx->nMixBuffSize, 3); // 3 AAC Frames

		for(i=0; i < pamixCtx->nNumChannels; i++){
			pamixCtx->listInputConn[i] = CreateStrmConn(pamixCtx->nMixBuffSize, 3);
		}

		if(pamixCtx->nDestType == OUT_ACHAIN_MIX_ENC_SPKR || pamixCtx->nDestType == OUT_ACHAIN_MIX_ENC) {
			if(nNumChannels) {
				pamixCtx->listInputChannels = malloc(nNumChannels * sizeof(ACHAN_PARAM_T));
				memcpy(pamixCtx->listInputChannels, listInputChannels, nNumChannels * sizeof(ACHAN_PARAM_T));
			}
		} else if (pamixCtx->nDestType == OUT_ACHAIN_SPKR) {
			// TODO
		}
		if (pamixCtx->nDestType == OUT_ACHAIN_ENC || pamixCtx->nDestType == OUT_ACHAIN_MIX_ENC_SPKR || pamixCtx->nDestType == OUT_ACHAIN_MIX_ENC) {
			pEncChain = aencchainCreate();
			pamixCtx->pAEncChain = pEncChain;
		}

		if(pamixCtx->pAEncChain) {
			pamixCtx->pAEncChain->Open(pamixCtx->pAEncChain, NULL);
			pamixCtx->pAEncChain->SetInputConn(pamixCtx->pAEncChain, 0, pamixCtx->pConnMixerToEnc);
			pamixCtx->pAEncChain->SetOutputConn(pamixCtx->pAEncChain, 0, pamixCtx->pConnEncOutput);
		}

		if(pamixCtx->pAEncChain) {
			pamixCtx->pAEncChain->Start(pamixCtx->pAEncChain);
		}

		amixStartStream(pamixCtx);
	}
EXIT:

	// TODO: ValidatepamixCtx->nDestType == nDestType
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return pamixCtx;
}

AUDIO_MIXER_T *amixGetInstance()
{
	return pamixCtx;
}


int amixDeinitResource(AUDIO_MIXER_T *pCtx)
{
	int i;
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	if(pamixCtx->pAEncChain) {
		pamixCtx->pAEncChain->Delete(pamixCtx->pAEncChain);
	}
	
	if(pamixCtx->listInputChannels){
		DBG_LOG(DBGLVL_TRACE, ("Free listInputChannels"));
		free(pamixCtx->listInputChannels);
	}
	
	if(pamixCtx->pConnMixerToEnc) {
		DBG_LOG(DBGLVL_TRACE, ("DeleteStrmConn"));
		DeleteStrmConn(pamixCtx->pConnMixerToEnc);
	}
	for(i=0; i < pamixCtx->nNumChannels; i++){
		DeleteStrmConn(pamixCtx->listInputConn[i]);
	}

	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return 0;
}

ConnCtxT *amixGetInputPortConnCtx(AUDIO_MIXER_T *pCtx, int nPort)
{
	ConnCtxT *pInConn = NULL;
	DBG_LOG(DBGLVL_TRACE, ("Enter"));

	if(pCtx->nDestType == OUT_ACHAIN_MIX_ENC_SPKR || pCtx->nDestType == OUT_ACHAIN_MIX_ENC) {
		if(nPort >= 0 && nPort < pamixCtx->nNumChannels) {
			pInConn = pamixCtx->listInputConn[nPort];
		}
	} else if(pCtx->nDestType == OUT_ACHAIN_ENC) {
		// Connect to Encoder
		//IL_ClientConnectComponents (pPeer, nPeerPort,	pCtx->disILComp,  OMX_VFDC_INPUT_PORT_START_INDEX);
	} else if(pCtx->nDestType == OUT_ACHAIN_SPKR) {
		// Connect to speaker
	}
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	 return pInConn;
}

int amixGetInputPortParam(AUDIO_MIXER_T *pCtx, int nPort, int *pnSampleRate, int *pnSamplesPerFrame, int *pnNumChannels)
{
	*pnSamplesPerFrame = pCtx->nSamplesPerFrame;
	*pnSampleRate = pCtx->nSampleRate;
	*pnNumChannels = pCtx->nNumOutChannels;
}

int amixSetStreamSink(AUDIO_MIXER_T *pCtx, ConnCtxT *pSink)
{
	pCtx->pConnEncOutput = pSink;
	if(pamixCtx->pAEncChain) {
		pamixCtx->pAEncChain->SetOutputConn(pamixCtx->pAEncChain, 0, pamixCtx->pConnEncOutput);
	}
}

static int AddChannelFrame(short *pMixingBuff, short *pChanBuff, int nMixBuffSamples, int nChanVolRatio)
{
#if 1
	int i;
	for (i=0; i < nMixBuffSamples; i++) {
		float samplef1 = pMixingBuff[i] / 32768.0f;
		float samplef2 = pChanBuff[i] / 32768.0f;
		float mixed = samplef1 + samplef2;
		// reduce the volume a bit:
		//mixed *= 0.8;
		// hard clipping
		if (mixed > 1.0f) mixed = 1.0f;
		if (mixed < -1.0f) mixed = -1.0f;
		pMixingBuff[i] = (short)(mixed * 32768.0f);
	}
#else
	memcpy(pMixingBuff, pChanBuff, nMixBuffSamples * AUD_BYTES_PER_SAMPLE);
#endif
	return 0;
}

static ShowStats(AUDIO_MIXER_T *pCtx)
{
	// Display Statistics
	{
		char szClck[256];
		char szPts[256];

		CLOCK_T pts = pCtx->crnt_input_pts;
		CLOCK_T clk = ClockGetInternalTime(NULL/*pCtx->pClk*/);
		if(clk - pCtx->StatPrevClk >= TIME_SECOND) {
			int buffOcupancy;
			float fMixRate;
			float fEmptyFrmRate;
			Clock2HMSF(clk, szClck, 255);
			Clock2HMSF(pts, szPts, 255);
			buffOcupancy = 0;//pCtx->pConnSrc->BufferFullness(pCtx->pConnSrc);
			fMixRate = 1.0 * (pCtx->nMixedFrms - pCtx->nStatPrevFrames) / ((clk - pCtx->StatPrevClk) / TIME_SECOND);
			fEmptyFrmRate = 1.0 * (pCtx->nEmptyFrms - pCtx->nStatPrevEmptyFrames) / ((clk - pCtx->StatPrevClk) / TIME_SECOND);
			DBG_PRINT("<%s:Mix frame=%d(Empty=%d) rate=%0.2f(Empty=%0.2f) crnt frame  pts=%s>\n", szClck,pCtx->nMixedFrms, pCtx->nEmptyFrms, fMixRate, fEmptyFrmRate, szPts);
			pCtx->nStatPrevFrames = pCtx->nMixedFrms;
			pCtx->nStatPrevEmptyFrames = pCtx->nEmptyFrms;
			pCtx->StatPrevClk = clk;
		}
	}
}

static void *thrdProcess(void *thrdArg)
{
	AUDIO_MIXER_T *pamixCtx = (AUDIO_MIXER_T *)thrdArg;
	long long ullPts;
	unsigned long ulFlags;
	
	void *pClck = clkCreate(0);
	CLOCK_T clkNextFrame = 0;
	
	CLOCK_T clkFrameDurationUs = (1000000.0 / pamixCtx->nSampleRate) * pamixCtx->nSamplesPerFrame;
	char *pMixBuff = pamixCtx->pMixBuff;
	int   nMixBuffSize = pamixCtx->nMixBuffSize;
	int nInLen = nMixBuffSize;
	char *pInData = (char *)malloc(nMixBuffSize);
	int fEmptyMixFrame;
	ClockStart(pClck, 0);
	pamixCtx->nMixedFrms = 0;
	pamixCtx->nEmptyFrms = 0;
	while(pamixCtx->m_fStream){
		int i;

		//clear frame
		memset(pMixBuff, 0x00, nMixBuffSize);
		fEmptyMixFrame = 1;
		for(i=0; i < pamixCtx->nNumChannels; i++){
			ConnCtxT *pInConn = pamixCtx->listInputConn[i];
			if(pInConn && !pInConn->IsEmpty(pInConn)) {
				 //while(pInConn->IsEmpty(pInConn) && pamixCtx->m_fStream){
				//	 OAL_TASK_SLEEP(5)
				 //}
				if(!pamixCtx->m_fStream) goto Exit;
				
				pInConn->Read(pInConn, pInData, &nInLen, &ulFlags, &ullPts);
				AddChannelFrame((short *)pMixBuff, (short *)pInData, pamixCtx->nNumOutChannels *  pamixCtx->nSamplesPerFrame, 1);
				fEmptyMixFrame = 0;
			}
		}
		if(pamixCtx->pConnMixerToEnc) {
			while(pamixCtx->pConnMixerToEnc->IsFull(pamixCtx->pConnMixerToEnc) && pamixCtx->m_fStream)
				OAL_TASK_SLEEP(2)
			if(pamixCtx->m_fStream)
				pamixCtx->pConnMixerToEnc->Write(pamixCtx->pConnMixerToEnc, pMixBuff, nMixBuffSize, 0, 0);
		}
		if(fEmptyMixFrame)
			pamixCtx->nEmptyFrms++;
		pamixCtx->nMixedFrms++;
		ShowStats(pamixCtx);
		// Wait for next frame clock
		clkNextFrame = pamixCtx->nMixedFrms * clkFrameDurationUs;
		WaitForClock(pClck, clkNextFrame, 50000);
	}
Exit:

	free(pInData);
	clockDelete(pClck);
}

int amixStartStream(AUDIO_MIXER_T *pCtx)
{
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	pamixCtx->m_fStream = 1;
	oalThreadCreate(&pCtx->thrdHandle, thrdProcess,pCtx);
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return 0;
}

int amixStopStream(AUDIO_MIXER_T *pCtx)
{
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	if(pCtx->pAEncChain) {
		pCtx->pAEncChain->Stop(pCtx->pAEncChain);
	}
	pCtx->m_fStream = 0;
	oalThreadJoin(pCtx->thrdHandle, 1000);
	DBG_LOG(DBGLVL_TRACE, ("Leave"));

	return 0;
}

int amixDeinit(AUDIO_MIXER_T *pCtx)
{
	DBG_LOG(DBGLVL_TRACE, ("Enter"));

	amixStopStream(pamixCtx);
	amixDeinitResource(pamixCtx);
	free(pamixCtx);
	pamixCtx = NULL;


	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return 0;
}
