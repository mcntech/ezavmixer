#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#ifdef WIN32
#include <windows.h>
#else
#endif
#ifndef DBG_DO_NOT_USE_TI_OMX
#include <xdc/runtime/System.h>
#include <xdc/std.h>
#include <xdc/runtime/knl/Thread.h>

/*-------------------------program files -------------------------------------*/
#include "ti/omx/interfaces/openMaxv11/OMX_Core.h"
#include "ti/omx/interfaces/openMaxv11/OMX_Component.h"
#endif


#include "strmcomp.h"
#include "udprx.h"
#include "dec_filesrc.h"
#include "dec_xport.h"
#include "vdec_omx_chain.h"
#include "adec_omx_chain.h"
#include "vdec_fakedec.h"
#include "dec_clock.h"
#include "dbglog.h"
#include "dec_main.h"
#include "minini.h"
#ifndef DBG_DO_NOT_USE_TI_OMX
#include "aac_dec_chain.h"
#endif
#include "g711_dec_chain.h"
#include "audio_mixer.h"

int decmainRestart(void *pCtx);

#define  UDP_SRC_PREFIX   "udp://"

typedef struct STREAMING_CTX_
{
	IL_VID_ARGS            args;
	IL_AUD_ARGS            audArgs;
	DemuxSelectProgramT    demuxArgs;

	StrmCompIf             *srcComp;
	StrmCompIf            *demuxComp;
	StrmCompIf            *pAudChain;
	StrmCompIf             *pVidChain;
#ifdef WIN32
	HANDLE                 thrdIdVidOmxChain;
	HANDLE                 thrdIdAudOmxChain;
#else
	pthread_t              thrdIdVidOmxChain;
	pthread_t              thrdIdAudOmxChain;
#endif
	int                    fAudEnable;
	int                    fVidEnable;
	ConnCtxT               *pConnCtxSrcToDemux;
	int                    nVidSrcType;
	int                    nAudSrcType;
	ConnCtxT               *pConnVidChainSrc;
	ConnCtxT               *pConnAudChainSrc;
} STREAMING_CTX;



int decmainAcquireResource(
	void *pCtx,
	int  nSessionId,
	int  nVidSrcType,
	void *pVidInput, 
	char *szVidCodecName, 
	int  nAudSrcType,
	void *pAudInput, 
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
	void *pContext)
{
	STREAMING_CTX *pStrmCtx = (STREAMING_CTX *)pCtx;
	IL_VID_ARGS *argsp = &pStrmCtx->args;
	IL_AUD_ARGS *pAudArgs = &pStrmCtx->audArgs;
	void *pMasterClk = NULL;
	int fFileSource = 0;
	int res = 0;
	DemuxSelectProgramT *pdemuxArgs;

	memset(pStrmCtx, 0x00, sizeof(STREAMING_CTX));

	pdemuxArgs = &pStrmCtx->demuxArgs;

	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	argsp->nInstanceId = nSessionId;
	argsp->dbglevel = 1;
	argsp->statuslog = 0;
	argsp->eventlong = 0;
	argsp->use_demux = 1;
	argsp->display_id = 1;
	argsp->frame_rate = 60;
	argsp->gfx = 0;
	argsp->sync = nLatency ? 1 : 0;
	argsp->latency = nLatency * 1000; // micro secs
	argsp->deinterlace = nDeinterlace;
	argsp->frame_rate = nFrameRate;
	argsp->buffer = 4096;             // KB

	argsp->dec_width = nDecWidth;
	argsp->dec_height = nDecHeight;

	argsp->disp_width = nDispWidth;
	argsp->disp_height = nDispHeight;

	pAudArgs->nSessionId = nSessionId;
	pAudArgs->sync = nLatency ? 1 : 0;
	pAudArgs->latency = nLatency * 1000; // micro secs
	pAudArgs->max_input_pkt_size = 188;
	

	if(szVidCodecName) {
		DBG_LOG(DBGLVL_TRACE, ("using video codec %s", szVidCodecName));
		strncpy (argsp->codec_name, szVidCodecName, MAX_CODEC_NAME_SIZE);
		pStrmCtx->fVidEnable = 1;
		if(strcmp(szVidCodecName, "mpeg2") == 0) {
			pdemuxArgs->video_stream_type = 0x02;
		} else {
			pdemuxArgs->video_stream_type = 0x1B;
		}
	} else {
		DBG_LOG(DBGLVL_TRACE, ("Disable Video"));
		pStrmCtx->fVidEnable = 0;
	}

	if(szAudCodecName && strcmp(szAudCodecName, "aaclc") == 0) {
		DBG_LOG(DBGLVL_TRACE, ("using audio codec %s samplerate=%d audbuff=%dkbytes", szAudCodecName, pAudArgs->nSampleRate, pAudArgs->buffer_size));
		strncpy (pAudArgs->codec_name, szAudCodecName, MAX_CODEC_NAME_SIZE);
		pAudArgs->nSampleRate = nAudSampleRate;
		pAudArgs->buffer_size = 256; // Kilobytes
		pStrmCtx->fAudEnable = 1;
		pdemuxArgs->audio_stream_type = 0x0F;

		pAudArgs->max_input_pkt_size = 188;
		pAudArgs->dec_input_buffer_size = (4 * 1024);
		pAudArgs->dec_output_buffer_size = (8 * 1024);
		pAudArgs->alsa_output_buffer_size = (8 * 1024);
	} else if(szAudCodecName && strcmp(szAudCodecName, "g711u") == 0) {
		DBG_LOG(DBGLVL_TRACE, ("using audio codec %s", szAudCodecName));
		strncpy (pAudArgs->codec_name, szAudCodecName, MAX_CODEC_NAME_SIZE);
		pStrmCtx->fAudEnable = 1;
		pAudArgs->max_input_pkt_size = 1024 * 4;
		pAudArgs->dec_input_buffer_size = (4 * 1024);
		pAudArgs->dec_output_buffer_size = (8 * 1024);
		pAudArgs->alsa_output_buffer_size = (8 * 1024);
	} else {
		DBG_LOG(DBGLVL_TRACE, ("Disable Audio"));
		pStrmCtx->fAudEnable = 0;
	}

	if(nVidSrcType == DEC_SRC_URI) {
		DBG_LOG(DBGLVL_TRACE, ("Using Source", (char *)pVidInput));
		strncpy (argsp->input_file, (char *)pVidInput, MAX_FILE_NAME_SIZE);

		if (strncmp(argsp->input_file, UDP_SRC_PREFIX, strlen(UDP_SRC_PREFIX)) == 0 ) {
			DBG_LOG(DBGLVL_TRACE, ("Using UDP Source"));
			pStrmCtx->srcComp = udprxCreate();
		} else {
			DBG_LOG(DBGLVL_TRACE, ("Using File Source"));
			pStrmCtx->srcComp = filesrcCreate();
		}
		if( pStrmCtx->srcComp->Open(pStrmCtx->srcComp, argsp->input_file) != 0) {
				res = -1;
				goto Exit;
		}

		pdemuxArgs->detect_program_pids = nDetectProgramPids;
		if(nDetectProgramPids == 0) {
			pdemuxArgs->pcr_pid = nPcrPidOrProg;
			pdemuxArgs->audio_pid = nAudPidOrChan;
			pdemuxArgs->video_pid = nVidPidOrChan;
		} else {
			pdemuxArgs->program = nPcrPidOrProg;
			pdemuxArgs->audio_channel = nAudPidOrChan;
			pdemuxArgs->video_channel = nVidPidOrChan;
		}

		pStrmCtx->pConnCtxSrcToDemux = CreateStrmConn(DMA_READ_SIZE, 64);
		pStrmCtx->demuxComp = demuxCreate();
		pStrmCtx->demuxComp->Open(pStrmCtx->demuxComp, NULL);
		pStrmCtx->demuxComp->SetOption(pStrmCtx->demuxComp, DEMUX_CMD_SELECT_PROGRAM, pdemuxArgs);

		pStrmCtx->srcComp->SetOutputConn(pStrmCtx->srcComp, 0, pStrmCtx->pConnCtxSrcToDemux);
		pStrmCtx->demuxComp->SetInputConn(pStrmCtx->demuxComp, 0, pStrmCtx->pConnCtxSrcToDemux);

		if (strncmp(argsp->input_file, UDP_SRC_PREFIX, strlen(UDP_SRC_PREFIX)) == 0 ) {
			DBG_LOG(DBGLVL_TRACE, ("UDP source"));
			pMasterClk = pStrmCtx->demuxComp->GetClkSrc(pStrmCtx->demuxComp);
		} else {
			// Local file
			DBG_LOG(DBGLVL_TRACE, ("File source"));
			fFileSource = 1;
		}

		if(pStrmCtx->fAudEnable) {
			int nAParseBuffers =  pAudArgs->buffer_size * 1024 / 188;
			DBG_LOG(DBGLVL_SETUP, ("Creating Stream Connection Demux to AudParse: buffsize=%dkbytes nParseBuffers=%d",  pAudArgs->buffer_size, nAParseBuffers))
			pStrmCtx->pConnAudChainSrc = CreateStrmConn(188, nAParseBuffers);
			pStrmCtx->demuxComp->SetOutputConn(pStrmCtx->demuxComp, 0, pStrmCtx->pConnAudChainSrc);
		}
		if(pStrmCtx->fVidEnable) {
			int nVParseBuffers = argsp->buffer * 1024 / 188;
			DBG_LOG(DBGLVL_SETUP, ("Creating Stream Connection Demux to VidParse: buffsize=%dkbytes nParseBuffers=%d", argsp->buffer, nVParseBuffers))
			pStrmCtx->pConnVidChainSrc = CreateStrmConn(188, nVParseBuffers);
			pStrmCtx->demuxComp->SetOutputConn(pStrmCtx->demuxComp, 1, pStrmCtx->pConnVidChainSrc);
		}
	} else if (nVidSrcType == DEC_SRC_STRM_CONN) {
		if(pStrmCtx->fVidEnable) {
			pStrmCtx->pConnVidChainSrc = (ConnCtxT *)pVidInput;
		}
		if(pStrmCtx->fAudEnable) {
			pStrmCtx->pConnAudChainSrc = (ConnCtxT *)pAudInput;
		}
	}
	pStrmCtx->nVidSrcType = nVidSrcType;
	argsp->strmCallback = strmCallback;
	argsp->pAppCtx = pContext;

#ifndef DBG_DO_NOT_USE_TI_OMX
//	OMX_Init ();
#endif
	if(pStrmCtx->fAudEnable) {
		AUDIO_MIXER_T *pAmix = amixGetInstance();
		ConnCtxT * pConnDest = amixGetInputPortConnCtx(pAmix, pAudArgs->nSessionId);
		if(szAudCodecName && strcmp(szAudCodecName, "g711u") == 0){
			pStrmCtx->pAudChain = g711decchainCreate();
		}
#ifndef DBG_DO_NOT_USE_TI_OMX
		else if(szAudCodecName && strcmp(szAudCodecName, "aaclc") == 0)	{
			pStrmCtx->pAudChain = aacdecchainCreate();
		}
#endif
		pStrmCtx->pAudChain->SetInputConn(pStrmCtx->pAudChain, 0,pStrmCtx->pConnAudChainSrc);
		pStrmCtx->pAudChain->SetOutputConn(pStrmCtx->pAudChain, 0,pConnDest);
		if(fFileSource)
			pMasterClk = pStrmCtx->pAudChain->GetClkSrc(pStrmCtx->pAudChain);
	}

	if(pStrmCtx->fVidEnable) {
#ifndef DBG_DO_NOT_USE_TI_OMX
		pStrmCtx->pVidChain = vidchainCreate();
#else
		pStrmCtx->pVidChain = fakedecCreate();
#endif
		pStrmCtx->pVidChain->SetInputConn(pStrmCtx->pVidChain, 0,pStrmCtx->pConnVidChainSrc);
	}

	if(pStrmCtx->demuxComp) {
		 pStrmCtx->demuxComp->SetClkSrc(pStrmCtx->demuxComp, pMasterClk);
	}
	if(pStrmCtx->pVidChain) {
		pStrmCtx->pVidChain->SetClkSrc(pStrmCtx->pVidChain, pMasterClk);
	}
	if(pStrmCtx->pAudChain) {
		pStrmCtx->pAudChain->SetClkSrc(pStrmCtx->pAudChain, pMasterClk);
	}
	
	if(pStrmCtx->fVidEnable) {
		pStrmCtx->pVidChain->SetOption(pStrmCtx->pVidChain, DEC_CMD_SET_PARAMS, &pStrmCtx->args);
		pStrmCtx->pVidChain->Open(pStrmCtx->pVidChain, NULL);
	}
Exit:
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return res;
}                              

int decmainStartStreaming(void *pCtx)
{
	STREAMING_CTX *pStrmCtx = (STREAMING_CTX *)pCtx;
	if(pStrmCtx->fVidEnable && pStrmCtx->pVidChain) {
		pStrmCtx->pVidChain->Start(pStrmCtx->pVidChain);
	}

	if(pStrmCtx->fAudEnable && pStrmCtx->pAudChain) {
		IL_AUD_ARGS *args = &pStrmCtx->audArgs;
		pStrmCtx->pAudChain->SetOption(pStrmCtx->pAudChain, AUD_DEC_CMD_SET_PARAMS, args);
		pStrmCtx->pAudChain->Open(pStrmCtx->pAudChain, NULL);
		pStrmCtx->pAudChain->Start(pStrmCtx->pAudChain);
	}
	
	if(pStrmCtx->nVidSrcType == DEC_SRC_URI && pStrmCtx->srcComp && pStrmCtx->demuxComp) {
		pStrmCtx->srcComp->Start(pStrmCtx->srcComp);
		pStrmCtx->demuxComp->Start(pStrmCtx->demuxComp);
	}
}

int decmainStopStreaming(void *pCtx)
{
	void *ret_value;
	STREAMING_CTX *pStrmCtx = (STREAMING_CTX *)pCtx;
	IL_VID_ARGS *argsp = &pStrmCtx->args;
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	
	// Stop the file src
	if(pStrmCtx->nVidSrcType == DEC_SRC_URI) {
		pStrmCtx->srcComp->Stop(pStrmCtx->srcComp);
		pStrmCtx->demuxComp->Stop(pStrmCtx->demuxComp);
	}
	DBG_LOG(DBGLVL_SETUP, (":VidChain_Stop..."));
	pStrmCtx->pVidChain->Stop(pStrmCtx->pVidChain);
	if(pStrmCtx->fAudEnable) {
		DBG_LOG(DBGLVL_SETUP, (":AudChain_Stop..."));
		pStrmCtx->pAudChain->Stop(pStrmCtx->pAudChain);
	}

	if(pStrmCtx->nVidSrcType == DEC_SRC_URI) {
		if(pStrmCtx->srcComp) {
			pStrmCtx->srcComp->Close(pStrmCtx->srcComp);
			pStrmCtx->srcComp->Delete(pStrmCtx->srcComp);
			pStrmCtx->srcComp = NULL;
		}

		if(pStrmCtx->demuxComp) {
			pStrmCtx->demuxComp->Delete(pStrmCtx->demuxComp);
			pStrmCtx->demuxComp = NULL;
		}
		if(pStrmCtx->pConnCtxSrcToDemux){
			DeleteStrmConn(pStrmCtx->pConnCtxSrcToDemux);
		}
		if(pStrmCtx->pConnVidChainSrc){
			DeleteStrmConn(pStrmCtx->pConnVidChainSrc);
		}
		if(pStrmCtx->pConnAudChainSrc){
			DeleteStrmConn(pStrmCtx->pConnAudChainSrc);
		}
	}
	if(pStrmCtx->pAudChain) {
		pStrmCtx->pAudChain->Delete(pStrmCtx->pAudChain);
		pStrmCtx->pAudChain = NULL;
	}

	if(pStrmCtx->pVidChain) {
		pStrmCtx->pVidChain->Delete(pStrmCtx->pVidChain);
		pStrmCtx->pVidChain = NULL;
	}

	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return 0;
}                              

void *decmainCreateStream()
{
	STREAMING_CTX  *pStrmCtx = (STREAMING_CTX  *)malloc(sizeof(STREAMING_CTX));
	return pStrmCtx;
}

void decmainDeleteStream(void *pCtx)
{
	free(pCtx);
}