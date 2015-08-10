#ifndef __VIDEO_MIXER__
#define __VIDEO_MIXER__

#include <stdint.h>
#include <stdint.h>
#include "vdec_es_parser.h"
#include "semp.h"
#include "OMX_Types.h"
#include "OMX_Core.h"
#include "OMX_Component.h"
#include "dec_xport.h"
#include "dec_filesrc.h"
#include "dec_omx_chains_common.h"
#include "strmcomp.h"
#include "ilclient_common.h"
#include "venc_omx_chain.h"

#ifdef __cplusplus              /* required for headers that might */
extern "C"
{                               /* be compiled by a C++ compiler */
#endif


#define DEST_TYPE_NULL         2
#define DEST_TYPE_SWMOSAIC     1
#define DEST_TYPE_DISP         0

#define OUT_CHAIN_DEI_ENC_DISP 0
#define OUT_CHAIN_DEI_ENC      1
#define OUT_CHAIN_ENC          2
#define OUT_CHAIN_DISP         3

#define MAX_INPUT_WIDNOWS      64

typedef struct _VIDEO_MIXER_T
{
	IL_CLIENT_COMP_PRIVATE *vswmosaicILComp;
	OMX_CALLBACKTYPE       pCb;
	IL_CLIENT_COMP_PRIVATE *disILComp;
	OMX_HANDLETYPE         pctrlHandle;
	StrmCompIf             *pVEncChain;
	int            nInstanceCount;
	int            nNumWindows;
	int            displayId;

	int            nMixerOutWidth;
	int            nMixerOutHeight;

	int            nDispWidth;
	int            nDispHeight;
	int            nEncWidth;
	int            nEncHeight;
	int            nFrameRate;
	int            nBitRate;
	int            nDispStride;
	int            nSwMosaicOutputBuffers;
	int            nDisplayInputBuffers;
	int            nDestType;
	int            fEnableDisplay;
	WINDOW_PARAM_T *listInputWindows;
	WINDOW_PARAM_T WindOutput;
} VIDEO_MIXER_T;


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
	int nEncWidth,
	int nEncHeight,
	int nEncFramerate, 
	int nEncBitrate,
	int nGoplen,
	int nProflieAndLevel,
	int nDispStride,
	int nSwMosaicOutputBuffers,
	int nDisplayInputBuffers,
	int nDispId
	);
VIDEO_MIXER_T *vmixGetInstance();

int vmixInitResource(VIDEO_MIXER_T *pCtx);
int vmixDeinitResource(VIDEO_MIXER_T *pCtx);
int vmixConnectInputPort(VIDEO_MIXER_T *pCtx, int nPort, IL_CLIENT_COMP_PRIVATE *pPeer, int nPeerPort);
int vmixSetInportPeer(VIDEO_MIXER_T *pCtx, int nInputPort, IL_CLIENT_OUTPORT_PARAMS *peerPortParams);
int vmixSetStreamSink(VIDEO_MIXER_T *pCtx, ConnCtxT *pSink);

IL_CLIENT_OUTPORT_PARAMS *
	vmixGetInportParam(VIDEO_MIXER_T *pCtx, int nInputPort);

int vmixEnable(VIDEO_MIXER_T *pAppDAta);
int vmixDisable(VIDEO_MIXER_T *pAppDAta);
int vmixSetStateExec(VIDEO_MIXER_T *pCtx);
int vmixSetSateIdle(VIDEO_MIXER_T *pCtx);

int vmixStartStream(VIDEO_MIXER_T *pAppDAta);
int vmixStopStream(VIDEO_MIXER_T *pAppDAta);
int vmixDeinit(VIDEO_MIXER_T *pAppDAta);
int vmixGetDestinationType(VIDEO_MIXER_T *pCtx, int nInstanceId);
int vmixGetInputPortParam(VIDEO_MIXER_T *pCtx, int nSessionId, int *pnScalerOutWidth, int *pnScalerOutHeight, int *pnBufferCount);
int vmixSwMosaicPort(VIDEO_MIXER_T *pCtx, int nInstanceId);
#ifdef __cplusplus              /* matches __cplusplus construct above */
}
#endif

#endif
