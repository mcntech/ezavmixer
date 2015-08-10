#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <xdc/std.h>

#include "ti/omx/interfaces/openMaxv11/OMX_Core.h"
#include "ti/omx/interfaces/openMaxv11/OMX_Component.h"
#include "OMX_TI_Common.h"
#include "OMX_Video.h"
#include "OMX_TI_Video.h"
#include "vcap_omx_chain.h"
#include <omx_venc.h>
#include <omx_vfpc.h>
#include <omx_vfdc.h>
#include <omx_vfcc.h>
#include <omx_ctrl.h>
#include <OMX_TI_Index.h>
#include <omx_vswmosaic.h>
#include "dec_platform_utils.h"
#include "video_mixer.h"
#include "semp.h"
#include "dbglog.h"

int SetEncodeDispChainOptions(VIDEO_MIXER_T *pCtx, StrmCompIf *pChain)
{
	VENC_CHAIN_ARGS   args;
	args.input_width = pCtx->nMixerOutWidth;
	args.input_height = pCtx->nMixerOutHeight;
	args.enc_width = pCtx->nEncWidth;
	args.enc_height = pCtx->nEncHeight;
	args.disp_width = pCtx->nDispWidth;
	args.disp_height = pCtx->nDispHeight;
	args.frame_rate = pCtx->nFrameRate;
	args.bit_rate = pCtx->nBitRate;
	args.input_buffers = pCtx->nSwMosaicOutputBuffers;
	args.output_buffers = pCtx->nDisplayInputBuffers;

	pChain->SetOption(pChain, VENC_CMD_SET_PARAMS, (char *)&args);
	return 0;
}

int DeinitEncode(StrmCompIf *pChain)
{
	return 0;
}

static VIDEO_MIXER_T *pvmixCtx = NULL;

VIDEO_MIXER_T *vmixInit(
	int nDestType, 
	int fEncode, 
	int fEnableDisplay,
	int nNumWindows,
	WINDOW_PARAM_T *listInputWindows,
	int nMixerOutWidth,
	int nMixerOutHeight,
	int nDispWidth,
	int nDispHeight,
	int nDispStride,
	int nEncWidth,
	int nEncHeight,
	int nEncFramerate, 
	int nEncBitrate,
	int nGoplen,
	int nProflieAndLevel,
	int nSwMosaicOutputBuffers,
	int nDisplayInputBuffers,
	int nDispId
	)
{
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	
	StrmCompIf * pEncChain = NULL;

	if(pvmixCtx == NULL) {
		pvmixCtx = (VIDEO_MIXER_T *)malloc(sizeof(VIDEO_MIXER_T));
		memset(pvmixCtx, 0x00, sizeof(VIDEO_MIXER_T));
		pvmixCtx->nInstanceCount = 0;
		pvmixCtx->nDestType = nDestType;
		pvmixCtx->nNumWindows = nNumWindows;

		pvmixCtx->nMixerOutWidth = nMixerOutWidth;
		pvmixCtx->nMixerOutHeight = nMixerOutHeight;

		pvmixCtx->nDispWidth = nDispWidth;
		pvmixCtx->nDispHeight = nDispHeight;
		pvmixCtx->fEnableDisplay = fEnableDisplay;
		// TODO: Get from UI
		pvmixCtx->nEncWidth = nEncWidth;
		pvmixCtx->nEncHeight = nEncHeight;
		pvmixCtx->nFrameRate = nEncFramerate;
		pvmixCtx->nBitRate = nEncBitrate;

		pvmixCtx->nDispStride = nDispStride;
		pvmixCtx->displayId = nDispId;

		pvmixCtx->nSwMosaicOutputBuffers = nSwMosaicOutputBuffers;
		pvmixCtx->nDisplayInputBuffers = nDisplayInputBuffers;
		pvmixCtx->pCb.EventHandler = IL_ClientCbEventHandler;
		pvmixCtx->pCb.EmptyBufferDone = IL_ClientCbEmptyBufferDone;
		pvmixCtx->pCb.FillBufferDone = IL_ClientCbFillBufferDone;

		DBG_PRINT ("####### OMX_Init ######\n");
		OMX_Init ();

		if(pvmixCtx->nDestType == DEST_TYPE_SWMOSAIC) {
			// Default configuration
			WINDOW_PARAM_T *pWndOut = &pvmixCtx->WindOutput;
			WINDOW_PARAM_T *pWndInput = malloc(nNumWindows * sizeof(WINDOW_PARAM_T));
			memcpy(pWndInput, listInputWindows, nNumWindows * sizeof(WINDOW_PARAM_T));
			pvmixCtx->listInputWindows = pWndInput;

			pWndOut->nWidth = nMixerOutWidth;      //nDispWidth;
			pWndOut->nHeight = nMixerOutHeight;     // nDispHeight;
			pWndOut->nStride = nMixerOutWidth * 2; //nDispStride;
			pWndOut->nBufferCount = nSwMosaicOutputBuffers;
			pvmixCtx->vswmosaicILComp = CreateILMultiInputCompWrapper(nNumWindows, 1, OMX_VSWMOSAIC_OUTPUT_PORT_START_INDEX, pWndInput,	nSwMosaicOutputBuffers, nMixerOutWidth * nMixerOutHeight * 2);
			if(pvmixCtx->fEnableDisplay) {
				pvmixCtx->disILComp = CreateILCompWrapper(1, 0, 0, 	nDisplayInputBuffers, nDispHeight * nDispWidth * 2, 0, 0);
			}
			if(fEncode) {
				pEncChain = vencchainCreate();
				SetEncodeDispChainOptions(pvmixCtx, pEncChain);
				pvmixCtx->pVEncChain = pEncChain;
			} else {
				if(pvmixCtx->disILComp) {
					IL_ClientConnectComponents (pvmixCtx->vswmosaicILComp, OMX_VSWMOSAIC_OUTPUT_PORT_START_INDEX, pvmixCtx->disILComp, OMX_VFDC_INPUT_PORT_START_INDEX);
				}
			}
		} else if (pvmixCtx->nDestType == DEST_TYPE_DISP) {
			WINDOW_PARAM_T *pWndInput = pvmixCtx->listInputWindows;
			pvmixCtx->disILComp = CreateILCompWrapper(1, 0, 0, 	pWndInput->nBufferCount, nDispHeight * nDispWidth * 2, 0, 0);
		}

		if(pvmixCtx->pVEncChain) {
			pvmixCtx->pVEncChain->Open(pvmixCtx->pVEncChain, NULL);
			pEncChain->SetInputConn2(pEncChain, 0, pvmixCtx->vswmosaicILComp, OMX_VSWMOSAIC_OUTPUT_PORT_START_INDEX + 0);
			if(pvmixCtx->disILComp) {
				pEncChain->SetOutputConn2(pEncChain, 0, pvmixCtx->disILComp, 0);
			}
		}

		vmixInitResource(pvmixCtx);

		if(vmixSetStateExec(pvmixCtx) != 0)
			goto EXIT;

		if(pvmixCtx->pVEncChain) {
			pvmixCtx->pVEncChain->Start(pvmixCtx->pVEncChain);
		}

		vmixStartStream(pvmixCtx);
	}
EXIT:
	pvmixCtx->nInstanceCount++;
	// TODO: ValidatepvmixCtx->nDestType == nDestType
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return pvmixCtx;
}

int vmixGetDestinationType(VIDEO_MIXER_T * pCtx, int nInstanceId)
{
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	int nPort = vmixSwMosaicPort(pCtx, nInstanceId);
	if(pCtx->nNumWindows == 0 || nPort < 0 || nPort >= pCtx->nNumWindows){
		return DEST_TYPE_NULL;
	} else{
		return pCtx->nDestType;
	}
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return 0;
}

WINDOW_PARAM_T *vmixGetWindowForStrmSrc(VIDEO_MIXER_T * pCtx, int nStrmSrc)
{
	int i;
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	for (i = 0; i < pCtx->nNumWindows; i++) {
		WINDOW_PARAM_T *pWnd = &pCtx->listInputWindows[i];
		if(pWnd->nStrmSrc == nStrmSrc)
			return pWnd;
	}
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return NULL;
}

int vmixGetWindowIdFromStrmSrc(VIDEO_MIXER_T * pCtx, int nStrmSrc)
{
	int i;
	int nWindId = -1;
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	for (i = 0; i < pCtx->nNumWindows; i++) {
		WINDOW_PARAM_T *pWnd = &pCtx->listInputWindows[i];
		if(pWnd->nStrmSrc == nStrmSrc) {
			nWindId =  i;
		}
	}
	DBG_LOG(DBGLVL_TRACE, ("Window ID %d", nWindId));
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return nWindId;
}

int vmixGetInputPortParam(VIDEO_MIXER_T * pCtx, int nSessionId, int *pnScelerOutWidth, int *pnScalerOutHeight, int *pnBufferCount)
{
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	if(pCtx->nDestType == DEST_TYPE_SWMOSAIC) {
		WINDOW_PARAM_T *pWnd = vmixGetWindowForStrmSrc(pCtx, nSessionId);
		*pnScelerOutWidth = pWnd->nWidth;
		*pnScalerOutHeight = pWnd->nHeight;
		*pnBufferCount = pWnd->nBufferCount;
	} else {
		WINDOW_PARAM_T *pWnd = &pCtx->listInputWindows[0];
		*pnScelerOutWidth = pCtx->nDispWidth;
		*pnScalerOutHeight = pWnd->nBufferCount;
	}
	DBG_LOG(DBGLVL_SETUP, ("SessionId=%d w=%d h=%d", nSessionId, *pnScelerOutWidth, *pnScalerOutHeight));
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return 0;
}

int vmixSwMosaicPort(VIDEO_MIXER_T * pCtx, int nInstanceId)
{
	int nPort = -1;
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	if(pCtx->nNumWindows >= 1 ){
		nPort = vmixGetWindowIdFromStrmSrc(pCtx, nInstanceId);
	}
	DBG_LOG(DBGLVL_SETUP, ("port=%d for nInstanceId=%d", nPort, nInstanceId));
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return nPort;
}

VIDEO_MIXER_T *vmixGetInstance()
{
	return pvmixCtx;
}


int vmixSetInportPeer(VIDEO_MIXER_T *pCtx, int nInputPort, IL_CLIENT_OUTPORT_PARAMS *peerPortParams)
{
	if(pCtx->nDestType == DEST_TYPE_SWMOSAIC) {
		IL_CLIENT_COMP_PRIVATE *pComp = pCtx->vswmosaicILComp;
		IL_CLIENT_INPORT_PARAMS  *inPortParamsPtr = pComp->inPortParams + nInputPort;
		WINDOW_PARAM_T *pWnd = &pCtx->listInputWindows[nInputPort];

		pWnd->nBufferCount = peerPortParams->nBufferCountActual;
		// Overwriting the nBufferCountActual value set in CreateILCompWrapper
		// TODO: Cleanup
		inPortParamsPtr->nBufferCountActual = peerPortParams->nBufferCountActual;
	} else if(pCtx->nDestType == DEST_TYPE_DISP) {
		if(pvmixCtx->disILComp) {
			compSetInportAllocationType(pCtx->disILComp, 0, 1);
		}
	}
}

IL_CLIENT_OUTPORT_PARAMS *vmixGetInportParam(VIDEO_MIXER_T *pCtx, int nInputPort)
{
	if(pCtx->nDestType == DEST_TYPE_SWMOSAIC) {
		IL_CLIENT_COMP_PRIVATE *pComp = pCtx->vswmosaicILComp;
		IL_CLIENT_INPORT_PARAMS  *inPortParamsPtr = pComp->inPortParams + nInputPort;
		return inPortParamsPtr;
	} else if(pCtx->nDestType == DEST_TYPE_DISP && pvmixCtx->disILComp) {
		IL_CLIENT_COMP_PRIVATE *pComp = pCtx->disILComp;
		IL_CLIENT_INPORT_PARAMS  *inPortParamsPtr = pComp->inPortParams;
		return inPortParamsPtr;
	}
	return NULL;
}


int vmixInitResource(VIDEO_MIXER_T *pCtx)
{
	int j;
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	int res = 0;
	DBG_LOG(DBGLVL_TRACE, ("Enter"));

	if(pCtx->nDestType == DEST_TYPE_SWMOSAIC) {
		IL_ClientInitSwMosaic(pCtx->vswmosaicILComp, pCtx->nNumWindows, pCtx->listInputWindows, &pCtx->WindOutput, &pCtx->pCb);
		if(pCtx->disILComp) {
			compSetInportAllocationType(pCtx->disILComp, 0, 1);
		}
	}
	if(pCtx->disILComp) {
		IL_CLIENT_INPORT_PARAMS  *inPortParamsPtr = pCtx->disILComp->inPortParams;
		IL_CLIENT_OUTPORT_PARAMS *peerPortParams = compGetConnectedCompOutPortParams( pCtx->disILComp, 0);
		int nBufferCountActual;
		if(peerPortParams) {
			 nBufferCountActual = peerPortParams->nBufferCountActual;
		} else {
			nBufferCountActual = inPortParamsPtr->nBufferCountActual;
		}
		if(displayInit(pCtx->displayId, pCtx, pCtx->disILComp, &pCtx->pCb, pCtx->nDispWidth, pCtx->nDispHeight,nBufferCountActual, &pCtx->pctrlHandle) != 0) {
			res = -1;; goto EXIT;
		} 
	}
	
	if(pCtx->nDestType == DEST_TYPE_SWMOSAIC) {
		if(compInitResource(pCtx->vswmosaicILComp, 0) != 0) {
			res = -1; goto EXIT;
		} 
	}
	TRACE_PROGRESS
	if(pvmixCtx->pVEncChain) {
		pvmixCtx->pVEncChain->AllocateResource(pvmixCtx->pVEncChain);
	}
	TRACE_PROGRESS
	if(pCtx->disILComp) {
		if(compInitResource(pCtx->disILComp, pCtx->pctrlHandle) != 0) {
			goto EXIT;
		} 
	}
EXIT:
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return res;
}

int vmixDeinitResource(VIDEO_MIXER_T *pCtx)
{
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	if(pCtx->nDestType == DEST_TYPE_SWMOSAIC) {
		compDeinitResource(pCtx->vswmosaicILComp, 0);
	}

	if(pCtx->disILComp)
		compDeinitResource(pCtx->disILComp, pCtx->pctrlHandle);

	if(pCtx->pVEncChain) {
		DeinitEncode(pCtx->pVEncChain);
	}
	if(pvmixCtx->pVEncChain) {
		pvmixCtx->pVEncChain->Delete(pvmixCtx->pVEncChain);
	}

	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return 0;
}

int vmixConnectInputPort(VIDEO_MIXER_T *pCtx, int nPort, IL_CLIENT_COMP_PRIVATE *pPeer, int nPeerPort)
{
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	if(pCtx->nDestType == DEST_TYPE_SWMOSAIC) {
		IL_ClientConnectComponents (pPeer, nPeerPort,	pCtx->vswmosaicILComp,  OMX_VSWMOSAIC_INPUT_PORT_START_INDEX + nPort);
	} else if(pCtx->nDestType == DEST_TYPE_DISP && pCtx->disILComp) {
		IL_ClientConnectComponents (pPeer, nPeerPort,	pCtx->disILComp,  OMX_VFDC_INPUT_PORT_START_INDEX);
	}
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	 return 0;
}

int vmixSetStreamSink(VIDEO_MIXER_T *pCtx, ConnCtxT *pSink)
{
	if(pvmixCtx->pVEncChain) {
		pvmixCtx->pVEncChain->SetOutputConn(pvmixCtx->pVEncChain, 0, pSink);
	}
}

int vmixEnable(VIDEO_MIXER_T *pCtx)
{

}

int vmixDisable(VIDEO_MIXER_T *pCtx)
{

}

int vmixSetStateExec(VIDEO_MIXER_T *pCtx)
{
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	if(pCtx->disILComp) {
		if(compSetStateExec(pCtx->disILComp, pCtx->pctrlHandle) != 0)
			return -1;
	}

	if(pCtx->nDestType == DEST_TYPE_SWMOSAIC) {
		if(compSetStateExec(pCtx->vswmosaicILComp, 0) != 0)
			return -1;
	}
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return 0;
}
int vmixSetSateIdle(VIDEO_MIXER_T *pCtx)
{
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	if(pCtx->nDestType == DEST_TYPE_SWMOSAIC) {
		if(compSetSateIdle(pCtx->vswmosaicILComp, 0) != 0)
			return -1;
	}
	if(pCtx->disILComp) {
		if(compSetSateIdle(pCtx->disILComp, pCtx->pctrlHandle) != 0)
			return -1;
	}

	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return 0;
}

int vmixStartStream(VIDEO_MIXER_T *pCtx)
{
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	if(pCtx->nDestType == DEST_TYPE_SWMOSAIC) {
		compStartStream(pCtx->vswmosaicILComp, (ILC_StartFcnPtr) IL_ClientConnInConnOutTask);
	} 
	if(pCtx->disILComp) {
		compStartStream(pCtx->disILComp, (ILC_StartFcnPtr) IL_DisplayTask);
	}

	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return 0;
}

int vmixStopStream(VIDEO_MIXER_T *pCtx)
{
	if(pCtx->nDestType == DEST_TYPE_SWMOSAIC) {
		compStopStream(pCtx->vswmosaicILComp);
	} 
	if(pCtx->disILComp){
		compStopStream(pCtx->disILComp);
	}
	if(pvmixCtx->pVEncChain) {
		pvmixCtx->pVEncChain->Stop(pvmixCtx->pVEncChain);
	}
	return 0;
}

int vmixDeinit(VIDEO_MIXER_T *pCtx)
{
	DBG_LOG(DBGLVL_TRACE, ("Enter"));

	pvmixCtx->nInstanceCount--;
	if(pvmixCtx->nInstanceCount <= 0) {
		vmixStopStream(pvmixCtx);
		vmixSetSateIdle(pvmixCtx);
		vmixDeinitResource(pvmixCtx);
		DBG_PRINT ("####### OMX_Deinit ######\n");
		OMX_Deinit();
		if(pvmixCtx->listInputWindows)
			free(pvmixCtx->listInputWindows);
		free(pvmixCtx);
		pvmixCtx = NULL;
	}

	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return 0;
}
