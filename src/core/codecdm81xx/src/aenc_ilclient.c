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
 *  @file  ilclient.c
 *  @brief This file contains all Functions related to Test Application
 *
 *         This is the example IL Client support to create, configure & chaining
 *         of multi channel omx-components using proprietary tunneling
 *         mode
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
#include <unistd.h>
#include <xdc/std.h>
#include <xdc/runtime/Assert.h>
#include <xdc/runtime/knl/Thread.h>
#include <xdc/runtime/Timestamp.h>
#include <ti/omx/omxutils/omx_utils.h>
#ifdef CODEC_AACENC
#include <ti/sdo/codecs/aaclcenc/imp4aacenc.h>
#endif
/*-------------------------program files -------------------------------------*/
#include "ti/omx/interfaces/openMaxv11/OMX_Audio.h"
#include "ti/omx/interfaces/openMaxv11/OMX_Core.h"
#include "ti/omx/interfaces/openMaxv11/OMX_Component.h"
#include "omx_base_utils.h"
#include "semp.h"
#include "OMX_TI_Common.h"
#include "omx_aenc.h"

#include "timm_osal_interfaces.h"
#include <alsa/asoundlib.h>
#include "strmcomp.h"
#include "dbglog.h"
#include "aenc_ilclient_utils.h"
#include "dec_clock.h"
/*******************************************************************************
 * EXTERNAL REFERENCES NOTE : only use if not found in header file
*******************************************************************************/

/****************************************************************
 * DEFINES
 ****************************************************************/

/** Event definition to indicate input buffer consumed */
#define AENC_ENCODER_INPUT_READY 1

/** Event definition to indicate output buffer consumed */
#define AENC_ENCODER_OUTPUT_READY   2

/** Event definition to indicate error in processing */
#define AENC_ENCODER_ERROR_EVENT 4

/** Event definition to indicate End of stream */
#define AENC_ENCODER_END_OF_STREAM 8

#define INPUT_BUF_SIZE 1024

#define MAX(a,b) (a) > (b)? (a): (b)

static int mDbgLevel = DBGLVL_SETUP;
/****************************************************************
 * GLOBALS
 ****************************************************************/
int gOutputEventOccured = 0;

semp_t *port_sem;
semp_t *state_sem;

static OMX_BOOL bEOS_Sent;

/** Macro to initialize memset and initialize the OMX structure */
#define OMX_AENC_TEST_INIT_STRUCT_PTR(_s_, _name_)       \
 memset((_s_), 0x0, sizeof(_name_)); \
    (_s_)->nSize = sizeof(_name_);              \
    (_s_)->nVersion.s.nVersionMajor = 0x1;      \
    (_s_)->nVersion.s.nVersionMinor = 0x1;      \
    (_s_)->nVersion.s.nRevision  = 0x0;       \
    (_s_)->nVersion.s.nStep   = 0x0;

static ShowStats(AENC_Client * pCtx)
{
	// Display Statistics
	{
		char szClck[256];
		char szPts[256];

		CLOCK_T pts = pCtx->crnt_input_pts;
		CLOCK_T clk = ClockGetInternalTime(NULL/*pCtx->pClk*/);
		if(clk - pCtx->StatPrevClk >= TIME_SECOND) {
			int buffOcupancy;
			Clock2HMSF(clk, szClck, 255);
			Clock2HMSF(pts, szPts, 255);
			buffOcupancy = pCtx->pConnSrc->BufferFullness(pCtx->pConnSrc);
			DBG_PRINT("<%s:AEnc frame=%d rate=%0.2f crnt frame size=%d buff_full=%d pts=%s>\n", szClck,pCtx->nEncodedFrms, 1.0 * (pCtx->nEncodedFrms - pCtx->nStatPrevFrames) / ((clk - pCtx->StatPrevClk) / TIME_SECOND), pCtx->nCrntFrameLen, buffOcupancy, szPts);
			pCtx->nStatPrevFrames = pCtx->nEncodedFrms;
			pCtx->StatPrevClk = clk;
		}
	}
}

/*--------------------- function prototypes ----------------------------------*/
Void getDefaultHeapStats();

OMX_ERRORTYPE AENC_SetParamPortDefinition(AENC_Client * pAppData);

/* ========================================================================== */
/**
* AENC_AllocateResources() : Allocates the resources required for Audio
* Encoder.
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
static OMX_ERRORTYPE AENC_AllocateResources(AENC_Client * pAppData)
{
    OMX_U32 retval;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    pAppData->pCb = (OMX_CALLBACKTYPE *) malloc(sizeof(OMX_CALLBACKTYPE));
    if (!pAppData->pCb) {
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    pAppData->pInPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *)
        malloc(sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    if (!pAppData->pInPortDef) {
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    pAppData->pOutPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *)
        malloc(sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    if (!pAppData->pOutPortDef) {
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    /* Create a pipes for Input and Output Buffers.. used to queue data from the callback. */
    retval = pipe((int *) pAppData->IpBuf_Pipe);
    if (retval == -1) {
        eError = OMX_ErrorContentPipeCreationFailed;
        goto EXIT;
    }
    retval = pipe((int *) pAppData->OpBuf_Pipe);
    if (retval == -1) {
        eError = OMX_ErrorContentPipeCreationFailed;
        goto EXIT;
    }
    retval = pipe((int *) pAppData->Event_Pipe);
    if (retval == -1) {
        eError = OMX_ErrorContentPipeCreationFailed;
        goto EXIT;
    }

  EXIT:
    return eError;
}

/* ========================================================================== */
/**
* AENC_FreeResources() : Free the resources allocated for Audio
* Encoder.
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
static void AENC_FreeResources(AENC_Client * pAppData)
{
	if (pAppData->pCb)
		free(pAppData->pCb);

	if (pAppData->pInPortDef)
		free(pAppData->pInPortDef);

    if (pAppData->pOutPortDef)
        free(pAppData->pOutPortDef);

    close((int) pAppData->IpBuf_Pipe);

    close((int) pAppData->OpBuf_Pipe);

    close((int) pAppData->Event_Pipe);

    return;
}

/* ========================================================================== */
/**
* AENC_GetEncoderErrorString() : Function to map the OMX error enum to string
*
* @param error   : OMX Error type
*
*  @return
*  String conversion of the OMX_ERRORTYPE
*
*/
/* ========================================================================== */
#ifdef ILC_AENC_GETENCERRSTR_IN_BUILD
static OMX_STRING AENC_GetEncoderErrorString(OMX_ERRORTYPE error)
{
    OMX_STRING errorString;

    switch (error) {
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
#endif /* ILC_AENC_GETENCERRSTR_IN_BUILD */

static int GetInputData(AENC_Client *pAppData, unsigned char *pData, int lenData)
{
	int nBytesCopied = 0;

	int ret = 0;
	unsigned long long ullPts = 0;
	unsigned long ulFlags;
	DBG_LOG_M(DBGLVL_FRAME,  (" Waitfor buffer..."));
	while(pAppData->pConnSrc->IsEmpty(pAppData->pConnSrc) && pAppData->m_fStream){
		usleep(1000);
	}
	DBG_LOG_M(DBGLVL_FRAME,  (" Waitfor buffer:Done"));
	if(!pAppData->m_fStream) {
		goto Exit;
	} 
	ret = pAppData->pConnSrc->Read(pAppData->pConnSrc, pData, lenData, &ulFlags, &ullPts);
	DBG_LOG(DBGLVL_FRAME,  (" Read bytes=%d ulFlags=0x%0x ullPts=%lld", ret, ulFlags, ullPts));
	if(nBytesCopied == 0/*first buffer */)
		pAppData->crnt_input_pts = ullPts;

	nBytesCopied += ret; 

Exit:

	return nBytesCopied;
}

static int SendOutputData(AENC_Client *pAppData, unsigned char *pData, int lenData)
{
	CLOCK_T pts = 0;
	DBG_LOG_M(DBGLVL_FRAME,  ("pAppData->pConnDest=%p pData=%p lenData=%d", pAppData->pConnDest, pData, lenData));
	if(pAppData->pConnDest) {
		DBG_LOG_M(DBGLVL_FRAME,  ("Waitfor buffer:Begin"));
		while(pAppData->pConnDest->IsFull(pAppData->pConnDest) && pAppData->m_fStream){
			DBG_LOG(DBGLVL_WAITLOOP,("Waiting for free buffer"))
	#ifdef WIN32
				Sleep(1);
	#else
				usleep(1000);
	#endif
		}
		if(!pAppData->m_fStream)
			goto Exit;
		DBG_LOG_M(DBGLVL_FRAME,  ("Waitfor buffer:Done"));
		pts =  ClockGetTime(NULL);
		pAppData->pConnDest->Write(pAppData->pConnDest, pData, lenData,  0, pts);
	} else {
		DBG_LOG_M(DBGLVL_FRAME,  ("No output conn. Discarding data.."));
	}
Exit:
	return 0;
}

/* ========================================================================== */
/**
* AENC_FillData() : Function to fill the input buffer with data.
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
static OMX_U32
AENC_FillData(AENC_Client * pAppData, OMX_BUFFERHEADERTYPE * pBufHdr)
{
    OMX_U32 nRead = 0;
    OMX_U32 toRead = pAppData->pInPortDef->nBufferSize;

    int tFrames;

	if (pAppData->pConnSrc != NULL){
		nRead = GetInputData(pAppData, pBufHdr->pBuffer, toRead);
    }
    pBufHdr->nFilledLen = toRead;
    pBufHdr->nOffset = 0;

    return nRead;
}

/* ========================================================================== */
/**
* AENC_ChangePortSettings() : This method will
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
static OMX_ERRORTYPE AENC_ChangePortSettings(AENC_Client * pAppData)
{

    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 i;


    eError =
        OMX_SendCommand(pAppData->pHandle, OMX_CommandPortDisable,
                        pAppData->pOutPortDef->nPortIndex, NULL);
    if (eError != OMX_ErrorNone) {
        goto EXIT;
    }

    for (i = 0; i < pAppData->pOutPortDef->nBufferCountActual; i++) {
        eError =
            OMX_FreeBuffer(pAppData->pHandle, pAppData->pOutPortDef->nPortIndex,
                           pAppData->pOutBuff[i]);
        if (eError != OMX_ErrorNone) {
            goto EXIT;
        }
    }

    eError =
        OMX_GetParameter(pAppData->pHandle, OMX_IndexParamPortDefinition,
                         pAppData->pOutPortDef);
    if (eError != OMX_ErrorNone) {
        goto EXIT;
    }

    eError =
        OMX_SendCommand(pAppData->pHandle, OMX_CommandPortEnable,
                        pAppData->pOutPortDef->nPortIndex, NULL);
    if (eError != OMX_ErrorNone) {
        goto EXIT;
    }

    for (i = 0; i < pAppData->pOutPortDef->nBufferCountActual; i++) {
        eError =
            OMX_AllocateBuffer(pAppData->pHandle, &pAppData->pOutBuff[i],
                               pAppData->pOutPortDef->nPortIndex, pAppData,
                               pAppData->pOutPortDef->nBufferSize);
        if (eError != OMX_ErrorNone) {
            goto EXIT;
        }
    }

  EXIT:
    return eError;

}

/* ========================================================================== */
/**
* AENC_EventHandler() : This method is the event handler implementation to
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
static OMX_ERRORTYPE
AENC_EventHandler(OMX_HANDLETYPE hComponent,
                                       OMX_PTR ptrAppData,
                                       OMX_EVENTTYPE eEvent, OMX_U32 nData1,
                                       OMX_U32 nData2, OMX_PTR pEventData)
{
    AENC_Client *pAppData = ptrAppData;
    /*OMX_STATETYPE state; */
    OMX_S32 retval;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 postEvent;
	DBG_PRINT( "Event %d\n", eEvent);
    switch (eEvent) {
    case OMX_EventCmdComplete:
        if (nData1 == OMX_CommandStateSet) {
            semp_post(state_sem);
        }
        if (nData1 == OMX_CommandPortEnable || nData1 == OMX_CommandPortDisable) {
            semp_post(port_sem);
        }
        break;
    case OMX_EventError:
        postEvent = AENC_ENCODER_ERROR_EVENT;
        retval = write(pAppData->Event_Pipe[1], &postEvent, sizeof(postEvent));
        if (retval != sizeof(postEvent)) {
            eError = OMX_ErrorNotReady;
            return eError;
        }
        eError = OMX_GetState(pAppData->pHandle, &pAppData->eState);
        /*For create errors: */
        if (pAppData->eState == OMX_StateLoaded) {
            semp_post(state_sem);
        }
        break;
    case OMX_EventMark:

        break;
    case OMX_EventPortSettingsChanged:

        /* In case of change in output buffer sizes re-allocate the buffers */
        eError = AENC_ChangePortSettings(pAppData);

        break;
    case OMX_EventBufferFlag:
        postEvent = AENC_ENCODER_END_OF_STREAM;
        retval = write(pAppData->Event_Pipe[1], &postEvent, sizeof(postEvent));
        if (retval != sizeof(postEvent)) {
            eError = OMX_ErrorNotReady;
			DBG_PRINT( "Event OMX_ErrorNotReady\n");
            return eError;
        }
        /* EOS here nData1-> port....  nData2->OMX_BUFFERFLAG_EOS */
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
    case OMX_EventVendorStartUnused:
        break;
    case OMX_EventKhronosExtensions:
        break;
    }                           /* end of switch */

    return eError;
}

/* ========================================================================== */
/**
* AENC_FillBufferDone() : This method handles the fill buffer done event
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

static OMX_ERRORTYPE
AENC_FillBufferDone(OMX_HANDLETYPE hComponent,
                    OMX_PTR ptrAppData, OMX_BUFFERHEADERTYPE * pBuffer)
{
    AENC_Client *pAppData = ptrAppData;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_S32 retval = 0;
    gOutputEventOccured = 0;
    retval = write(pAppData->OpBuf_Pipe[1], &pBuffer, sizeof(pBuffer));

    if (retval != sizeof(pBuffer)) {
        eError = OMX_ErrorNotReady;
		DBG_PRINT( "FBD error: %d / %d\n", (int) retval, sizeof(pBuffer));
        return eError;
    }
    return eError;
}

/* ========================================================================== */
/**
* AENC_EmptyBufferDone() : This method handles the Empty buffer done event
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
static int gEOF = 0, gbytesInInputBuffer = INPUT_BUF_SIZE;
static OMX_ERRORTYPE
AENC_EmptyBufferDone(OMX_HANDLETYPE hComponent,
                     OMX_PTR ptrAppData, OMX_BUFFERHEADERTYPE * pBuffer)
{
    AENC_Client *pAppData = ptrAppData;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_S32 retval = 0;

#ifdef AENC_LINUX_CLIENT
#ifdef SRCHANGES
    pBuffer->pBuffer = SharedRegion_getPtr(pBuffer->pBuffer);
#endif
#endif
    retval = write(pAppData->IpBuf_Pipe[1], &pBuffer, sizeof(pBuffer));

    if (retval != sizeof(pBuffer)) {
		DBG_PRINT( "EBD error: %d / %d\n", (int) retval, sizeof(pBuffer));
        eError = OMX_ErrorNotReady;
        return eError;
    }


    return eError;
}


/******************************************************************************/
/* Main entrypoint into the Test */
/******************************************************************************/
static void *thrdProcess(void *arg)
{
    AENC_Client *pAppData = (AENC_Client *)arg;
    OMX_HANDLETYPE pHandle;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_BUFFERHEADERTYPE *pBufferIn = NULL;
    OMX_BUFFERHEADERTYPE *pBufferOut = NULL;
    OMX_U32 i;
    OMX_U32 nRead = 0;
    OMX_CALLBACKTYPE appCallbacks;

    OMX_U32 pRetrievedEvents;
    OMX_U32 frames_encoded;
    int frameLen;
    struct timeval timeout;

    OMX_S32 nfds = 0;
    fd_set rd, wr, er;
    FD_ZERO(&wr);
    FD_ZERO(&er);

	int nChannels = pAppData->nChannels;
	int bitrate	  = pAppData->bitrate;
    int samplerate = pAppData->samplerate;

	DBG_LOG(DBGLVL_TRACE, ("Begin"));
    appCallbacks.EventHandler = AENC_EventHandler;
    appCallbacks.EmptyBufferDone = AENC_EmptyBufferDone;
    appCallbacks.FillBufferDone = AENC_FillBufferDone;

    frames_encoded = 0;
    bEOS_Sent = OMX_FALSE;

    port_sem = (semp_t *) malloc(sizeof(semp_t));
    semp_init(port_sem, 0);

    state_sem = (semp_t *) malloc(sizeof(semp_t));
    semp_init(state_sem, 0);

   
    /* Allocate memory for the structure fields present in the pAppData(AENC_Client) */
    eError = AENC_AllocateResources(pAppData);
    if (eError != OMX_ErrorNone) {
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    pAppData->eState = OMX_StateInvalid;
    *pAppData->pCb = appCallbacks;


    /* Load the AENC Component */
    eError = OMX_GetHandle(&pHandle, (OMX_STRING) "OMX.TI.DSP.AUDENC"
                           /*StrAENCEncoder */ , pAppData, pAppData->pCb);
    if ((eError != OMX_ErrorNone) || (pHandle == NULL)) {
        printf("Couldn't get a handle\n");
        goto EXIT;
    }


    pAppData->pHandle = pHandle;
    pAppData->pComponent = (OMX_COMPONENTTYPE *) pHandle;

    AENC_SetParamPortDefinition(pAppData);

    OMX_SendCommand(pAppData->pHandle, OMX_CommandPortEnable,
                    OMX_AUDENC_INPUT_PORT, NULL);


    /* Wait for initialization to complete.. Wait for port enable of component  */
    semp_pend(port_sem);

    OMX_SendCommand(pAppData->pHandle, OMX_CommandPortEnable,
                    OMX_AUDENC_OUTPUT_PORT, NULL);
    /* Wait for initialization to complete.. Wait for port enable of component  */
    semp_pend(port_sem);

    if (eError != OMX_ErrorNone) {
        goto EXIT;
    }

    eError =
        OMX_GetParameter(pHandle, OMX_IndexParamPortDefinition,
                         pAppData->pInPortDef);
    if (eError != OMX_ErrorNone) {
        goto EXIT;
    }


    eError =
        OMX_GetParameter(pHandle, OMX_IndexParamPortDefinition,
                         pAppData->pOutPortDef);
    if (eError != OMX_ErrorNone) {
        goto EXIT;
    }

    /* OMX_SendCommand expecting OMX_StateIdle */
    eError = OMX_SendCommand(pHandle, OMX_CommandStateSet, OMX_StateIdle, NULL);

    if (eError != OMX_ErrorNone) {
        goto EXIT;
    }

    /* Allocate I/O Buffers */

    for (i = 0; i < pAppData->pInPortDef->nBufferCountActual; i++) {
        eError = OMX_AllocateBuffer(pHandle,
                                    &pAppData->pInBuff[i],
                                    pAppData->pInPortDef->nPortIndex, pAppData,
                                    pAppData->pInPortDef->nBufferSize);
    }

    for (i = 0; i < pAppData->pOutPortDef->nBufferCountActual; i++) {
        eError = OMX_AllocateBuffer(pHandle,
                                    &pAppData->pOutBuff[i],
                                    pAppData->pOutPortDef->nPortIndex,
                                    pAppData,
                                    pAppData->pOutPortDef->nBufferSize);
    }

    /* Wait for initialization to complete.. Wait for Idle state of component  */
    semp_pend(state_sem);
    /*check for error event */
    FD_ZERO(&rd);
    FD_SET(pAppData->Event_Pipe[0], &rd);
    //nfds = MAX (nfds, pAppData->Event_Pipe[0]);
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    select(pAppData->Event_Pipe[0] + 1, &rd, &wr, &er, &timeout);

    if (FD_ISSET(pAppData->Event_Pipe[0], &rd)) {
        read(pAppData->Event_Pipe[0], &pRetrievedEvents,
             sizeof(pRetrievedEvents));
        if (pRetrievedEvents == AENC_ENCODER_ERROR_EVENT) {
            printf
                ("Encoder returned an error while creating. Check your input parameters\n");
            goto FREE_HANDLE;
        }
    }

    eError =
        OMX_SendCommand(pHandle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    if (eError != OMX_ErrorNone) {
        goto FREE_HANDLE;
    }

    semp_pend(state_sem);

    for (i = 0; i < pAppData->pInPortDef->nBufferCountActual; i++) {
        nRead = AENC_FillData(pAppData, pAppData->pInBuff[i]);
        if (!nRead) {
			DBG_LOG(DBGLVL_ERROR,  ("No data Exiting."));
            goto EXIT1;
        }

        eError =  pAppData->pComponent->EmptyThisBuffer(pHandle,  pAppData->pInBuff[i]);
        if (eError != OMX_ErrorNone) {
			DBG_LOG(DBGLVL_ERROR,  ("EmptyThisBuffer: Failed."));
            goto EXIT1;
        }
    }

    for (i = 0; i < pAppData->pOutPortDef->nBufferCountActual; i++) {
        eError = pAppData->pComponent->FillThisBuffer(pHandle, pAppData->pOutBuff[i]);
        if (eError != OMX_ErrorNone) {
			DBG_LOG(DBGLVL_ERROR,  ("FillThisBuffer: Failed."));
            goto EXIT1;
        }
    }

    eError = OMX_GetState(pHandle, &pAppData->eState);

    /* Initialize the number of encoded frames to zero */
    pAppData->nEncodedFrms = 0;
	pAppData->nStatPrevFrames = 0;
	pAppData->StatPrevClk = 0;
	pAppData->nCrntFrameLen = 0;

    while ((eError == OMX_ErrorNone) && (pAppData->eState != OMX_StateIdle) && pAppData->m_fStream) {
        FD_ZERO(&rd);
        FD_SET(pAppData->OpBuf_Pipe[0], &rd);
        nfds = MAX(nfds, pAppData->OpBuf_Pipe[0]);
        FD_SET(pAppData->IpBuf_Pipe[0], &rd);
        nfds = MAX(nfds, pAppData->IpBuf_Pipe[0]);
        FD_SET(pAppData->Event_Pipe[0], &rd);
        nfds = MAX(nfds, pAppData->Event_Pipe[0]);

        select(nfds + 1, &rd, &wr, &er, NULL);

        if (FD_ISSET(pAppData->Event_Pipe[0], &rd)) {

            read(pAppData->Event_Pipe[0], &pRetrievedEvents, sizeof(pRetrievedEvents));
            if (pRetrievedEvents == AENC_ENCODER_END_OF_STREAM) {
				DBG_LOG(DBGLVL_ERROR,  (" End of stream processed"));
                break;
            } else if (pRetrievedEvents & AENC_ENCODER_ERROR_EVENT) {
                eError = OMX_ErrorUndefined;
				DBG_LOG(DBGLVL_ERROR,  (" OMX_ErrorUndefined."));
                break;
            }
        }

        if ((FD_ISSET(pAppData->OpBuf_Pipe[0], &rd))) {
            gOutputEventOccured = 1;
            /*read from the pipe */
            read(pAppData->OpBuf_Pipe[0], &pBufferOut, sizeof(pBufferOut));

			SendOutputData(pAppData, pBufferOut->pBuffer, pBufferOut->nFilledLen);

            eError = pAppData->pComponent->FillThisBuffer(pHandle, pBufferOut);
            if (eError != OMX_ErrorNone) {
                DBG_LOG(DBGLVL_ERROR,("error from ftb %x \n", eError));
                goto EXIT1;
            }
            pAppData->nEncodedFrms++;
			pAppData->nCrntFrameLen = pBufferOut->nFilledLen;
			ShowStats(pAppData);
        }

        if ((FD_ISSET(pAppData->IpBuf_Pipe[0], &rd))) {
            /*read from the pipe */
            read(pAppData->IpBuf_Pipe[0], &pBufferIn, sizeof(pBufferIn));

            if (pBufferIn != NULL) {
                nRead = AENC_FillData(pAppData, pBufferIn);
                eError = pAppData->pComponent->EmptyThisBuffer(pHandle, pBufferIn);
            }
        }
        eError = OMX_GetState(pHandle, &pAppData->eState);
    }

EXIT1:
    printf("\nGoing to Idle\n");
    eError = OMX_SendCommand(pHandle, OMX_CommandStateSet, OMX_StateIdle, NULL);
    if (eError != OMX_ErrorNone) {
        goto EXIT;
    }

	semp_pend(state_sem);
	printf("Going to Loaded\n");
	eError = OMX_SendCommand(pHandle, OMX_CommandStateSet, OMX_StateLoaded, NULL);

	printf("Freeing buffers\n");
	for (i = 0; i < pAppData->pInPortDef->nBufferCountActual; i++) {
		eError = OMX_FreeBuffer(pHandle, pAppData->pInPortDef->nPortIndex, pAppData->pInBuff[i]);
		if (eError != OMX_ErrorNone) {
			goto EXIT;
		}
	}

	for (i = 0; i < pAppData->pOutPortDef->nBufferCountActual; i++) {
		eError =  OMX_FreeBuffer(pHandle, pAppData->pOutPortDef->nPortIndex,  pAppData->pOutBuff[i]);
		if (eError != OMX_ErrorNone) {
			goto EXIT;
		}
	}

	semp_pend(state_sem);
    printf("Component transitioned from Idle to Loaded\n");

FREE_HANDLE:
	/* UnLoad the Encoder Component */
	eError = OMX_FreeHandle(pHandle);
	if ((eError != OMX_ErrorNone)) {
		goto EXIT;
	}

	semp_deinit(port_sem);
	free(port_sem);
	port_sem = NULL;

	semp_deinit(state_sem);
	free(state_sem);
	state_sem = NULL;

EXIT:
	if (pAppData) {
		AENC_FreeResources(pAppData);
	}

	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return (0);
}                               /* OMX_Audio_Encode_Test */


int aencchainStart(StrmCompIf *pComp)
{
	AENC_Client *pCtx = (AENC_Client *)pComp->pCtx;
	pCtx->m_fStream = 1;
	oalThreadCreate(&pCtx->thrdHandle, thrdProcess,pCtx);
	return 0;
}


int aencchainSetOption(
	StrmCompIf *pComp, 	
	int         nCmd, 
	char       *pOptionData)
{
	int ret;
	AENC_Client *pAppData = (AENC_Client *)pComp->pCtx;
#if 0
	if(nCmd == AUD_DEC_CMD_SET_PARAMS) {
		IL_AUD_ARGS  *pArgs = (IL_AUD_ARGS  *)pOptionData;

		strncpy(pAppData->codec_name, pArgs->codec_name, 31);
		pAppData->aacRawFormat = pArgs->nRrawFormat; 
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


	}
#endif
#if 0
    if (!strcmp(outputFormat, "RAW")) {
        pAppData->outputformat = OMX_AUDIO_AACStreamFormatRAW;
    } else if (!strcmp(outputFormat, "ADTS")) {
        pAppData->outputformat = OMX_AUDIO_AACStreamFormatMP4ADTS;
    } else if (!strcmp(outputFormat, "ADIF")) {
        pAppData->outputformat = OMX_AUDIO_AACStreamFormatADIF;
    }
#endif
	return 0;
}


static int aencchainOpen(StrmCompIf *pComp, const char *pszResource)
{
	int ret;
	AENC_Client *pAppData = (AENC_Client *)pComp->pCtx;
	return 0;
}


void aencchainSetClkSrc(StrmCompIf *pComp, void *pClk)
{
	AENC_Client *pAppData = (AENC_Client *)pComp->pCtx;
	pAppData->pClk = pClk;
}

void *aencchainGetClkSrc(StrmCompIf *pComp)
{
	AENC_Client *pAppData = (AENC_Client *)pComp->pCtx;
	pAppData->fClkSrc= 1;
	pAppData->pClk = clkCreate(1);
	return pAppData->pClk;
}

void aencchainSetInputConn(StrmCompIf *pComp, int nConnNum, ConnCtxT *pConn)
{
	AENC_Client *pAppData = (AENC_Client *)pComp->pCtx;
	pAppData->pConnSrc = pConn;
}

void aencchainSetOutputConn(StrmCompIf *pComp, int nConnNum, ConnCtxT *pConn)
{
	DBG_LOG(DBGLVL_TRACE, ("Begin"));
	AENC_Client *pAppData = (AENC_Client *)pComp->pCtx;
	pAppData->pConnDest = pConn;
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
}

int aencchainStop(StrmCompIf *pComp)
{
	AENC_Client *pAppData = (AENC_Client *)pComp->pCtx;
	pAppData->m_fStream = 0;
	int nTimeout = 300000;
	DBG_LOG(DBGLVL_SETUP, ("Waiting for stream stop: beign"));
	oalThreadJoin(pAppData->thrdHandle, 1000);
	DBG_LOG(DBGLVL_SETUP, ("Waiting for stream stop: end"));
	return 0;
}

int aencchainDelete(StrmCompIf *pComp)
{
	DBG_LOG(DBGLVL_SETUP, ("Enter"));
	if(pComp && pComp->pCtx) {
		DBG_LOG(DBGLVL_SETUP, ("Free AENC_Client"));
		AENC_Client *pAppData = (AENC_Client *)pComp->pCtx;
		free (pAppData);
	}

	if(pComp) {
		DBG_LOG(DBGLVL_SETUP, ("Free StrmCompIf"));
		free(pComp);
	}
	DBG_LOG(DBGLVL_SETUP, ("Leave"));
	return 0;
}

#define AUD_DEFAULT_INPUT_PKT_SIZE (1024 * 2 * 16) // 16 bit 2 channel 1024 sample frame
StrmCompIf *
aencchainCreate()
{
	DBG_LOG(DBGLVL_TRACE, ("Begin"));
	StrmCompIf *pComp = (StrmCompIf *)malloc(sizeof(StrmCompIf));
	memset (pComp, 0x0, sizeof (StrmCompIf));
	AENC_Client *pAppData = (AENC_Client *) malloc (sizeof (AENC_Client));
	memset (pAppData, 0x0, sizeof (AENC_Client));
	pComp->pCtx = pAppData;
	pAppData->max_input_pkt_size = AUD_DEFAULT_INPUT_PKT_SIZE;

	pAppData->nChannels = 2; //default
	pAppData->bitrate = 128000;
    pAppData->samplerate = 48000;
	pAppData->eCompressionFormat = OMX_AUDIO_CodingAAC;
	pAppData->outputformat = OMX_AUDIO_AACStreamFormatMP4ADTS;
    

	pComp->Open= aencchainOpen;
	pComp->SetOption = aencchainSetOption;
	pComp->SetInputConn= aencchainSetInputConn;
	pComp->SetOutputConn= aencchainSetOutputConn;
	pComp->SetClkSrc = aencchainSetClkSrc;
	pComp->GetClkSrc = aencchainGetClkSrc;
	pComp->Start = aencchainStart;
	pComp->Stop = aencchainStop;
	pComp->Close = NULL;
	pComp->Delete = aencchainDelete;
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return pComp;
}
