/*
 *  Copyright (c) 2010-2011, Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  *  Neither the name of Texas Instruments Incorporated nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  Contact information for paper mail:
 *  Texas Instruments
 *  Post Office Box 655303
 *  Dallas, Texas 75265
 *  Contact information:
 *  http://www-k.ext.ti.com/sc/technical-support/product-information-centers.htm?
 *  DCMP=TIHomeTracking&HQS=Other+OT+home_d_contact
 *  ============================================================================
 *
 */
/**
 *******************************************************************************
 *  @file  adec_ilclient.c
 *  @brief This file contains all Functions related to Test Application
 *
 *         This is the example IL Client support to create, configure & execute
 *         adec omx-component using standard non-tunneling mode
 *
 *  @rev 1.0
 *******************************************************************************
 */

/*******************************************************************************
*                             Compilation Control Switches
*******************************************************************************/
/* None */

/*******************************************************************************
*                             INCLUDE FILES
*******************************************************************************/

/*--------------------- system and platform files ----------------------------*/
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <xdc/std.h>
#include <xdc/runtime/Assert.h>
#include <xdc/runtime/Timestamp.h>
#ifdef CODEC_MP3DEC
#include <ti/sdo/codecs/mp3dec/imp3dec.h>
#endif
#include <ti/sdo/codecs/aaclcdec/iaacdec.h>
/*-------------------------program files -------------------------------------*/
#include "ti/omx/interfaces/openMaxv11/OMX_Audio.h"
#include "ti/omx/interfaces/openMaxv11/OMX_Core.h"
#include "ti/omx/interfaces/openMaxv11/OMX_Component.h"
#include "OMX_TI_Common.h"
#include "timm_osal_trace.h"
#include "timm_osal_interfaces.h"
#include "omx_adec.h"
#include <alsa/asoundlib.h>
#include "dec_clock.h"
#include "vdec_ilclient.h"
#include "adec_ilclient_utils.h"
#include "dbglog.h"
#include "dec_clock.h"
#include "adec_ilclient.h"
#include "strmcomp.h"
#include "adec_omx_chain.h"
#include "dbglog.h"
#include "audio_mixer.h"
#include "aac_dec_chain.h"

//#define DBG_DUMP
/*******************************************************************************
 * EXTERNAL REFERENCES NOTE : only use if not found in header file
*******************************************************************************/

/****************************************************************
 * DEFINES
 ****************************************************************/

/** Event definition to indicate input buffer consumed */
#define ADEC_DECODER_INPUT_READY 1

/** Event definition to indicate output buffer consumed */
#define ADEC_DECODER_OUTPUT_READY   2

/** Event definition to indicate error in processing */
#define ADEC_DECODER_ERROR_EVENT 4

/** Event definition to indicate End of stream */
#define ADEC_DECODER_END_OF_STREAM 8

#define ADEC_STATETRANSITION_COMPLETE 16

#define ADEC_PORTCONFIGURATION_COMPLETE 32

#define MAX_AUD_DEV_NAME_SIZE      256

/****************************************************************
 * GLOBALS
 ****************************************************************/

static TIMM_OSAL_PTR pSem_Events = NULL;
static TIMM_OSAL_PTR myEvent;
static TIMM_OSAL_PTR ADEC_CmdEvent;

/** Number of input buffers in the ADEC Decoder IL Client */
#define NUM_OF_IN_BUFFERS 1

/** Number of output buffers in the ADEC Decoder IL Client */
#define NUM_OF_OUT_BUFFERS 1


/** Macro to initialize memset and initialize the OMX structure */
#define OMX_ADEC_TEST_INIT_STRUCT_PTR(_s_, _name_)       \
 memset((_s_), 0x0, sizeof(_name_)); \
    (_s_)->nSize = sizeof(_name_);              \
    (_s_)->nVersion.s.nVersionMajor = 0x1;      \
    (_s_)->nVersion.s.nVersionMinor = 0x1;      \
    (_s_)->nVersion.s.nRevision  = 0x0;       \
    (_s_)->nVersion.s.nStep   = 0x0;

#ifndef timersub
#define	timersub(a, b, result) \
do { \
	(result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
	(result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
	if ((result)->tv_usec < 0) { \
		--(result)->tv_sec; \
		(result)->tv_usec += 1000000; \
	} \
} while (0)
#endif

typedef struct _THREAD_POS_T
{
	int nSrcLine;
	char szDescript[128];
} THREAD_POS_T;

#define SET_THREAD_POS(p,s,l) { p->threadPos.nSrcLine = l; strncpy(p->threadPos.szDescript, s, 63);}
#define SHOW_THREAD_POS(p) { fprintf(stderr, "Thread Pos:%s %d\n",p->threadPos.szDescript, p->threadPos.nSrcLine);}

/* ========================================================================== */
/** ADEC_CHAIN_T is the structure definition for the ADEC Decoder IL Client
*
* @param pHandle               OMX Handle
* @param pComponent            Component Data structure
* @param pCb                   Callback function pointer
* @param eState                Current OMX state
* @param pInPortDef            Structure holding input port definition
* @param pOutPortDef           Structure holding output port definition
* @param eCompressionFormat    Format of the input data
* @param pInBuff               Input Buffer pointer
* @param pOutBuff              Output Buffer pointer
* @param IpBuf_Pipe            Input Buffer Pipe
* @param OpBuf_Pipe            Output Buffer Pipe
* @param fIn                   File pointer of input file
* @param fOut                  Output file pointer
* @param nDecodedFrm           Total number of decoded frames
* @param fStreaming           Flag to indicate Stop further processing
*/
/* ========================================================================== */
typedef struct ADEC_CHAIN_T
{
	OMX_HANDLETYPE pHandle;
	OMX_COMPONENTTYPE *pComponent;
	OMX_CALLBACKTYPE *pCb;
	OMX_STATETYPE eState;
	OMX_PARAM_PORTDEFINITIONTYPE *pInPortDef;
	OMX_PARAM_PORTDEFINITIONTYPE *pOutPortDef;
	OMX_U8 eCompressionFormat;
	OMX_BUFFERHEADERTYPE *pInBuff[NUM_OF_IN_BUFFERS];
	OMX_BUFFERHEADERTYPE *pOutBuff[NUM_OF_OUT_BUFFERS];
	OMX_PTR IpBuf_Pipe;
	OMX_PTR OpBuf_Pipe;
	ConnCtxT *pConnSrc;

	int          fEoS;
	unsigned long long crnt_aud_pts;
	unsigned long long start_aud_pts;
	unsigned long long crnt_sample;
	


	int                buffer_size;

	OMX_U32 nDecodedFrms;
	OMX_U32 nDroppedFrms;
	OMX_U32 fStreaming;
#ifdef DBG_DUMP
	int     fDbgSave;
#endif
	int     fClkSrc;
	void    *pClk;
	int     nUiCmd;

	int   codec_name[32];
	int   audRawSampleRate;
	int   aacRawFormat;

	int        sync;
	CLOCK_T    nJitterLatency;
	CLOCK_T    nSyncMaxWait;
	CLOCK_T    nSyncMaxWaitRunning;
	CLOCK_T    nSyncMaxWaitStartup;
	CLOCK_T    nSyncMaxLateness;
	int        max_input_pkt_size;	// Max input size. 188 for TS Demux, (8*1536) for SDI AC3
	int        dec_input_buffer_size;
	int        dec_output_buffer_size;
	int        alsa_output_buffer_size;

	char       device[MAX_AUD_DEV_NAME_SIZE];
	int        fPcmPassthru;
	int        fEnableFormatChange;
	int        adecchainPrimed;

	THREAD_POS_T threadPos;
	ConnCtxT   *pConnSink;
	int         nMixerPort;
} ADEC_CHAIN_T;

/*---------------------function prototypes -----------------------------------*/
OMX_ERRORTYPE ADEC_SetParamPortDefinition(
				OMX_HANDLETYPE handle, 
				Int32 decType,
				Int aacRawFormat, 
				Int audRawSampleRate,
				int nInputBufferSize,
				int nOutputBufferSize
				);

static void DumpCtx(ADEC_CHAIN_T *pCtx)
{
	fprintf(stderr,"****** Aud Dec Context ******\n");
	fprintf(stderr,"nJitterLatency\t=%d\n", pCtx->nJitterLatency);
	fprintf(stderr,"nSyncMaxWait\t=%d\n", pCtx->nSyncMaxWait);
	fprintf(stderr,"nSyncMaxWaitRunning\t=%d\n", pCtx->nSyncMaxWaitRunning);
	fprintf(stderr,"nSyncMaxWaitStartup\t=%d\n", pCtx->nSyncMaxWaitStartup);
	fprintf(stderr,"nSyncMaxLateness\t=%d\n", pCtx->nSyncMaxLateness);

	fprintf(stderr,"max_input_pkt_size\t=%d\n", pCtx->max_input_pkt_size);
	fprintf(stderr,"dec_input_buffer_size\t=%d\n", pCtx->dec_input_buffer_size);
	fprintf(stderr,"dec_output_buffer_size\t=%d\n", pCtx->dec_output_buffer_size);
	fprintf(stderr,"alsa_output_buffer_size\t=%d\n", pCtx->alsa_output_buffer_size);
	
	
	fprintf(stderr,"eCompressionFormat\t=0x%x\n", pCtx->eCompressionFormat);
	fprintf(stderr,"fPcmPassthru\t=%d\n", pCtx->fPcmPassthru);
}

/* ========================================================================== */
/**
* ADEC_AllocateResources() : Allocates the resources required for Audio
* Decoder.
*
*
* @param pAppData   : Pointer to the application data
*
*  @return
*  OMX_ErrorNone = Successful
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */
static OMX_ERRORTYPE ADEC_AllocateResources(ADEC_CHAIN_T * pAppData)
{
  OMX_U32 retval;
  OMX_ERRORTYPE eError = OMX_ErrorNone;

  /* Creating IL client specific dtaa structure to maintained at appliaction/
     IL client */
  /* callback structure , standard OpenMax structure */
  pAppData->pCb =
    (OMX_CALLBACKTYPE *) TIMM_OSAL_Malloc (sizeof (OMX_CALLBACKTYPE),
                                           TIMM_OSAL_TRUE, 0,
                                           TIMMOSAL_MEM_SEGMENT_EXT);
  if (!pAppData->pCb)
  {
    eError = OMX_ErrorInsufficientResources;
    goto EXIT;
  }

  pAppData->pInPortDef =
    (OMX_PARAM_PORTDEFINITIONTYPE *)
    TIMM_OSAL_Malloc (sizeof (OMX_PARAM_PORTDEFINITIONTYPE), TIMM_OSAL_TRUE,
                      0, TIMMOSAL_MEM_SEGMENT_EXT);
  if (!pAppData->pInPortDef)
  {
    eError = OMX_ErrorInsufficientResources;
    goto EXIT;
  }

  pAppData->pOutPortDef =
    (OMX_PARAM_PORTDEFINITIONTYPE *)
    TIMM_OSAL_Malloc (sizeof (OMX_PARAM_PORTDEFINITIONTYPE), TIMM_OSAL_TRUE,
                      0, TIMMOSAL_MEM_SEGMENT_EXT);
  if (!pAppData->pOutPortDef)
  {
    eError = OMX_ErrorInsufficientResources;
    goto EXIT;
  }

  /* Create a pipes for Input and Output Buffers.. used to queue data from the
     callback. */
  retval =
    TIMM_OSAL_CreatePipe (&(pAppData->IpBuf_Pipe),
                          sizeof (OMX_BUFFERHEADERTYPE *) * NUM_OF_IN_BUFFERS,
                          sizeof (OMX_BUFFERHEADERTYPE *), OMX_TRUE);
  if (retval != 0)
  {
    DBG_LOG(DBGLVL_ERROR,  ("Error: TIMM_OSAL_CreatePipe failed to open"));
    eError = OMX_ErrorContentPipeCreationFailed;
    goto EXIT;
  }

  retval =
    TIMM_OSAL_CreatePipe (&(pAppData->OpBuf_Pipe),
                             sizeof(OMX_BUFFERHEADERTYPE *) *
                             NUM_OF_OUT_BUFFERS, sizeof(OMX_BUFFERHEADERTYPE *),
                             OMX_TRUE);
  if (retval != 0)
  {
    DBG_LOG(DBGLVL_ERROR,  ("Error: TIMM_OSAL_CreatePipe failed to open"));
    eError = OMX_ErrorContentPipeCreationFailed;
    goto EXIT;
  }

EXIT:

  return eError;
}

/* ========================================================================== */
/**
* ADEC_FreeResources() : Free the resources allocated for Audio
* Decoder.
*
*
* @param pAppData   : Pointer to the application data
*
*  @return
*  OMX_ErrorNone = Successful
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */
static void ADEC_FreeResources(ADEC_CHAIN_T * pAppData)
{
  /* freeing up IL client alloacted data structures */
  if (pAppData->pCb) {
    TIMM_OSAL_Free (pAppData->pCb);
  }

  if (pAppData->pInPortDef) {
    TIMM_OSAL_Free (pAppData->pInPortDef);
  }

  if (pAppData->pOutPortDef) {
    TIMM_OSAL_Free (pAppData->pOutPortDef);
  }

  if (pAppData->IpBuf_Pipe) {
    TIMM_OSAL_DeletePipe (pAppData->IpBuf_Pipe);
  }

  if (pAppData->OpBuf_Pipe) {
    TIMM_OSAL_DeletePipe (pAppData->OpBuf_Pipe);
  }

  return;
}

/* ========================================================================== */
/**
* ADEC_GetDecoderErrorString() : Function to map the OMX error enum to string
*
* @param error   : OMX Error type
*
*  @return
*  String conversion of the OMX_ERRORTYPE
*
*/
/* ========================================================================== */
static OMX_STRING ADEC_GetDecoderErrorString(OMX_ERRORTYPE error)
{
  OMX_STRING errorString;

  /* used for printing purpose */
  switch (error)
  {
    case OMX_ErrorNone:
      errorString = "OMX_ErrorNone";
      break;
    case OMX_ErrorInsufficientResources:
      errorString = "OMX_ErrorInsufficientResources";
      break;
    case OMX_ErrorUndefined:
      errorString = "OMX_ErrorUndefined";
      break;
    case OMX_ErrorInvalidComponentName:
      errorString = "OMX_ErrorInvalidComponentName";
      break;
    case OMX_ErrorComponentNotFound:
      errorString = "OMX_ErrorComponentNotFound";
      break;
    case OMX_ErrorInvalidComponent:
      errorString = "OMX_ErrorInvalidComponent";
      break;
    case OMX_ErrorBadParameter:
      errorString = "OMX_ErrorBadParameter";
      break;
    case OMX_ErrorNotImplemented:
      errorString = "OMX_ErrorNotImplemented";
      break;
    case OMX_ErrorUnderflow:
      errorString = "OMX_ErrorUnderflow";
      break;
    case OMX_ErrorOverflow:
      errorString = "OMX_ErrorOverflow";
      break;
    case OMX_ErrorHardware:
      errorString = "OMX_ErrorHardware";
      break;
    case OMX_ErrorInvalidState:
      errorString = "OMX_ErrorInvalidState";
      break;
    case OMX_ErrorStreamCorrupt:
      errorString = "OMX_ErrorStreamCorrupt";
      break;
    case OMX_ErrorPortsNotCompatible:
      errorString = "OMX_ErrorPortsNotCompatible";
      break;
    case OMX_ErrorResourcesLost:
      errorString = "OMX_ErrorResourcesLost";
      break;
    case OMX_ErrorNoMore:
      errorString = "OMX_ErrorNoMore";
      break;
    case OMX_ErrorVersionMismatch:
      errorString = "OMX_ErrorVersionMismatch";
      break;
    case OMX_ErrorNotReady:
      errorString = "OMX_ErrorNotReady";
      break;
    case OMX_ErrorTimeout:
      errorString = "OMX_ErrorTimeout";
      break;
    default:
      errorString = "<unknown>";
  }

  return errorString;
}

static int AudParserReadData(ADEC_CHAIN_T *pAppData, unsigned char *pData, int lenData)
{
	int nBytesCopied = 0;
	int nMaxPktSize = pAppData->max_input_pkt_size;
#if EN_AUD_PROF_BUFF_READ
	struct timeval time1, time2, diff;
	gettimeofday(&time1, 0);
#endif

	while(nBytesCopied < lenData - nMaxPktSize) {
		int ret = 0;
		unsigned long long ullPts = 0;
		unsigned long ulFlags;
		DBG_LOG(DBGLVL_FRAME,  (" Waitfor buffer..."));
		while(pAppData->pConnSrc->IsEmpty(pAppData->pConnSrc) && pAppData->nUiCmd != STRM_CMD_STOP){
			usleep(1000);
		}
		DBG_LOG(DBGLVL_FRAME,  (" Waitfor buffer:Done"));
		if(pAppData->nUiCmd == STRM_CMD_STOP) {
			ulFlags = OMX_BUFFERFLAG_EOS;
		} else {
			ret = pAppData->pConnSrc->Read(pAppData->pConnSrc, pData + nBytesCopied, nMaxPktSize, &ulFlags, &ullPts);
			DBG_LOG(DBGLVL_FRAME,  (" Read bytes=%d ulFlags=0x%0x ullPts=%lld", ret, ulFlags, ullPts));
			if(nBytesCopied == 0/*first buffer */)
				pAppData->crnt_aud_pts = ullPts;

#ifdef DBG_DUMP
			if (pAppData->fDbgSave)	{
				DBG_LOG(DBGLVL_FRAME,  (" Write to file %d", ret));
				fwrite(pData + nBytesCopied, 1, ret, pAppData->fDbgSave);
			}
#endif
		}
		if(ulFlags & OMX_BUFFERFLAG_EOS) {
			DBG_LOG(DBGLVL_TRACE, ("AudParserReadData: End of file"));
			pAppData->fEoS = 1;
			break;
		}
		//Process dynamic codec fromat change
		// This code should be active only for SDI capture due to OMX_AUD_DYNAMIC_FMT_MASK set only by SDI alsa source
		if((ulFlags & OMX_AUD_DYNAMIC_FMT_MASK) == OMX_AUD_DYNAMIC_FMT_AC3) {
			// Make pcm passthrough off
			pAppData->fPcmPassthru = 0;
		} else if((ulFlags & OMX_AUD_DYNAMIC_FMT_MASK) == OMX_AUD_DYNAMIC_FMT_PCM){
			// Make pcm passthrough on
			pAppData->fPcmPassthru = 1;
		} else {
			// No dynamic format change
		}
		nBytesCopied += ret; 
	}
//Exit:

#if EN_AUD_PROF_BUFF_READ
	gettimeofday(&time2, 0);
	timersub(&time2, &time1, &diff);
	DBG_PRINT("<AbuffWait%.3f ms>",diff.tv_sec * 1000 + diff.tv_usec / 1000.0);
#endif

	return nBytesCopied;
}

/* ========================================================================== */
/**
* ADEC_FillData() : Function to fill the input buffer with data.
* This function currently reads the entire file into one single memory chunk.
* May require modification to support bigger file sizes.
*
*
* @param pAppData   : Pointer to the application data
* @param pBuf       : Pointer to the input buffer
*
*  @return
*  OMX_ErrorNone = Successful
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */
static OMX_U32 ADEC_FillData(ADEC_CHAIN_T *pAppData,
                             OMX_BUFFERHEADERTYPE *pBuf)
{
	OMX_U32 nRead = 0;
	int i;
	pBuf->nFilledLen = 0;
	pBuf->nOffset = 0;
	pBuf->nInputPortIndex = OMX_AUDDEC_INPUT_PORT;

	for(i = 0; i < 2; i++) {
		nRead = AudParserReadData(pAppData, pBuf->pBuffer + pBuf->nFilledLen, pBuf->nAllocLen - pBuf->nFilledLen);
		pBuf->nFilledLen += nRead;
	}
	// Do we need this ??
	pBuf->nAllocLen = pAppData->pInPortDef->nBufferSize;
	return pBuf->nFilledLen;
}

/* ========================================================================== */
/**
* ADEC_WaitForState() : This method will wait for the component to get
* to the correct state.
*
* @param pHandle        : Handle to the component
* @param DesiredState   : Desired
*
*  @return
*  OMX_ErrorNone = Successful
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */
static OMX_ERRORTYPE ADEC_WaitForState(OMX_HANDLETYPE * pHandle,
                                          OMX_STATETYPE DesiredState)
{
  OMX_ERRORTYPE eError = OMX_ErrorNone;
  TIMM_OSAL_U32 uRequestedEvents, pRetrievedEvents;
  TIMM_OSAL_ERRORTYPE retval;

  /* Wait for an event, which would be triggered through callback function */
  uRequestedEvents = (ADEC_STATETRANSITION_COMPLETE );
  retval = TIMM_OSAL_EventRetrieve(ADEC_CmdEvent, uRequestedEvents, TIMM_OSAL_EVENT_OR_CONSUME, &pRetrievedEvents, TIMM_OSAL_SUSPEND);

  if (TIMM_OSAL_ERR_NONE != retval) {
    TIMM_OSAL_Trace ("\nError in EventRetrieve !\n");
    eError = OMX_ErrorInsufficientResources;
    goto EXIT;
  }

  if (pRetrievedEvents & ADEC_DECODER_ERROR_EVENT) {
    eError = OMX_ErrorUndefined;
  }
  else {
    eError = OMX_ErrorNone;
  }

  EXIT:
  return eError;
}

/* ========================================================================== */
/**
* ADEC_WaitForPortConfig() : This method will wait for the component to get
* to the correct port configuration.
*
* @param pHandle        : Handle to the component
*
*  @return
*  OMX_ErrorNone = Successful
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */
static OMX_ERRORTYPE ADEC_WaitForPortConfig(OMX_HANDLETYPE * pHandle)
{
	//OMX_STATETYPE CurState = OMX_StateInvalid;
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	//OMX_U32 nCnt = 0;
	//OMX_COMPONENTTYPE *pComponent = (OMX_COMPONENTTYPE *) pHandle;
	TIMM_OSAL_U32 uRequestedEvents, pRetrievedEvents;
	TIMM_OSAL_ERRORTYPE retval;

	/* Wait for an event */
	uRequestedEvents = (ADEC_PORTCONFIGURATION_COMPLETE );
	retval = TIMM_OSAL_EventRetrieve(ADEC_CmdEvent, uRequestedEvents, TIMM_OSAL_EVENT_OR_CONSUME, &pRetrievedEvents, TIMM_OSAL_SUSPEND);
	if (TIMM_OSAL_ERR_NONE != retval) {
		TIMM_OSAL_Trace("\nError in EventRetrieve !\n");
		eError = OMX_ErrorInsufficientResources;
		goto EXIT;
	}

	if (pRetrievedEvents & ADEC_DECODER_ERROR_EVENT) {
		eError = OMX_ErrorUndefined;
	}
	else {
		eError = OMX_ErrorNone;
	}

EXIT:
	return eError;
}

/* ========================================================================== */
/**
* ADEC_ChangePortSettings() : This method will perform output Port
* settings change
*
* @param pHandle        : Handle to the component
*
*  @return
*  OMX_ErrorNone = Successful
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */
static OMX_ERRORTYPE ADEC_ChangePortSettings(ADEC_CHAIN_T * pAppData)
{

  TIMM_OSAL_ERRORTYPE retval;
  OMX_ERRORTYPE eError = OMX_ErrorNone;
  OMX_U32 i;

  /* in case we need to change the port setting, while executing, port needs to
     be disabled, parameters needs to be changed, and buffers would be
     alloacted as new sizes and port would be enabled */

  eError =
    OMX_SendCommand (pAppData->pHandle, OMX_CommandPortDisable,
                     pAppData->pOutPortDef->nPortIndex, NULL);
  if (eError != OMX_ErrorNone)
  {
    DBG_PRINT ("Error from SendCommand OMX_CommandPortDisable ");
    goto EXIT;
  }

  for (i = 0; i < pAppData->pOutPortDef->nBufferCountActual; i++)
  {
    eError =
      OMX_FreeBuffer (pAppData->pHandle, pAppData->pOutPortDef->nPortIndex,
                      pAppData->pOutBuff[i]);
    if (eError != OMX_ErrorNone)
    {
      DBG_PRINT ("Error in OMX_FreeBuffer");
      goto EXIT;
    }
  }

  eError =
    OMX_GetParameter (pAppData->pHandle, OMX_IndexParamPortDefinition,
                      pAppData->pOutPortDef);
  if (eError != OMX_ErrorNone)
  {
    DBG_PRINT ("Error in OMX_GetParameter");
    goto EXIT;
  }

  eError =
    OMX_SendCommand (pAppData->pHandle, OMX_CommandPortEnable,
                     pAppData->pOutPortDef->nPortIndex, NULL);
  if (eError != OMX_ErrorNone)
  {
    DBG_PRINT ("Error in OMX_SendCommand:OMX_CommandPortEnable");
    goto EXIT;
  }

  retval = TIMM_OSAL_ClearPipe (pAppData->OpBuf_Pipe);
  if (retval != TIMM_OSAL_ERR_NONE)
  {
    DBG_PRINT ("Error in clearing Output Pipe!");
    eError = OMX_ErrorNotReady;
    return eError;
  }

  for (i = 0; i < pAppData->pOutPortDef->nBufferCountActual; i++)
  {
    eError =
      OMX_AllocateBuffer (pAppData->pHandle, &pAppData->pOutBuff[i],
                          pAppData->pOutPortDef->nPortIndex, pAppData,
                          pAppData->pOutPortDef->nBufferSize);
    if (eError != OMX_ErrorNone)
    {
      DBG_PRINT ("Error in Allocating buffers");
      goto EXIT;
    }
  }

EXIT:

  return eError;
}

/* ========================================================================== */
/**
* ADEC_EventHandler() : This method is the event handler implementation to
* handle events from the OMX MP3 Derived component
*
* @param hComponent        : Handle to the component
* @param ptrAppData        :
* @param eEvent            :
* @param nData1            :
* @param nData2            :
* @param pEventData        :
*
*  @return
*  OMX_ErrorNone = Successful
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */
static OMX_ERRORTYPE ADEC_EventHandler(OMX_HANDLETYPE hComponent,
                                          OMX_PTR ptrAppData,
                                          OMX_EVENTTYPE eEvent, OMX_U32 nData1,
                                          OMX_U32 nData2, OMX_PTR pEventData)
{
	ADEC_CHAIN_T *pAppData = ptrAppData;
	/* OMX_STATETYPE state;*/
	TIMM_OSAL_ERRORTYPE retval;
	OMX_ERRORTYPE eError = OMX_ErrorNone;
  
	DBG_MSG("ADEC_EventHandler:eEvent=%d\n", eEvent);
	switch (eEvent)
	{
		case OMX_EventCmdComplete:
		/* callback from component indicated that command has been completed */
		if (nData1 == OMX_CommandStateSet)	{
        TIMM_OSAL_SemaphoreRelease (pSem_Events);
        retval = TIMM_OSAL_EventSet (ADEC_CmdEvent,  ADEC_STATETRANSITION_COMPLETE, TIMM_OSAL_EVENT_OR);
		if (retval != TIMM_OSAL_ERR_NONE) {
				DBG_PRINT ("\nADEC_EventHandler:Error in setting the event!\n");
				eError = OMX_ErrorNotReady;
				return eError;
			}
		}
		if (nData1 == OMX_CommandPortEnable || nData1 == OMX_CommandPortDisable) {
			retval = TIMM_OSAL_EventSet(ADEC_CmdEvent, ADEC_PORTCONFIGURATION_COMPLETE, TIMM_OSAL_EVENT_OR);
			if (retval != TIMM_OSAL_ERR_NONE) {
				DBG_PRINT("\nADEC_EventHandler:Error in setting event.......\n");
				eError = OMX_ErrorNotReady;
				return eError;
			}
		}
      break;
	case OMX_EventError:
		DBG_PRINT("\nADEC_EventHandler:Error event received from ADEC.......\n");
	break;
	case OMX_EventMark:
	break;
	case OMX_EventPortSettingsChanged:
		/* In case of change in output buffer sizes re-allocate the buffers */
		DBG_MSG ("ADEC_EventHandler: OMX_EventPortSettingsChanged\n");
		eError = ADEC_ChangePortSettings(pAppData);
	break;
	case OMX_EventBufferFlag:
		retval =  TIMM_OSAL_EventSet (myEvent, ADEC_DECODER_END_OF_STREAM, TIMM_OSAL_EVENT_OR);
		if (retval != TIMM_OSAL_ERR_NONE) {
			DBG_PRINT ("ADEC_EventHandler: Error in setting the event!");
			eError = OMX_ErrorNotReady;
			return eError;
		}
	break;
	case OMX_EventResourcesAcquired:
		break;
	case OMX_EventComponentResumed:
		break;
	case OMX_EventDynamicResourcesAvailable:
		break;
	case OMX_EventPortFormatDetected:
		break;
	case OMX_EventMax:
		break;
	default:
		DBG_PRINT("ADEC_EventHandler:Unhandled event\n");
		break;
  }  // end of switch

  return eError;
}

/* ========================================================================== */
/**
* ADEC_FillBufferDone() : This method handles the fill buffer done event
* got from the derived component
*
* @param hComponent        : Handle to the component
* @param ptrAppData        : Pointer to the app data
*
*  @return
*  OMX_ErrorNone = Successful
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */
static OMX_ERRORTYPE ADEC_FillBufferDone(OMX_HANDLETYPE hComponent,
                                            OMX_PTR ptrAppData,
                                            OMX_BUFFERHEADERTYPE *pBuffer)
{
	ADEC_CHAIN_T *pAppData = ptrAppData;
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	TIMM_OSAL_ERRORTYPE retval;

	if(!pAppData->fStreaming)	{
	    return eError;
	}
  
	/* Output buffers is available now, put in the queue */
	retval = TIMM_OSAL_WriteToPipe (pAppData->OpBuf_Pipe, &pBuffer, sizeof (pBuffer), TIMM_OSAL_SUSPEND);
	if (retval != TIMM_OSAL_ERR_NONE)  {
		DBG_PRINT ("Error writing to Output buffer Pipe!");
		eError = OMX_ErrorNotReady;
		return eError;
	}
	/* IL client checks this even for recycling the buffer */
	retval = TIMM_OSAL_EventSet (myEvent, ADEC_DECODER_OUTPUT_READY, TIMM_OSAL_EVENT_OR);
	if (retval != TIMM_OSAL_ERR_NONE) {
		DBG_PRINT ("Error in setting the o/p event!");
		eError = OMX_ErrorNotReady;
		return eError;
	}
	return eError;
}

/* ========================================================================== */
/**
* ADEC_EmptyBufferDone() : This method handles the Empty buffer done event
* got from the derived component
*
* @param hComponent        : Handle to the component
* @param ptrAppData        : Pointer to the app data
*
*  @return
*  OMX_ErrorNone = Successful
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */
static int gEOF=0, gbytesInInputBuffer= 0;
static OMX_ERRORTYPE ADEC_EmptyBufferDone(OMX_HANDLETYPE hComponent,
                                             OMX_PTR ptrAppData,
                                             OMX_BUFFERHEADERTYPE *pBuffer)
{
	ADEC_CHAIN_T *pAppData = ptrAppData;
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	TIMM_OSAL_ERRORTYPE retval;

	if (!pAppData->fStreaming) {
		return eError;
	}  
	/* input buffer is consumed and recycled in the queue */
	retval = TIMM_OSAL_WriteToPipe (pAppData->IpBuf_Pipe, &pBuffer, sizeof (pBuffer), TIMM_OSAL_SUSPEND);
	if (retval != TIMM_OSAL_ERR_NONE)  {
		DBG_PRINT ("Error writing to Input buffer i/p Pipe!");
		eError = OMX_ErrorNotReady;
		return eError;
	}

	/* IL client in this example is checking for this event to re-use the buffer
	*/
	retval = TIMM_OSAL_EventSet (myEvent, ADEC_DECODER_INPUT_READY, TIMM_OSAL_EVENT_OR);
	if (retval != TIMM_OSAL_ERR_NONE)  {
		DBG_PRINT ("Error in setting the event!");
		eError = OMX_ErrorNotReady;
		return eError;
	}

	return eError;
}


static void prg_exit(int code) 
{
//	done_stdin();
//	if (handle)
//		snd_pcm_close(handle);
//	if (pidfile_written)
//		remove (pidfile_name);
//	exit(code);
}

static char *gpOutputBuffer = NULL;
static void ConvertData16To32(char *pInBuffer, char *pOutputBuffer, int nFrames)
{
	int i;
	int nSamples = nFrames * 2;
	unsigned short *pusSrc = (unsigned short *)pInBuffer;
	unsigned long *pulDst = (unsigned long *)pOutputBuffer;
	for (i=0; i < nSamples; i++){
		*pulDst++ = (((unsigned long)(*pusSrc++)) << 16) & 0xFFFF0000;
	}
}

static int adecchainSetOption(
	StrmCompIf *pComp, 	
	int         nCmd, 
	char       *pOptionData)
{
	int ret;
	ADEC_CHAIN_T *pAppData = (ADEC_CHAIN_T *)pComp->pCtx;
	if(nCmd == AUD_DEC_CMD_SET_PARAMS) {
		IL_AUD_ARGS  *pArgs = (IL_AUD_ARGS  *)pOptionData;

		strncpy(pAppData->codec_name, pArgs->codec_name, 31);
		pAppData->aacRawFormat = pArgs->nRrawFormat; 
		if(pArgs->nSampleRate) {
			pAppData->audRawSampleRate = pArgs->nSampleRate;
		}
		pAppData->buffer_size= pArgs->buffer_size;
		pAppData->sync = pArgs->sync;
		pAppData->nSyncMaxLateness = pArgs->latency;
		if(pArgs->max_input_pkt_size) {
			pAppData->max_input_pkt_size = pArgs->max_input_pkt_size;
			DBG_LOG(DBGLVL_SETUP, ("max_input_pkt_size=%d",pAppData->max_input_pkt_size));
		}
		if(pArgs->dec_input_buffer_size) {
			pAppData->dec_input_buffer_size = pArgs->dec_input_buffer_size;
			DBG_LOG(DBGLVL_SETUP, ("dec_input_buffer_size=%d",pAppData->dec_input_buffer_size));
		}
		if(pArgs->dec_output_buffer_size){
			pAppData->dec_output_buffer_size = pArgs->dec_output_buffer_size;
			DBG_LOG(DBGLVL_SETUP, ("dec_output_buffer_size=%d",pAppData->dec_output_buffer_size));
		}
		if(pArgs->alsa_output_buffer_size){
			pAppData->alsa_output_buffer_size = pArgs->alsa_output_buffer_size;
			DBG_LOG(DBGLVL_SETUP, ("alsa_output_buffer_size=%d",pAppData->alsa_output_buffer_size));
		}

		pAppData->nMixerPort = pArgs->nSessionId;
	}
	return 0;
}

static int adecchainOpen(StrmCompIf *pComp, const char *pszResource)
{
	int ret;
	ADEC_CHAIN_T *pAppData = (ADEC_CHAIN_T *)pComp->pCtx;


	return 0;
}

static int adecchainDoPriming(OMX_HANDLETYPE pHandle, ADEC_CHAIN_T *pCtx)
{
	int i;
	OMX_U32 nRead;
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	ADEC_CHAIN_T *pAppData = pCtx;
  /* parser would fill the chunked frames in input buffers, which needs to be
     passed to component in empty buffer call; initially all buffers are
     avaialble so they be filled and given to component */

	// There is only one input buffer for now
	for (i = 0; i < pAppData->pInPortDef->nBufferCountActual; i++)	{
		nRead = ADEC_FillData (pAppData, pAppData->pInBuff[i]);
		DBG_LOG(DBGLVL_FRAME, ("<Initial aud frame len=%d pts=%lld>\n",nRead, pAppData->crnt_aud_pts));
		
		eError =  pAppData->pComponent->EmptyThisBuffer (pHandle, pAppData->pInBuff[i]);

		if (eError != OMX_ErrorNone)  {
			DBG_LOG(DBGLVL_ERROR, ("!!!Error from Empty this buffer : %s!!!", ADEC_GetDecoderErrorString (eError)));
			goto EXIT;
		}
		gbytesInInputBuffer = nRead;
	}
	

	/* Initially all bufers are available in IL client, so we can pass all free
	buffers to component */
	for (i = 0; i < pAppData->pOutPortDef->nBufferCountActual; i++)	{
		((OMX_BUFFERHEADERTYPE *) (pAppData->pOutBuff[i]))->nOffset = 0;
		eError = pAppData->pComponent->FillThisBuffer (pHandle, pAppData->pOutBuff[i]);
		if (eError != OMX_ErrorNone) {
			DBG_LOG(DBGLVL_ERROR,  ("!!! Error from Fill this buffer : %s !!!\n",	ADEC_GetDecoderErrorString (eError)));
			goto EXIT;
		}
	}
	pAppData->adecchainPrimed = 1;
EXIT:
	return eError;
}

static int adecchainStart(StrmCompIf *pComp)
{
	ADEC_CHAIN_T *pAppData = (ADEC_CHAIN_T *)pComp->pCtx;

	OMX_HANDLETYPE pHandle;
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_BUFFERHEADERTYPE *pBufferIn = NULL;
	OMX_BUFFERHEADERTYPE *pBufferOut = NULL;
	TIMM_OSAL_ERRORTYPE tTIMMSemStatus;
	OMX_U32 i;
	OMX_U32 actualSize;
	OMX_CALLBACKTYPE appCallbacks;
	
	AUDIO_MIXER_T *pAmixer = amixGetInstance();
	pAppData->pConnSink = amixGetInputPortConnCtx(pAmixer, pAppData->nMixerPort);

	TIMM_OSAL_U32 uRequestedEvents, pRetrievedEvents;
	OMX_U32 bytesRead=0;
	int decType;
	OMX_AUDIO_PARAM_AACPROFILETYPE aacParams;
	OMX_ADEC_TEST_INIT_STRUCT_PTR(&aacParams, OMX_AUDIO_PARAM_AACPROFILETYPE);

	pAppData->fStreaming = 1;
	pAppData->fEoS = 0;

	DBG_LOG(DBGLVL_TRACE,("Enter"));

  /* Callbacks are passed during getHandle call to component, Componnet uses
     these callaback to communicate with IL Client */

	appCallbacks.EventHandler = ADEC_EventHandler;
	appCallbacks.EmptyBufferDone = ADEC_EmptyBufferDone;
	appCallbacks.FillBufferDone = ADEC_FillBufferDone;

	/* Create evenets, which will be triggered during callback from componnet */

	tTIMMSemStatus = TIMM_OSAL_EventCreate (&myEvent);
	if (TIMM_OSAL_ERR_NONE != tTIMMSemStatus)	{
		DBG_LOG(DBGLVL_ERROR,  ("Error in creating event!"));
		eError = OMX_ErrorInsufficientResources;
		goto EXIT;
	}
	
	tTIMMSemStatus = TIMM_OSAL_EventCreate (&ADEC_CmdEvent);
	if (TIMM_OSAL_ERR_NONE != tTIMMSemStatus) {
		TIMM_OSAL_Trace ("Error in creating event!\n");
		eError = OMX_ErrorInsufficientResources;
		goto EXIT;
	}
	/* Allocating data structure for IL client structure / buffer management */

#ifdef DBG_DUMP
	DBG_LOG(DBGLVL_SETUP,  ("Opening Debug File"));
	pAppData->fDbgSave = fopen ("dump.aud", "wb");
#endif

	/* compression format as aaclc, OMX enumeration */
	if (strcmp (pAppData->codec_name, "aaclc") == 0)	{
		pAppData->eCompressionFormat = OMX_AUDIO_CodingAAC;
		decType = 1;
	}  else if (strcmp (pAppData->codec_name, "mp3") == 0)  {
		pAppData->eCompressionFormat = OMX_AUDIO_CodingMP3;
		decType = 0;
#ifdef EN_DDPDEC
	}  else if (strcmp (pAppData->codec_name, "ac3") == 0)  {
		pAppData->eCompressionFormat = OMX_AUDIO_CodingDDP;
		decType = 2;
#endif
	}  else   {
		DBG_LOG(DBGLVL_ERROR,  ("Invalid bitstream format specified, should be either aaclc or mp3"));
		return -1;
	}

	gpOutputBuffer =  (char *)malloc(32 * 1024);

	/* Allocating data structure for buffer queues in IL client */
	eError = ADEC_AllocateResources(pAppData);
	if (eError != OMX_ErrorNone)  {
		DBG_LOG(DBGLVL_ERROR,  ("Error allocating resources in main!"));
		eError = OMX_ErrorInsufficientResources;
		goto EXIT;
	}

	pAppData->eState = OMX_StateInvalid;
	*pAppData->pCb = appCallbacks;

	tTIMMSemStatus = TIMM_OSAL_SemaphoreCreate (&pSem_Events, 0);
	if (tTIMMSemStatus != TIMM_OSAL_ERR_NONE)	{
		DBG_LOG(DBGLVL_ERROR, ("Semaphore Create failed!"));
		goto EXIT;
	}


	DBG_LOG(DBGLVL_TRACE,  (" calling getHandle \n"));

	/* Create the Decoder Component, component handle would be returned component
		name is unique and fixed for a componnet, callback are passed to
		componnet in this function. Componnet would be loaded state post this call
	*/

	eError = OMX_GetHandle(&pHandle, (OMX_STRING) "OMX.TI.DSP.AUDDEC", pAppData, pAppData->pCb);
	DBG_MSG (" got handle \n");

  if ((eError != OMX_ErrorNone) || (pHandle == NULL))  {
		DBG_LOG(DBGLVL_ERROR,  ("Error in Get Handle function : %s",	ADEC_GetDecoderErrorString (eError)));
		goto EXIT;
	}

	pAppData->pHandle = pHandle;
	pAppData->pComponent = (OMX_COMPONENTTYPE *) pHandle;

	/* for input port parameter settings */

	/* number of bufferes are port properties, component tracks number of buffers
	     allocated during loaded to idle transition */
	pAppData->pInPortDef->nBufferCountActual = 1;
	pAppData->pInPortDef->nPortIndex = OMX_AUDDEC_INPUT_PORT;
	pAppData->pInPortDef->nBufferSize = pAppData->dec_input_buffer_size;
	pAppData->pInPortDef->format.audio.eEncoding = pAppData->eCompressionFormat;

	/* for output port parameters setting */
	pAppData->pOutPortDef->nBufferCountActual = 1;
	pAppData->pOutPortDef->nPortIndex = OMX_AUDDEC_OUTPUT_PORT;
	pAppData->pOutPortDef->nBufferSize = pAppData->dec_output_buffer_size;

	ADEC_SetParamPortDefinition(pAppData->pHandle, decType, pAppData->aacRawFormat, pAppData->audRawSampleRate, 
		pAppData->pInPortDef->nBufferSize, pAppData->pOutPortDef->nBufferSize);


	OMX_SendCommand  ( pAppData->pHandle, OMX_CommandPortEnable,
						OMX_AUDDEC_INPUT_PORT, NULL );
	/* Wait for initialization to complete.. Wait for port enable of component  */
	eError = ADEC_WaitForPortConfig(pHandle);
	if (eError != OMX_ErrorNone) {
		goto EXIT;
	}

	OMX_SendCommand  ( pAppData->pHandle, OMX_CommandPortEnable,
                     OMX_AUDDEC_OUTPUT_PORT, NULL );
	/* Wait for initialization to complete.. Wait for port enable of component  */
	eError = ADEC_WaitForPortConfig(pHandle);
	if (eError != OMX_ErrorNone) {
		goto EXIT;
	}

  /* OMX_SendCommand expecting OMX_StateIdle, after this command component
     would create codec, and will wait for all buffers to be allocated */
	eError = OMX_SendCommand (pHandle, OMX_CommandStateSet, OMX_StateIdle, NULL);
	if (eError != OMX_ErrorNone)	{
		DBG_LOG(DBGLVL_ERROR,  ("Error in SendCommand()-OMX_StateIdle State set : %s", ADEC_GetDecoderErrorString (eError)));
		goto EXIT;
	}
  TIMM_OSAL_Trace ("\nCame back from send command without error\n");

	/* Allocate I/O Buffers; componnet would allocated buffers and would return
     the buffer header containing the pointer to buffer */
	for (i = 0; i < pAppData->pInPortDef->nBufferCountActual; i++)	{
		eError = OMX_AllocateBuffer (pHandle,       /* &pBufferIn */
										&pAppData->pInBuff[i],
										pAppData->pInPortDef->nPortIndex, pAppData,
										pAppData->pInPortDef->nBufferSize);
		if (eError != OMX_ErrorNone)   {
			DBG_LOG(DBGLVL_ERROR, ("Error in OMX_AllocateBuffer()- Input Port State set : %s \n", ADEC_GetDecoderErrorString (eError)));
			goto EXIT;
		}
	}
	/* buffer alloaction for output port */
	for (i = 0; i < pAppData->pOutPortDef->nBufferCountActual; i++)	{
		eError = OMX_AllocateBuffer (pHandle,       /* &pBufferOut */
										&pAppData->pOutBuff[i],
										pAppData->pOutPortDef->nPortIndex, pAppData,
										pAppData->pOutPortDef->nBufferSize);
		if (eError != OMX_ErrorNone)    {
			DBG_LOG(DBGLVL_ERROR, ("Error in OMX_AllocateBuffer()-Output Port State set : %s", ADEC_GetDecoderErrorString (eError)));
			goto EXIT;
		}
	}


  /* Wait for initialization to complete.. Wait for Idle stete of component
     after all buffers are alloacted componet would chnage to idle */

	eError = ADEC_WaitForState(pHandle, OMX_StateIdle);
	if (eError != OMX_ErrorNone)	{
		DBG_LOG(DBGLVL_ERROR,  ("Error %s:    WaitForState has timed out \n", ADEC_GetDecoderErrorString (eError)));
		goto EXIT;
	}
	DBG_LOG(DBGLVL_TRACE,  (" state IDLE"));

	/* change state to execute so that buffers processing can start */
	eError = OMX_SendCommand (pHandle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
	if (eError != OMX_ErrorNone)	{
		DBG_LOG(DBGLVL_ERROR,  ("Error from SendCommand-Executing State set :%s \n", ADEC_GetDecoderErrorString (eError)));
		goto EXIT;
	}

	eError = ADEC_WaitForState(pHandle, OMX_StateExecuting);
	if (eError != OMX_ErrorNone){
		DBG_LOG(DBGLVL_ERROR,  ("Error %s:    WaitForState has timed out",	ADEC_GetDecoderErrorString (eError)));
		goto EXIT;
	}
	DBG_LOG(DBGLVL_TRACE,  (" state execute"));



	/* all available buffers have been passed to component, now wait for
		processed buffers to come back via eventhandler callback */
	SET_THREAD_POS(pAppData , __FUNCTION__, __LINE__)
	while (!pAppData->fEoS) {
		
		if(!pAppData->adecchainPrimed){

			SET_THREAD_POS(pAppData , __FUNCTION__, __LINE__)
			eError = adecchainDoPriming(pHandle, pAppData);
			if (eError != OMX_ErrorNone) {
				DBG_LOG(DBGLVL_ERROR,  ("!!!Aud Chain priming failed!!!"));
				goto EXIT;
			}
		}

		SET_THREAD_POS(pAppData , __FUNCTION__, __LINE__)
		eError = OMX_GetState (pHandle, &pAppData->eState);
		if(eError != OMX_ErrorNone || pAppData->eState == OMX_StateIdle){
			DBG_LOG(DBGLVL_ERROR,  ("!!!Error in OMX_GetState eError=%d  eState=%d!!!", eError,pAppData->eState));
			goto EXIT;
		}
		uRequestedEvents = (ADEC_DECODER_INPUT_READY | ADEC_DECODER_OUTPUT_READY |
					ADEC_DECODER_ERROR_EVENT | ADEC_DECODER_END_OF_STREAM);

		DBG_LOG(DBGLVL_FRAME, (":TIMM_OSAL_EventRetrieve..."));
		SET_THREAD_POS(pAppData , __FUNCTION__, __LINE__)
		tTIMMSemStatus = TIMM_OSAL_EventRetrieve (myEvent, uRequestedEvents, TIMM_OSAL_EVENT_OR_CONSUME, &pRetrievedEvents, TIMM_OSAL_SUSPEND);
		if (TIMM_OSAL_ERR_NONE != tTIMMSemStatus) {
			DBG_LOG(DBGLVL_ERROR,  ("Error in creating event!"));
			eError = OMX_ErrorUndefined;
			goto EXIT;
		}
		if (pRetrievedEvents & ADEC_DECODER_END_OF_STREAM)	{
			DBG_LOG(DBGLVL_ERROR,  ("End of stream processed\n"));
			break;
		}

		if ((pRetrievedEvents & ADEC_DECODER_OUTPUT_READY) && (pAppData->fStreaming)) {
			int err;
			int fDrop = 0;
			/* read from the pipe */
			SET_THREAD_POS(pAppData , __FUNCTION__, __LINE__)
			TIMM_OSAL_ReadFromPipe (pAppData->OpBuf_Pipe, &pBufferOut,sizeof (pBufferOut), &actualSize,	TIMM_OSAL_SUSPEND);
			if(pAppData->eCompressionFormat == OMX_AUDIO_CodingAAC){
				eError = OMX_GetParameter(pHandle, OMX_IndexParamAudioAac, &aacParams);
				if (eError != OMX_ErrorNone) {
					goto EXIT;
				}
			}
			if(pBufferOut != NULL) {
				ConvertData16To32(pBufferOut->pBuffer, gpOutputBuffer, pBufferOut->nFilledLen/4);
				//audRenderFrame(pAppData, gpOutputBuffer, pBufferOut->nFilledLen/4);
				pAppData->pConnSink->Write(pAppData->pConnSink, gpOutputBuffer, pBufferOut->nFilledLen, 0, 0/*TODO:Time Stampe*/);
			}

			if (pAppData->fStreaming) {
				((OMX_BUFFERHEADERTYPE *) (pAppData->pOutBuff[0]))->nOffset = 0;
				eError = pAppData->pComponent->FillThisBuffer(pHandle, pAppData->pOutBuff[0]);
				if (eError != OMX_ErrorNone) {
					goto EXIT;
				}
			}
			pAppData->nDecodedFrms++;
		} //if ((pRetrievedEvents & ADEC_DECODER_OUTPUT_READY) && (pAppData->fStreaming))

		if ((pRetrievedEvents & ADEC_DECODER_INPUT_READY) && (pAppData->fStreaming)) {
			/*read from the pipe */
			SET_THREAD_POS(pAppData , __FUNCTION__, __LINE__)
			TIMM_OSAL_ReadFromPipe(pAppData->IpBuf_Pipe, &pBufferIn, sizeof(pBufferIn), &actualSize, TIMM_OSAL_SUSPEND);
			
			//printf("<AudIn Allc=%d supplied=%d remain=%d consumed=%d>",pBufferIn->nAllocLen, gbytesInInputBuffer, pBufferIn->nFilledLen, gbytesInInputBuffer-pBufferIn->nFilledLen);
			if(pBufferIn != NULL) {
				int nBytesConsumed = gbytesInInputBuffer-pBufferIn->nFilledLen;
				// gbytesInInputBuffer-pBufferIn->nFilledLen == bytes consumed ?? gives start of unused data ??
				if(pBufferIn->nFilledLen) {
					// Move unconsumed data to the beginning of the buffer
					memcpy(&pBufferIn->pBuffer[0], &pBufferIn->pBuffer[gbytesInInputBuffer-pBufferIn->nFilledLen],	pBufferIn->nFilledLen);
				}
				SET_THREAD_POS(pAppData , __FUNCTION__, __LINE__)
				bytesRead = AudParserReadData(pAppData, &pBufferIn->pBuffer[pBufferIn->nFilledLen], pBufferIn->nAllocLen - pBufferIn->nFilledLen);
				DBG_LOG(DBGLVL_FRAME, ("<consumed=%d parseReq=%d ParseRet=%d>",pBufferIn->nAllocLen - pBufferIn->nFilledLen, bytesRead, nBytesConsumed));
				gbytesInInputBuffer = pBufferIn->nFilledLen + bytesRead;
			}
			((OMX_BUFFERHEADERTYPE *) (pAppData->pInBuff[0]))->nFilledLen = gbytesInInputBuffer;
			((OMX_BUFFERHEADERTYPE *) (pAppData->pInBuff[0]))->nOffset = 0;
			eError =  pAppData->pComponent->EmptyThisBuffer(pHandle, pAppData->pInBuff[0]);
			if (eError != OMX_ErrorNone) {
				goto EXIT;
			}
		}
		if (pRetrievedEvents & ADEC_DECODER_ERROR_EVENT) {
			eError = OMX_ErrorUndefined;
		}
	}
	DBG_LOG(DBGLVL_SETUP, ("Component transitioning from Executing to Idle state"));

	//  EXIT1:
	DBG_LOG(DBGLVL_SETUP, ("Tearing down..."));
	/* change the state to idle, bufefr communication would stop in this state */
	eError = OMX_SendCommand (pHandle, OMX_CommandStateSet, OMX_StateIdle, NULL);
	if (eError != OMX_ErrorNone) {
		DBG_LOG(DBGLVL_ERROR,  ("Error from SendCommand-Idle State set : %s", ADEC_GetDecoderErrorString (eError)));
		goto EXIT;
	}
	SET_THREAD_POS(pAppData , __FUNCTION__, __LINE__)
	eError = ADEC_WaitForState(pHandle, OMX_StateIdle);
	if (eError != OMX_ErrorNone)	{
		DBG_LOG(DBGLVL_ERROR,  ("Error %s:    WaitForState has timed out",	ADEC_GetDecoderErrorString (eError)));
		goto EXIT;
	}

	/* change the state to loaded, componenet would wait for all buffers to be
		freed up, then only state transition would complete */
	eError =
	OMX_SendCommand (pHandle, OMX_CommandStateSet, OMX_StateLoaded, NULL);
	if (eError != OMX_ErrorNone)	{
		DBG_LOG(DBGLVL_ERROR,  ("Error from SendCommand-Loaded State set : %s",	ADEC_GetDecoderErrorString (eError)));
		goto EXIT;
	}
	/* During idle-> loaded state transition buffers need to be freed up */
	for (i = 0; i < pAppData->pInPortDef->nBufferCountActual; i++)
	{
		eError =
		OMX_FreeBuffer (pHandle, pAppData->pInPortDef->nPortIndex,
						pAppData->pInBuff[i]);
		if (eError != OMX_ErrorNone)
		{
			DBG_LOG(DBGLVL_ERROR,  ("Error in OMX_FreeBuffer : %s",	ADEC_GetDecoderErrorString (eError)));
			goto EXIT;
		}
	}

	for (i = 0; i < pAppData->pOutPortDef->nBufferCountActual; i++)
	{
		eError =
		OMX_FreeBuffer (pHandle, pAppData->pOutPortDef->nPortIndex,	pAppData->pOutBuff[i]);
		if (eError != OMX_ErrorNone) {
			DBG_LOG(DBGLVL_ERROR,  ("Error in OMX_FreeBuffer : %s \n", ADEC_GetDecoderErrorString (eError)));
			goto EXIT;
		}
  }
	/* wait for state transition to complete, componnet would generate an event,
	when state transition is complete */
	SET_THREAD_POS(pAppData , __FUNCTION__, __LINE__)
	eError = ADEC_WaitForState(pHandle, OMX_StateLoaded);
	if (eError != OMX_ErrorNone)	{
		DBG_LOG(DBGLVL_ERROR,  ("Error %s:    WaitForState has timed out \n",	ADEC_GetDecoderErrorString (eError)));
		goto EXIT;
	}
	DBG_LOG(DBGLVL_TRACE, (":free handle"));

	/* UnLoad the Decoder Component */
	eError = OMX_FreeHandle (pHandle);
	if ((eError != OMX_ErrorNone))  {
		DBG_LOG(DBGLVL_ERROR,  ("Error in Free Handle function : %s \n", ADEC_GetDecoderErrorString (eError)));
		goto EXIT;
	}

	DBG_LOG(DBGLVL_TRACE, (":free handle done"));

	tTIMMSemStatus = TIMM_OSAL_SemaphoreDelete (pSem_Events);
	if (tTIMMSemStatus != TIMM_OSAL_ERR_NONE) {
		DBG_LOG(DBGLVL_ERROR, ("Semaphore Delete failed!"));
		goto EXIT;
	}
	
EXIT:
	/* releasing IL client data structure */
	if (pAppData) {
#ifdef DBG_DUMP
		if (pAppData->fDbgSave) {
			fclose (pAppData->fDbgSave);
		}
#endif
	}


	tTIMMSemStatus = TIMM_OSAL_EventDelete (myEvent);
	if (TIMM_OSAL_ERR_NONE != tTIMMSemStatus)	{
		DBG_LOG(DBGLVL_ERROR, ("Error in deleting event!"));
	}

	tTIMMSemStatus = TIMM_OSAL_EventDelete (ADEC_CmdEvent);
	if (TIMM_OSAL_ERR_NONE != tTIMMSemStatus) {
		TIMM_OSAL_Trace ("Error in deleting event!\n");
	}
  
	if(gpOutputBuffer)
		free(gpOutputBuffer);

	if (pAppData) {
		ADEC_FreeResources (pAppData);
	}

	DBG_LOG(DBGLVL_TRACE,("Leave"));

	pAppData->fStreaming = 0;
	return (0);
} /* OMX_Audio_Decode_Test */

static void adecchainSetClkSrc(StrmCompIf *pComp, void *pClk)
{
	ADEC_CHAIN_T *pAppData = (ADEC_CHAIN_T *)pComp->pCtx;
	pAppData->pClk = pClk;
}

static void *adecchainGetClkSrc(StrmCompIf *pComp)
{
	ADEC_CHAIN_T *pAppData = (ADEC_CHAIN_T *)pComp->pCtx;
	pAppData->fClkSrc= 1;
	pAppData->pClk = clkCreate(1);
	return pAppData->pClk;
}

static void adecchainSetInputConn(StrmCompIf *pComp, int nConnNum, ConnCtxT *pConn)
{
	ADEC_CHAIN_T *pAppData = (ADEC_CHAIN_T *)pComp->pCtx;
	pAppData->pConnSrc = pConn;
}

static int adecchainStop(StrmCompIf *pComp)
{
	ADEC_CHAIN_T *pAppData = (ADEC_CHAIN_T *)pComp->pCtx;
	pAppData->nUiCmd = STRM_CMD_STOP;
	int nTimeout = 300000;
	DBG_LOG(DBGLVL_SETUP, ("Waiting for stream stop beign"));
	SHOW_THREAD_POS(pAppData)
	while(pAppData->fStreaming && nTimeout > 0) {
		usleep(1000);
		nTimeout -= 1000;
	}
	DBG_LOG(DBGLVL_TRACE, ("Waiting for stream stop end timeoutreamin=%dms", nTimeout/1000));
	return 0;
}

static int adecchainDelete(StrmCompIf *pComp)
{
	ADEC_CHAIN_T *pAppData = (ADEC_CHAIN_T *)pComp->pCtx;
	free (pAppData);
	return 0;
}

extern char   *gIniFile;
#define DECODER_SECTION "decode"

StrmCompIf *
aacdecchainCreate()
{
	DBG_LOG(DBGLVL_TRACE, ("Begin"));
	StrmCompIf *pComp = (StrmCompIf *)malloc(sizeof(StrmCompIf));
	memset (pComp, 0x0, sizeof (StrmCompIf));
	ADEC_CHAIN_T *pAppData = (ADEC_CHAIN_T *) malloc (sizeof (ADEC_CHAIN_T));
	memset (pAppData, 0x0, sizeof (ADEC_CHAIN_T));
	pAppData->nJitterLatency = ini_getl(DECODER_SECTION, "JITTER_LATENCY", 0, gIniFile);
	pAppData->nSyncMaxWaitRunning = ini_getl(DECODER_SECTION, "ASYNC_MAX_WAIT_RUNNING", 30000, gIniFile);
	pAppData->nSyncMaxWaitStartup = ini_getl(DECODER_SECTION, "ASYNC_MAX_WAIT_STARTUP", 500000, gIniFile);
	DBG_LOG(DBGLVL_SETUP, ("nSyncMaxWaitStartup=%d",pAppData->nSyncMaxWaitStartup));

	strcpy(pAppData->device,"default"); /* playback device */
	pAppData->audRawSampleRate = 48000;
	pAppData->fPcmPassthru = 0;
	pAppData->fEnableFormatChange = ini_getl(DECODER_SECTION, "ENABLE_AUD_DYNAMIC_FORMAT_CHANGE", 1, gIniFile);
	pComp->pCtx = pAppData;
	pAppData->nDecodedFrms = 0;
	pAppData->adecchainPrimed = 0;
	pAppData->dec_input_buffer_size = AUD_DEFAULT_INPUT_BUF_SIZE;
	pAppData->dec_output_buffer_size = AUD_DEFAULT_OUTPUT_BUF_SIZE;
	pAppData->alsa_output_buffer_size = ALSA_DEFAULT_PERIOD_BUFF_SIZE * 3; 
	pAppData->max_input_pkt_size = AUD_DEFAULT_INPUT_PKT_SIZE;

	pComp->Open= adecchainOpen;
	pComp->SetOption = adecchainSetOption;
	pComp->SetInputConn= adecchainSetInputConn;
	pComp->SetOutputConn= NULL;
	pComp->SetClkSrc = adecchainSetClkSrc;
	pComp->GetClkSrc = adecchainGetClkSrc;
	pComp->Start = adecchainStart;
	pComp->Stop =adecchainStop;
	pComp->Close = NULL;
	pComp->Delete = adecchainDelete;
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return pComp;
}
