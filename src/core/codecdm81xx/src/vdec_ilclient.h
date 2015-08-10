#ifndef __OMX_ILCLIENT_H__
#define __OMX_ILCLIENT_H__

/******************************************************************************\
 *      Includes
\******************************************************************************/
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
#include "dec_clock.h"
#include "ilclient_common.h"
#include "video_mixer.h"
/******************************************************************************/

#ifdef __cplusplus              /* required for headers that might */
extern "C"
{                               /* be compiled by a C++ compiler */
#endif

/* ========================================================================== */
typedef struct IL_Client
{
	OMX_HANDLETYPE pDecHandle, pDisHandle, pctrlHandle;
#ifdef EN_NF
	OMX_HANDLETYPE pNfHandle;
#endif
	OMX_COMPONENTTYPE *pComponent;
	OMX_CALLBACKTYPE pCb;
	OMX_STATETYPE eState;
	OMX_U8 eCompressionFormat;
	OMX_VIDEO_PARAM_AVCTYPE *pH264;

	OMX_COLOR_FORMATTYPE ColorFormat;
	OMX_U32 nDecWidth;
	OMX_U32 nDecHeight;
	OMX_U32 nDecStride;
	OMX_U32 nDispWidth;
	OMX_U32 nDispHeight;

	OMX_U32 nScalerOutWidth;
	OMX_U32 nScalerOutHeight;

	OMX_U32 nDeiPort2Width;
	OMX_U32 nDeiPort2Height;

	OMX_U32 nEncodedFrms;

	int     nSessionId;
	int     nSwMosaicPort;
	H264_ParsingCtx  H264Parser;

	MPEG4_ParsingCtx pcmpeg4;
	MPEG2_ParsingCtx pcmpeg2;
	OMX_VIDEO_CODINGTYPE  codingType;
	int                   useDemux;

	pthread_t          thrdidBitStreamTask;
	int                sync;
	int                nJitterLatency;
	int                nSyncMaxLateness;
	CLOCK_T            nSyncMaxWaitRunning;
	CLOCK_T            nSyncMaxWaitStartup;

	int                deinterlace;
	int                buffer;
	int                dropped;
	int                rendered;
	int                nDropFraction;
	int                nDecFrameRate;
	int                nDecInputBufferCount;
	int                nDecOutputBufferCount;
	int                nScalerInputBufferCount;
	int                nScalerOutputBufferCount;

	int                useDeiScalerAlways;	// Debug
	void *fieldBuf;

	IL_CLIENT_COMP_HOST_T  *vparseILComp;
	IL_CLIENT_COMP_PRIVATE *fakedecILComp;

	IL_CLIENT_COMP_PRIVATE *decILComp;
	IL_CLIENT_COMP_PRIVATE *scILComp;
	IL_CLIENT_COMP_PRIVATE *nfILComp;
	IL_CLIENT_GFX_PRIVATE gfx;

	pthread_t thrdidReaderTask;
	ConnCtxT              *pConnSrc;

	int IL_CLIENT_DECODER_INPUT_BUFFER_COUNT;
	int IL_CLIENT_DECODER_OUTPUT_BUFFER_COUNT;
	int IL_CLIENT_SCALAR_INPUT_BUFFER_COUNT;
	int IL_CLIENT_SCALAR_OUTPUT_BUFFER_COUNT;
	int IL_CLIENT_NF_INPUT_BUFFER_COUNT;
	int IL_CLIENT_NF_OUTPUT_BUFFER_COUNT;
	int IL_CLIENT_ENC_INPUT_BUFFER_COUNT; 


	int IL_CLIENT_DECODER_MAX_FRAME_RATE;
	int IL_COMPONENT_DEBUG_LEVEL;
	
	int  fStreaming;
	int  nUiCmd;
	void *pClk;

	CLOCK_T prev_clk;
	int     prev_frame_count;

	void (*strmCallback) (void *, int);
	int              nDestType;
	void             *pAppCtx;
	OMX_BOOL         bUseSwMosaic;
	VIDEO_MIXER_T  *pVidMix;
} IL_Client;

/**
 ** API
 */
#define DEC_CMD_SET_PARAMS		1

StrmCompIf *vidchainCreate();

void DumpHex(unsigned char *pData, int nSize);
void DumpAppStats( OMX_PTR ptrAppData);

#ifdef __cplusplus              /* matches __cplusplus construct above */
}
#endif

#endif

/*--------------------------------- end of file -----------------------------*/
