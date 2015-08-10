#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <stdio.h>
#include <xdc/runtime/knl/Thread.h>

/*-------------------------program files -------------------------------------*/
#include "ti/omx/interfaces/openMaxv11/OMX_Core.h"
#include "ti/omx/interfaces/openMaxv11/OMX_Component.h"
#include "msgq.h"
#include "vcap_omx_chain.h"
#include "acap_omx_chain.h"
#include "adec_omx_chain.h"
#include "aud_src_alsa.h"
#include "dbglog.h"

#define AUD_SRC_ALSA_PCM        0
#define AUD_SRC_ALSA_AC3        1

typedef struct STREAMING_CTX_
{
	IL_VCAP_ARGS           args;
	IL_ALOOP_ARGS          audArgs;       // For PCM
	IL_AUD_SRC_ALSA_ARGS   audsrcalsaArgs;       // For PCM
	IL_AUD_ARGS            auddecArgs;    // For ac3 decode
	StrmCompIf            *pAudChain;
	StrmCompIf             *pVidChain;    
	StrmCompIf             *pAudSrc;     // Used for AC3
#ifdef WIN32
	HANDLE                 thrdIdVidOmxChain;
	HANDLE                 thrdIdAudOmxChain;
#else
	pthread_t              thrdIdVidOmxChain;
	pthread_t              thrdIdAudOmxChain;
#endif
	int                    fAudEnable;
	int                    fVidEnable;
	int                    nAudSrcId;
	ConnCtxT               *pConnCtxAud;  // Used for AC3
} STREAMING_CTX;


static STREAMING_CTX  gStrmCtx = {{0}};


static void *thrdVideoStreaming(void *threadsArg)
{
	STREAMING_CTX *pStrmCtx = (STREAMING_CTX *)threadsArg;
	IL_VCAP_ARGS *args = &pStrmCtx->args;
	pStrmCtx->pVidChain->SetOption(pStrmCtx->pVidChain, VCAP_CMD_SET_PARAMS, args);
	pStrmCtx->pVidChain->Start(pStrmCtx->pVidChain);
}                     

static void *thrdAudioStreaming(void *threadsArg)
{
	STREAMING_CTX *pStrmCtx = (STREAMING_CTX *)threadsArg;
	if(pStrmCtx->nAudSrcId == AUD_SRC_ALSA_PCM){
		IL_ALOOP_ARGS *args = &pStrmCtx->audArgs;
		pStrmCtx->pAudChain->SetOption(pStrmCtx->pAudChain, AUD_LOOP_CMD_SET_PARAMS, args);
	} else if(pStrmCtx->nAudSrcId == AUD_SRC_ALSA_AC3){
		IL_AUD_ARGS *pAudDecArgs = &pStrmCtx->auddecArgs;
		IL_AUD_SRC_ALSA_ARGS *pAudCapArgs = &pStrmCtx->audsrcalsaArgs;
		pStrmCtx->pAudChain->SetOption(pStrmCtx->pAudChain, AUD_DEC_CMD_SET_PARAMS, pAudDecArgs);
		pStrmCtx->pAudSrc->SetOption(pStrmCtx->pAudSrc, AUD_SRC_ALSA_CMD_SET_PARAMS, pAudCapArgs);
		
		/* The order of the following two calls should note be  changed due to ALSA device open issue */
		if(pStrmCtx->pAudSrc->Open(pStrmCtx->pAudSrc, NULL) < 0) {
			DBG_LOG(DBGLVL_ERROR, ("Failed to open sdi ac3 audio source"));
			goto Exit;
		}
		pStrmCtx->pAudSrc->Close(pStrmCtx->pAudSrc);

		if(pStrmCtx->pAudChain->Open(pStrmCtx->pAudChain, NULL) < 0){
			DBG_LOG(DBGLVL_ERROR, ("Failed to open aud dec chain"));
			goto Exit;
		}
		if(pStrmCtx->pAudSrc->Open(pStrmCtx->pAudSrc, NULL) < 0) {
			DBG_LOG(DBGLVL_ERROR, ("Failed to open sdi ac3 audio source"));
			goto Exit;
		}

	}
	if(pStrmCtx->nAudSrcId == AUD_SRC_ALSA_AC3){
		pStrmCtx->pAudSrc->Start(pStrmCtx->pAudSrc);
	}
	// !!! Thread Blocks here !!!
	pStrmCtx->pAudChain->Start(pStrmCtx->pAudChain);
Exit:
	pthread_exit(0);
}                     


int capmainStartStreaming(
	char *szInput, 
	char *szVidCodecName, 
	char *szAudCodecName, 
	int  nSrcWidth, int  nSrcHeight, 
	int  nDispWidth, int nDispHeight, 
	int  nEncWidth, int nEncHeight, 
	int  nAudSampleRate, 
	int  nLatency, 
	int  nInterlace,
	int  nFrameRate,
	void (*strmCallback)(void *, int),	
	void *pContext)
{
	STREAMING_CTX *pStrmCtx = &gStrmCtx;
	IL_VCAP_ARGS *argsp = &pStrmCtx->args;
	IL_ALOOP_ARGS *audargs = &pStrmCtx->audArgs;
	IL_AUD_ARGS   *auddecArgs = &pStrmCtx->auddecArgs;
	IL_AUD_SRC_ALSA_ARGS *audsrcalsaArgs = &pStrmCtx->audsrcalsaArgs;
	argsp->frame_rate = nFrameRate;
	argsp->bit_rate = 10000000;
	argsp->disp_width = nDispWidth;
	argsp->disp_height = nDispHeight;

	argsp->num_frames = 1000;
	if(nSrcHeight == 1080){
		if(nInterlace) {
			strcpy(argsp->mode,"1080i");
		} else {
			strcpy(argsp->mode,"1080p");
		}
	} else if (nSrcHeight == 720){
		strcpy(argsp->mode,"720p");
	}
	argsp->display_id = 1;
	argsp->audchan_l = 0;
	argsp->audchan_r = 1;

	if(szVidCodecName && strcmp(szVidCodecName, "null") == 0){
		pStrmCtx->fVidEnable = 0;
	} else {
		pStrmCtx->fVidEnable = 1;
	}

	if(szAudCodecName && strcmp(szAudCodecName, "pcm") == 0 ){
		pStrmCtx->fAudEnable = 1;
		strcpy(audargs->input_device, "sdi_audio");
		//strcpy(audargs->input_device, "hw:0,0");
		strcpy(audargs->output_device, "default");
		//strcpy(audargs->output_device, "hw:0,2");
		audargs->buffer_size = (16 * 8 * 1024); // 16 1024 sample frames
		pStrmCtx->nAudSrcId = AUD_SRC_ALSA_PCM;
	} else if(szAudCodecName && strcmp(szAudCodecName, "ac3") == 0){
		pStrmCtx->fAudEnable = 1;
		/* Capture Params */
		strcpy(audsrcalsaArgs->device, "sdi_audio");
		audsrcalsaArgs->buffer_size = (16 * 8 * 1024); // 16 1024 sample frames
		audsrcalsaArgs->fDetectFormat = 1;
		/* Decode Params */
		auddecArgs->buffer_size = (16 * 8 * 1024); // 16 1024 sample frames
		auddecArgs->sync = 0;
		auddecArgs->max_input_pkt_size = ALSA_AC3_READ_SIZE;
		auddecArgs->dec_input_buffer_size = (2 * ALSA_AC3_READ_SIZE);
		auddecArgs->dec_output_buffer_size = (8 * 1536);
		auddecArgs->alsa_output_buffer_size = (8 * 1536);

		auddecArgs->nSampleRate = nAudSampleRate;
		strncpy (auddecArgs->codec_name, szAudCodecName, MAX_CODEC_NAME_SIZE);
		pStrmCtx->nAudSrcId = AUD_SRC_ALSA_AC3;
	} else {
		pStrmCtx->fAudEnable = 0;
	}

#ifndef DBG_DO_NOT_USE_TI_OMX
//	OMX_Init ();
#endif
	if(pStrmCtx->fVidEnable) {
		DBG_LOG(DBGLVL_SETUP, ("Creating Video Capture Chain"));

#ifndef DBG_DO_NOT_USE_TI_OMX
		pStrmCtx->pVidChain = vcapchainCreate();
#else
		pStrmCtx->pVidChain = fakevcapCreate();
#endif

#ifdef WIN32
		DWORD dwThreadId;
		pStrmCtx->thrdIdVidOmxChain = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)&thrdVideoStreaming,(LPVOID) pStrmCtx,0,&dwThreadId);
#else
		pthread_create(&pStrmCtx->thrdIdVidOmxChain, NULL, thrdVideoStreaming, pStrmCtx);
#endif
	}

	if(pStrmCtx->fAudEnable) {
		DBG_LOG(DBGLVL_SETUP, ("Creating Audio Capture Chain"));
#ifndef DBG_DO_NOT_USE_TI_OMX
		if(pStrmCtx->nAudSrcId == AUD_SRC_ALSA_PCM){
			DBG_LOG(DBGLVL_SETUP, ("Creating PCM Loopback chain"));
			pStrmCtx->pAudChain = acapchainCreate();
		} else if(pStrmCtx->nAudSrcId == AUD_SRC_ALSA_AC3) {
			DBG_LOG(DBGLVL_SETUP, ("Creating AC3 capture and playback"));
			pStrmCtx->pAudSrc = audsrcalsaCreate();
			pStrmCtx->pAudChain = audchainCreate();
			pStrmCtx->pConnCtxAud = CreateStrmConn(ALSA_AC3_READ_SIZE, 2);

			pStrmCtx->pAudSrc->SetOutputConn(pStrmCtx->pAudSrc, 0, pStrmCtx->pConnCtxAud);
			pStrmCtx->pAudChain->SetInputConn(pStrmCtx->pAudChain, 0, pStrmCtx->pConnCtxAud);
		}
#else
		//pStrmCtx->pAudChain = fakeacapCreate();
#endif

#ifdef WIN32
		DWORD dwThreadId;
		pStrmCtx->thrdIdAudOmxChain = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)&thrdAudioStreaming,(LPVOID) pStrmCtx,0,&dwThreadId);
#else
		pthread_create(&pStrmCtx->thrdIdAudOmxChain, NULL, thrdAudioStreaming, pStrmCtx);
#endif
	}
}

int capmainStopStreaming()
{
	void *ret_value;
	STREAMING_CTX *pStrmCtx = &gStrmCtx;
	DBG_LOG(DBGLVL_SETUP, ("Enter"));

	// TODO: Wait for thread completion
	if(pStrmCtx->fVidEnable) {
		pStrmCtx->pVidChain->Stop(pStrmCtx->pVidChain);
		DBG_LOG(DBGLVL_SETUP, ("Waiting for completion of main thread..."));
#ifdef WIN32
		WaitForSingleObject(pStrmCtx->thrdIdVidOmxChain, 0);
#else
		pthread_join (pStrmCtx->thrdIdVidOmxChain, (void **) &ret_value);
#endif

		if(pStrmCtx->pVidChain) {
			pStrmCtx->pVidChain->Delete(pStrmCtx->pVidChain);
			pStrmCtx->pVidChain = NULL;
		}

		DBG_LOG(DBGLVL_SETUP, (":Vid: main thread stopped..."));
	}

	if(pStrmCtx->fAudEnable) {
		if(pStrmCtx->nAudSrcId == AUD_SRC_ALSA_AC3){
			pStrmCtx->pAudSrc->Stop(pStrmCtx->pAudSrc);
		}

		DBG_LOG(DBGLVL_SETUP, (":AudChain_Stop..."));
		pStrmCtx->pAudChain->Stop(pStrmCtx->pAudChain);
		DBG_LOG(DBGLVL_SETUP, (":Aud:Waiting for completion of main thread..."));
#ifdef WIN32
		WaitForSingleObject(pStrmCtx->thrdIdAudOmxChain, 0);
#else
		pthread_join (pStrmCtx->thrdIdAudOmxChain, (void **) &ret_value);
#endif
		if(pStrmCtx->nAudSrcId == AUD_SRC_ALSA_AC3){
			if(pStrmCtx->pAudSrc) {
				pStrmCtx->pAudSrc->Close(pStrmCtx->pAudSrc);
				pStrmCtx->pAudSrc->Delete(pStrmCtx->pAudSrc);
				pStrmCtx->pAudSrc = NULL;
			}
			if(pStrmCtx->pConnCtxAud){
				DeleteStrmConn(pStrmCtx->pConnCtxAud);
			}
		}

#if 1 // Check why is this not enabled
		if(pStrmCtx->pAudChain) {
			pStrmCtx->pAudChain->Delete(pStrmCtx->pAudChain);
			pStrmCtx->pAudChain = NULL;
		}
#endif


		DBG_LOG(DBGLVL_SETUP, (":Aud: main thread stopped..."));
	}

#ifndef DBG_DO_NOT_USE_TI_OMX
	OMX_Deinit();
#endif

	DBG_LOG(DBGLVL_SETUP, ("Leave"));
}
