#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "dbglog.h"
/*-------------------------program files -------------------------------------*/

#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include "ti/omx/interfaces/openMaxv11/OMX_Core.h"
#include "ti/omx/interfaces/openMaxv11/OMX_Component.h"
#include "OMX_TI_Common.h"
#include <omx_vdec.h>
#include <omx_vfpc.h>
#include <omx_vfdc.h>
#include <omx_ctrl.h>
#include <omx_vswmosaic.h>
#include "msgq.h"

#include "vdec_ilclient.h"
#include "vdec_ilclient_utils.h"
#include "vdec_es_parser.h"
#include "dec_clock.h"
#include "vdec_omx_chain.h"
#include "minini.h"

#define DEF_TS_BLOCK	(188 * 21)

OMX_U8 PADX;
OMX_U8 PADY;


#define DEI_PORT2_MAX_WIDTH     1920
#define DEI_PORT2_MAX_HEIGHT    1080

static void IL_ClientDeInit (IL_Client *pAppData);

static OMX_BUFFERHEADERTYPE *AllocateHostBuffer(int nBufferSize);
static void FreeHostBuffer(OMX_BUFFERHEADERTYPE *pBufferHdr);

void ConfigOmxComponentDebugLevel(int fEnableStatusLog, int nDebugLevel)
{
	static int initDone = 0;
	DBG_LOG(DBGLVL_TRACE, ("Enter"));

	if(!initDone) {
		ConfigureUIA uiaCfg;
		/* Configuring logging options on slave cores */
		/* can be 0 or 1 */
		initDone = 1;
		uiaCfg.enableAnalysisEvents = 0;

		/* can be 0 or 1 */
		uiaCfg.enableStatusLogger = fEnableStatusLog;

		/* can be OMX_DEBUG_LEVEL1|2|3|4|5 */
		uiaCfg.debugLevel = nDebugLevel;//OMX_DEBUG_LEVEL1;

		/* configureUiaLoggerClient( COREID, &Cfg); */
		configureUiaLoggerClient(2, &uiaCfg);
		configureUiaLoggerClient(1, &uiaCfg);
	}
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
}



/* ========================================================================== */
/**
* IL_ClientFillBitStreamData() : Function to parse a frame and copy it to 
*                                decoder input buffer.
*
* @param pAppData   : application data structure
* @param pbuf       : OMX buffer header, it has pointer to actual buffer
*
*  @return      
*  String conversion of the OMX_ERRORTYPE
*
*/
/* ========================================================================== */

unsigned int IL_ClientFillBitStreamData (IL_Client *pAppData,  OMX_BUFFERHEADERTYPE *pBuf, unsigned long *pulFlags)
{
	unsigned int dataRead = 0;
	int frameSize = 0;
	*pulFlags = 0;
	/* update the parser buffer, with the decoder input buffer */
	if(pAppData->codingType == OMX_VIDEO_CodingAVC)	{
		//DBG_PRINT("AllocLen=%d\n", pBuf->nAllocLen);
		pAppData->H264Parser.outBuf.ptr = pBuf->pBuffer;
		pAppData->H264Parser.outBuf.bufsize = pBuf->nAllocLen;
		pAppData->H264Parser.outBuf.bufused = 0;
		pAppData->H264Parser.outBuf.nFlags = 0;
		frameSize = Decode_GetNextH264FrameSize (&pAppData->H264Parser);
		if(pAppData->useDemux) {
			pBuf->nFlags = pAppData->H264Parser.outBuf.nFlags;
			pBuf->nTimeStamp = pAppData->H264Parser.outBuf.nTimeStamp;
		}
		if(pAppData->H264Parser.fDisCont) {
			*pulFlags = OMX_EXT_BUFFERFLAG_DISCONT;
		}
	} else if(pAppData->codingType == OMX_VIDEO_CodingMPEG4) {
		pAppData->pcmpeg4.buff_in = pBuf->pBuffer;
		frameSize = Decode_GetNextMpeg4FrameSize (&pAppData->pcmpeg4);
	} else if(pAppData->codingType == OMX_VIDEO_CodingMPEG2) {
		pAppData->pcmpeg2.buff_in = pBuf->pBuffer;
		frameSize = Decode_GetNextMpeg2FrameSize (&pAppData->pcmpeg2);
		if(pAppData->useDemux) {
			pBuf->nFlags = pAppData->pcmpeg2.nCrntFrameFlags;
			pBuf->nTimeStamp = pAppData->pcmpeg2.nCrntFrameTimeStamp;
		}
	}

	/* Get the size of one frame at a time */
	dataRead = frameSize;

	/* Update the buffer header with buffer filled length and alloc length */
	pBuf->nFilledLen = frameSize;
	//DBG_PRINT("Filled=%d\n", pBuf->nFilledLen);
	return frameSize;
}

unsigned int IL_ClientAbortFillBitStreamData(IL_Client *pAppData)
{
	if(pAppData->codingType == OMX_VIDEO_CodingAVC)	{
		Decode_AbortGetNextH264Frame(&pAppData->H264Parser);
	} else if(pAppData->codingType == OMX_VIDEO_CodingMPEG4) {
		Decode_AbortGetNextMpeg2Frame(&pAppData->pcmpeg2);
	}
}	

/* ========================================================================== */
/**
* IL_ClientDecUseInitialInputResources() : This function gives initially all
*                                          input buffers to decoder component.
*                                          after consuming decoder would keep
*                                          in ipbufpipe for file read thread. 
*
* @param pAppdata   : application data structure
*
*  @return      
*  String conversion of the OMX_ERRORTYPE
*
*/
/* ========================================================================== */

OMX_ERRORTYPE IL_ClientDecUseInitialInputResources (IL_Client *pAppdata)
{

	OMX_ERRORTYPE err = OMX_ErrorNone;
	unsigned int i = 0;
	int frameSize = 0;
	unsigned long long nTimeStamp = 0;
	OMX_U32 nFlags = 0;
	IL_CLIENT_COMP_PRIVATE *decILComp = NULL;
	decILComp = ((IL_Client *) pAppdata)->decILComp;
	IL_CLIENT_INPORT_PARAMS *inPortParamsPtr;

  /* Give input buffers to component which is limited by no of input buffers
     available. Rest of the data will be read on the callback from input data
     read thread */
	for (i = 0; i < decILComp->inPortParams->nBufferCountActual; i++)	{

		if(pAppdata->nUiCmd == STRM_CMD_STOP){
			break;
		}

		if(pAppdata->codingType == OMX_VIDEO_CodingAVC)	{
			pAppdata->H264Parser.outBuf.ptr = decILComp->inPortParams->pInBuff[i]->pBuffer;
			pAppdata->H264Parser.outBuf.bufsize = decILComp->inPortParams->nBufferSize;
			pAppdata->H264Parser.outBuf.bufused = 0;
			DBG_LOG(DBGLVL_FRAME, ("Get Filled frame %d...", i));
			frameSize = Decode_GetNextH264FrameSize (&pAppdata->H264Parser);
			nTimeStamp = pAppdata->H264Parser.outBuf.nTimeStamp;
			nFlags = pAppdata->H264Parser.outBuf.nFlags;
		} else if(pAppdata->codingType == OMX_VIDEO_CodingMPEG4) {
			 pAppdata->pcmpeg4.buff_in = decILComp->inPortParams->pInBuff[i]->pBuffer;
			 frameSize = Decode_GetNextMpeg4FrameSize (&pAppdata->pcmpeg4);
		 }  else if(pAppdata->codingType == OMX_VIDEO_CodingMPEG2) {
			 pAppdata->pcmpeg2.buff_in = decILComp->inPortParams->pInBuff[i]->pBuffer;
			 DBG_LOG(DBGLVL_FRAME, ("Get Filled frame %d...", i));
			 frameSize = Decode_GetNextMpeg2FrameSize (&pAppdata->pcmpeg2);
			 nTimeStamp = pAppdata->H264Parser.outBuf.nTimeStamp;
			 nFlags = pAppdata->H264Parser.outBuf.nFlags;
		 }

		/* Exit the loop if no data available */
		if (!frameSize)	{
			DBG_LOG(DBGLVL_FRAME, ("Got empty frame %d...", i));
			err = OMX_ErrorNoMore;
			break;
		}

		decILComp->inPortParams->pInBuff[i]->nFlags = nFlags;
		decILComp->inPortParams->pInBuff[i]->nTimeStamp = nTimeStamp;

		decILComp->inPortParams->pInBuff[i]->nFilledLen = frameSize;
		decILComp->inPortParams->pInBuff[i]->nOffset = 0;
		//decILComp->inPortParams->pInBuff[i]->nAllocLen = frameSize;
		decILComp->inPortParams->pInBuff[i]->nInputPortIndex = 0;

		/* Pass the input buffer to the component */
		inPortParamsPtr = decILComp->inPortParams + decILComp->inPortParams->pInBuff[i]->nInputPortIndex;

		DBG_LOG(DBGLVL_FRAME, ("Sending Frame=%d to decoder", inPortParamsPtr->nFramesToM3));
		err = OMX_EmptyThisBuffer (decILComp->handle, decILComp->inPortParams->pInBuff[i]);
		inPortParamsPtr->nFramesToM3++;
	}
	return err;
}

/* ========================================================================== */
/**
* IL_ClientInputBitStreamReadTask() : This task function is file read task for
* decoder component. This function calls parser functions, which provides frames
* in each buffer to be consumed by decoder.
*
* @param threadsArg        : Handle to the application
*
*/
/* ========================================================================== */


void
IL_ClientInputBitStreamReadTask (void *threadsArg)
{
	unsigned int dataRead = 0;
	OMX_ERRORTYPE err = OMX_ErrorNone;
	unsigned long ulFlags = 0;
	IL_CLIENT_COMP_PRIVATE *decILComp = NULL;
	OMX_BUFFERHEADERTYPE *pBufferIn = NULL;
	IL_Client *pAppData = (IL_Client *)threadsArg;
	decILComp = ((IL_Client *) threadsArg)->decILComp;

	DBG_LOG(DBGLVL_TRACE, ("Enter"));

	pAppData->fStreaming = OMX_TRUE;

	/* use the initial i/p buffers and make empty this buffer calls */
	err = IL_ClientDecUseInitialInputResources (threadsArg);
	if(err != OMX_ErrorNone) {
		DBG_LOG(DBGLVL_SETUP, ("Failed to get initial input."));
		goto NoData;
	}
	DBG_LOG(DBGLVL_TRACE, ("Streaming loop start"));
	while (pAppData->nUiCmd != STRM_CMD_STOP)	{
		/* Read empty buffer pointer from the pipe */
		DBG_LOG(DBGLVL_FRAME, ("Get empty frame..."));
		read (decILComp->inPortParams->ipBufPipe[0],&pBufferIn, sizeof (pBufferIn));
		/* Fill the data in the empty buffer */
		DBG_LOG(DBGLVL_FRAME, ("Fill the frame..."));
		dataRead = IL_ClientFillBitStreamData (pAppData, pBufferIn, &ulFlags);
		/* Exit the loop if no data available */
		if ((dataRead <= 0) || pBufferIn->nFlags & OMX_BUFFERFLAG_EOS || pAppData->nUiCmd == STRM_CMD_STOP)	{
			DBG_LOG(DBGLVL_TRACE, ("No data or Stop: framesize=%d nUiCmd=%d", dataRead, pAppData->nUiCmd));
			pBufferIn->nFlags |= OMX_BUFFERFLAG_EOS;

			decILComp->inPortParams->nFramesToM3++;
			err = OMX_EmptyThisBuffer (decILComp->handle, pBufferIn);
			break;
		}
		decILComp->inPortParams->nFramesToM3++;

		if(ulFlags &  OMX_EXT_BUFFERFLAG_DISCONT) {
#if 0
			DBG_LOG(DBGLVL_SETUP, ("Disconinuity. Issuing Flush!!!"));
			OMX_SendCommand (decILComp->handle, OMX_CommandFlush, OMX_ALL, NULL);

			if (semp_timedpend(decILComp->done_sem, 1000) == -1) {
				DBG_LOG(DBGLVL_ERROR, ("Waitfor Flush completion : Failed!"));
				NotifyOmxFatalError(decILComp->pParent, -1);
				break;
			}
			if (semp_timedpend(decILComp->done_sem, 1000) == -1) {
				DBG_LOG(DBGLVL_ERROR, ("Waitfor Second Flush completion : Failed. Ignoring!"));
				break;
			}
			usleep(2*1000*1000);
#endif
		}

		/* Pass the input buffer to the component */
		DBG_LOG(DBGLVL_FRAME, ("Sending Frame=%d", decILComp->inPortParams->nFramesToM3));
		err = OMX_EmptyThisBuffer (decILComp->handle, pBufferIn);
		
		if (OMX_ErrorNone != err) {
			/* put back the frame in pipe and wait for state change */
			write (decILComp->inPortParams->ipBufPipe[1], &pBufferIn, sizeof (pBufferIn));
			DBG_LOG(DBGLVL_TRACE, ("OMX_ErrorNone != err"));
			break;
		}
	}
	DBG_LOG(DBGLVL_TRACE, ("Streaming loop end"));

NoData:
	pAppData->fStreaming = OMX_FALSE;
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	pthread_exit (decILComp);
}

// Debug routine to test demux speed
void
IL_ClientInputBitStreamReadTaskTest (void *threadsArg)
{
	unsigned int dataRead = 0;
	OMX_ERRORTYPE err = OMX_ErrorNone;
	unsigned long ulFlags;
	OMX_BUFFERHEADERTYPE BufferIn = {0};
	OMX_BUFFERHEADERTYPE *pBufferIn = &BufferIn;
	IL_Client *pAppData = (IL_Client *)threadsArg;
	
	pAppData->fStreaming = OMX_TRUE;

	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	pBufferIn->pBuffer = (char *)malloc(2 * 1024 * 1024);
	pBufferIn->nAllocLen = 2 * 1024 * 1024;
	int nFramesToM3 = 0;
	while (1)	{
		dataRead = IL_ClientFillBitStreamData (pAppData, pBufferIn, &ulFlags);

		{
			char szClck[256];
			char szPts[256];

			CLOCK_T pts = pBufferIn->nTimeStamp;
			CLOCK_T clk = ClockGetInternalTime(pAppData->pClk);
			if(clk - pAppData->prev_clk >= TIME_SECOND) {
				Clock2HMSF(clk, szClck, 255);
				Clock2HMSF(pts, szPts, 255);
				DBG_PRINT("<@%s: frame=%d size=%d rate=%0.2f pts=%s>\n", szClck,nFramesToM3, dataRead, 1.0 * (nFramesToM3 - pAppData->prev_frame_count) / ((clk - pAppData->prev_clk) / TIME_SECOND), szPts);
				pAppData->prev_frame_count = nFramesToM3;
				pAppData->prev_clk = clk;
			}
		}

		/* Exit the loop if no data available */
		if ((dataRead <= 0) || pBufferIn->nFlags & OMX_BUFFERFLAG_EOS)	{
			DBG_LOG(DBGLVL_TRACE, ("No data or Stop: framesize=%d nUiCmd=%d", dataRead, pAppData->nUiCmd));
			pBufferIn->nFlags |= OMX_BUFFERFLAG_EOS;
			nFramesToM3++;
			break;
		}
		nFramesToM3++;
	}
NoData:
	pAppData->fStreaming = OMX_FALSE;
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
}



static void DoSync(IL_Client *pCtx, CLOCK_T pts, int *fDrop, CLOCK_T nMaxWait, CLOCK_T nMaxLateness)
{
	*fDrop = 0;
	
	if(pCtx->pClk == NULL)
		return;

	if(IsClockRunning(pCtx->pClk))  {
		CLOCK_T clock = ClockGetTime(pCtx->pClk);

		if(pts > clock) {
			CLOCK_T wait_time = pts - clock ;
			if(wait_time > nMaxWait)
				wait_time = nMaxWait;
			if(wait_time > 0){
				DBG_LOG(DBGLVL_FRAME, ("Wait=%lld us", wait_time));
				WaitForClock(pCtx->pClk, pts, wait_time);
			}
		} else if(pts > 0 && pts < clock - nMaxLateness ) {
			*fDrop = 1;
		} else {
			DBG_LOG(DBGLVL_FRAME, ("Display the frame."));
		}
	} else {
		DBG_LOG(DBGLVL_STAT, ("Clock not running yet!"));
	}
}


void DropBuffer(IL_CLIENT_COMP_PRIVATE *thisComp, IL_CLIENT_PIPE_MSG *pipeMsg)
{

  OMX_ERRORTYPE err = OMX_ErrorNone;
  OMX_BUFFERHEADERTYPE *pBufferIn;

  // DO Dummy processing:
  /* search its own buffer header based on submitted by connected comp */
  IL_ClientUtilGetSelfBufHeader (thisComp, pipeMsg->bufHeader.pBuffer, ILCLIENT_INPUT_PORT,
                                 pipeMsg->bufHeader.nInputPortIndex, &pBufferIn);

  /* populate buffer header */
  pBufferIn->nFilledLen = pipeMsg->bufHeader.nFilledLen;
  pBufferIn->nOffset = pipeMsg->bufHeader.nOffset;
  pBufferIn->nTimeStamp = pipeMsg->bufHeader.nTimeStamp;
  pBufferIn->nFlags = pipeMsg->bufHeader.nFlags;
  pBufferIn->hMarkTargetComponent = pipeMsg->bufHeader.hMarkTargetComponent;
  pBufferIn->pMarkData = pipeMsg->bufHeader.pMarkData;
  pBufferIn->nTickCount = 0;

  /* call etb to the component */
  //err = OMX_EmptyThisBuffer (thisComp->handle, pBufferIn);
  // DO Dummy callback

  IL_ClientCbEmptyBufferDone(thisComp->handle, thisComp, pBufferIn);
}

int IsScalerComponent(IL_CLIENT_COMP_PRIVATE *thisComp)
{
	if(strcmp(thisComp->comp_name, "OMX.TI.VPSSM3.VFPC.DEIMDUALOUT") == 0 || strcmp(thisComp->comp_name, "OMX.TI.VPSSM3.VFPC.INDTXSCWB") == 0)
		return 1;
	return 0;
}





//=====================================================================================================================
#define USE_SINGLE_MALLOC_FOR_A8_PORT_BUF

OMX_BUFFERHEADERTYPE *AllocateHostBuffer(int nBufferSize)
{
	OMX_BUFFERHEADERTYPE *pBuffHdr = (OMX_BUFFERHEADERTYPE *)malloc(sizeof(OMX_BUFFERHEADERTYPE));
	memset(pBuffHdr, 0x00, sizeof(OMX_BUFFERHEADERTYPE));
	pBuffHdr->pBuffer = (OMX_U8*)malloc(nBufferSize);
	pBuffHdr->nAllocLen = nBufferSize;
	return pBuffHdr;
}





//=====================================================================================================================
int vidcahinSetOption(
	struct _StrmCompIf *pComp, 
	int                nCmd, 
	char               *pOptionData)
{

	IL_Client *pAppData = (IL_Client *)pComp->pCtx;

	if(nCmd == DEC_CMD_SET_PARAMS) {
		OMX_VIDEO_CODINGTYPE coding;
		IL_VID_ARGS *args = (IL_VID_ARGS *)pOptionData;

		pAppData->buffer = args->buffer;
		if(!strcmp(args->codec_name,"h264")){
			PADX = 32;
			PADY = 24;
			coding = OMX_VIDEO_CodingAVC;
		} else if(!strcmp(args->codec_name,"mpeg4")) {
			PADX = 16;
			PADY = 16;
			coding = OMX_VIDEO_CodingMPEG4;
		} else if(!strcmp(args->codec_name,"mpeg2")) {
			PADX = 8;
			PADY = 8;
			coding = OMX_VIDEO_CodingMPEG2;
		}
		pAppData->codingType = coding; 
		pAppData->nSessionId = args->nInstanceId;
		pAppData->nDecHeight = args->dec_height;
		pAppData->nDecWidth = args->dec_width;
		pAppData->nDispHeight = args->disp_height;
		pAppData->nDispWidth = args->disp_width;

		pAppData->nDeiPort2Width = DEI_PORT2_MAX_WIDTH;//args->dec_width;
		pAppData->nDeiPort2Height = DEI_PORT2_MAX_HEIGHT;//args->dec_height;

		pAppData->useDemux = args->use_demux;
		pAppData->sync = args->sync;
		pAppData->nSyncMaxLateness = args->latency;
		pAppData->deinterlace = args->deinterlace;
	
		if(pAppData->IL_CLIENT_DECODER_MAX_FRAME_RATE) {
			// Force debug frame rate
			pAppData->nDecFrameRate = pAppData->IL_CLIENT_DECODER_MAX_FRAME_RATE;
		} else if(args->frame_rate) {
			// Application specified frame rate
			pAppData->nDecFrameRate = args->frame_rate;
		} else {
			//pAppData->nDecFrameRate = 120;
			pAppData->nDecFrameRate = 60;
		}

		if(pAppData->IL_CLIENT_DECODER_INPUT_BUFFER_COUNT){
			// Force debug buffer count
			pAppData->nDecInputBufferCount = pAppData->IL_CLIENT_DECODER_INPUT_BUFFER_COUNT;
		} else {
			if(coding == OMX_VIDEO_CodingAVC) {
				pAppData->nDecInputBufferCount = 10;
			} else if(coding == OMX_VIDEO_CodingMPEG2) {
				pAppData->nDecInputBufferCount = 10;
			}
		}

		if(pAppData->IL_CLIENT_DECODER_OUTPUT_BUFFER_COUNT){
			// Force debug buffer count
			pAppData->nDecOutputBufferCount = pAppData->IL_CLIENT_DECODER_OUTPUT_BUFFER_COUNT;
		} else {
			if(coding == OMX_VIDEO_CodingAVC) {
				pAppData->nDecOutputBufferCount = 10;
			} else if(coding == OMX_VIDEO_CodingMPEG2) {
				pAppData->nDecOutputBufferCount = 8;
			}
		}
	
		pAppData->nScalerInputBufferCount = pAppData->nDecOutputBufferCount;
		pAppData->strmCallback =  args->strmCallback;
		pAppData->pAppCtx = args->pAppCtx;
	}
	return 0;
}

void NotifyOmxFatalError(void *_pCtx, int nErr)
{
	DBG_LOG(DBGLVL_ERROR, ("!!! OMX Fatal error (Needs restart) !!!"));
	IL_Client *pCtx = (IL_Client *)_pCtx;
	if(pCtx->strmCallback) {
		DBG_LOG(DBGLVL_ERROR, ("Notifying OMX AL Client Application."));
		pCtx->strmCallback(pCtx->pAppCtx, STRM_EVENT_OMX_ERROR);
	}
}


int vidchainOpen(StrmCompIf *pComp, const char *szResource)
{
	OMX_U32 i, j;
	void *ret_value;
	int res = 0;

	int nScalerOutput1BuffSize;
	int nScalerOutput2BuffSize;

	IL_Client *pAppData = (IL_Client *)pComp->pCtx;
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	IL_CLIENT_PIPE_MSG pipeMsg;
	
	DBG_LOG(DBGLVL_TRACE, ("Enter"));

	pAppData->nUiCmd = 	STRM_CMD_RUN;
	pAppData->pVidMix = vmixGetInstance();
	pAppData->nDestType = vmixGetDestinationType(pAppData->pVidMix, pAppData->nSessionId);

	pAppData->nSwMosaicPort = vmixSwMosaicPort(pAppData->pVidMix, pAppData->nSessionId);

	DBG_LOG (DBGLVL_SETUP, ("nDestType=%d nSwMosaicPort=%d", pAppData->nDestType, pAppData->nSwMosaicPort));
	if(pAppData->IL_COMPONENT_DEBUG_LEVEL > 0) {
		ConfigOmxComponentDebugLevel(1, pAppData->IL_COMPONENT_DEBUG_LEVEL);
	}

	if(pAppData->useDemux) {
		pAppData->vparseILComp = IL_ClientCreateHostComponent(1, 0/*output retrieved by app*/);

		if(pAppData->codingType == OMX_VIDEO_CodingAVC) 	{
			Decode_H264ParserInit2(&pAppData->H264Parser, pAppData->pConnSrc);
		} else if(pAppData->codingType == OMX_VIDEO_CodingMPEG4) {
			// TODO:
			//Decode_Mpeg4ParserInit (&pAppData->pcmpeg4, pAppData->vparseILComp->localPipe);
		} else if(pAppData->codingType == OMX_VIDEO_CodingMPEG2) {
			Decode_Mpeg2ParserInit2 (&pAppData->pcmpeg2, pAppData->pConnSrc);
		}  
	}


  /* Initialize application specific data structures and buffer management
     data structure */

	pAppData->pCb.EventHandler = IL_ClientCbEventHandler;
	pAppData->pCb.EmptyBufferDone = IL_ClientCbEmptyBufferDone;
	pAppData->pCb.FillBufferDone = IL_ClientCbFillBufferDone;

	int nScalerOutPorts = (pAppData->deinterlace || pAppData->useDeiScalerAlways) ? 2 : 1;
	int nScalerInputBufferCount = (pAppData->deinterlace) ? (pAppData->nDecOutputBufferCount * 2) : pAppData->nDecOutputBufferCount;
	int nDecOutputBuffSize = (UTIL_ALIGN ((pAppData->nDecWidth + (2 * PADX)), 128) * ((((pAppData->nDecHeight + 15) & 0xfffffff0) + (4 * PADY))) * 3) >> 1;
	nScalerOutput2BuffSize = (pAppData->nDeiPort2Height * pAppData->nDeiPort2Width * 3) >> 1;
	//nScalerOutput1BuffSize = pAppData->nDecHeight * ((pAppData->nDecWidth + 15) & 0xfffffff0) * 2;
	pAppData->decILComp = CreateILCompWrapper(1, 1, OMX_VIDDEC_OUTPUT_PORT, pAppData->nDecInputBufferCount, pAppData->nDecWidth * pAppData->nDecHeight,
		pAppData->nDecOutputBufferCount, nDecOutputBuffSize);
	if(pAppData->nDestType != DEST_TYPE_NULL){
		vmixGetInputPortParam(pAppData->pVidMix, pAppData->nSessionId, &pAppData->nScalerOutWidth, &pAppData->nScalerOutHeight, &pAppData->nScalerOutputBufferCount);
		nScalerOutput1BuffSize = pAppData->nScalerOutWidth * ((pAppData->nScalerOutHeight + 15) & 0xfffffff0) * 2;
		pAppData->scILComp = CreateILCompWrapper(1, nScalerOutPorts, OMX_VFPC_OUTPUT_PORT_START_INDEX, nScalerInputBufferCount, nDecOutputBuffSize,
											pAppData->nScalerOutputBufferCount, nScalerOutput1BuffSize);
	}

	decoderInit(pAppData, pAppData->decILComp, &pAppData->pCb);
	IL_DecClientSetDecodeParams (pAppData);
	TRACE_PROGRESS
	if(pAppData->nDestType == DEST_TYPE_NULL) {
		DBG_LOG (DBGLVL_TRACE, ("nDestType == DEST_TYPE_NULL"));
		// Do nothing
	} else  {
		TRACE_PROGRESS
		//Overwrite port 2 params
		if(nScalerOutPorts == 2) {
			compSetOutPortParam(pAppData->scILComp, 1, pAppData->nScalerOutputBufferCount, nScalerOutput2BuffSize);
		}

		if(scalerInit(pAppData->scILComp,
				pAppData->deinterlace, pAppData->useDeiScalerAlways,
				pAppData->nDecWidth, pAppData->nDecHeight, pAppData->nDecStride, nDecOutputBuffSize, pAppData->nScalerInputBufferCount, OMX_COLOR_FormatYUV420SemiPlanar,
				pAppData->nScalerOutWidth, pAppData->nScalerOutHeight, pAppData->nScalerOutWidth * 2,  nScalerOutput1BuffSize, pAppData->nScalerOutputBufferCount, OMX_COLOR_FormatYCbYCr,
				pAppData->nDeiPort2Width, pAppData->nDeiPort2Height, pAppData->nDeiPort2Width, nScalerOutput2BuffSize, pAppData->nScalerOutputBufferCount, OMX_COLOR_FormatYUV420SemiPlanar, 
				&pAppData->pCb) != 0)
			goto EXIT;

		DBG_LOG (DBGLVL_TRACE, ("Connect:Dec and Scaler"));
		IL_ClientConnectComponents (pAppData->decILComp, OMX_VIDDEC_OUTPUT_PORT,  pAppData->scILComp, OMX_VFPC_INPUT_PORT_START_INDEX);
		DBG_LOG (DBGLVL_TRACE, ("Connect:Scaler and SwMosaic"));
		vmixConnectInputPort(pAppData->pVidMix, pAppData->nSwMosaicPort, pAppData->scILComp,  OMX_VFPC_OUTPUT_PORT_START_INDEX);
		DBG_LOG (DBGLVL_TRACE, ("Connect:Scaler and SwMosaic: Success"));
	}

EXIT:
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return 0;
}


int vidchainStart(StrmCompIf *pComp)
{
	int res = 0;
	IL_Client *pAppData = (IL_Client *)pComp->pCtx;
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	
	DBG_LOG(DBGLVL_TRACE, ("Enter"));

	compInitResource(pAppData->decILComp, 0);

	if(pAppData->nDestType == DEST_TYPE_NULL) {
	} else 	{
		compSetInportAllocationType(pAppData->scILComp, 0, 1);
		compSetOutPortAllocationType(pAppData->scILComp, 0, vmixGetInportParam(pAppData->pVidMix, 1));
		if(scalerAllocateResource(pAppData->scILComp, pAppData->deinterlace) != 0)
			goto EXIT;

		if(compSetStateExec(pAppData->scILComp, 0) != 0)
			goto EXIT;
	}

	if(compSetStateExec(pAppData->decILComp, 0) != 0)
		goto EXIT;

	pthread_attr_init (&pAppData->decILComp->ThreadAttr);

	if (0 != pthread_create (&pAppData->thrdidReaderTask,
						&pAppData->decILComp->ThreadAttr,
						(ILC_StartFcnPtr) IL_ClientInputBitStreamReadTask, pAppData))  {
		DBG_LOG(DBGLVL_ERROR, ("Create_Task failed !"));
		goto EXIT;
	}

	if(pAppData->nDestType == DEST_TYPE_NULL)	{
		DBG_LOG(DBGLVL_TRACE,(" Creating sink thread"));
		pthread_attr_init (&pAppData->decILComp->ThreadAttr);

		if (0 != pthread_create (&pAppData->decILComp->outDataStrmThrdId,
							&pAppData->decILComp->ThreadAttr,
							(ILC_StartFcnPtr) IL_ClientSinkTask, pAppData->decILComp))	{
			DBG_LOG(DBGLVL_ERROR, ("Create_Task failed !"));
			goto EXIT;
		}
	} else {
		compStartStream(pAppData->decILComp, (ILC_StartFcnPtr) IL_ClientConnInConnOutTask);
		compStartStream(pAppData->scILComp, (ILC_StartFcnPtr) IL_ClientConnInConnOutTask);
	}
	DBG_LOG(DBGLVL_TRACE, ("Streaming now..."));
EXIT:
	DBG_LOG(DBGLVL_TRACE, ("Leave"));

	return 0;
}

vidchainStopInternal(StrmCompIf *pComp)
{
	OMX_U32 i, j;
	void *ret_value;
	int res = 0;

	IL_Client *pAppData = (IL_Client *)pComp->pCtx;
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	IL_CLIENT_PIPE_MSG pipeMsg;

	pipeMsg.cmd = IL_CLIENT_PIPE_CMD_EXIT;

	if(pAppData->nDestType == DEST_TYPE_NULL)	{
		//compStopStream(pAppData->decILComp);
		OMX_BUFFERHEADERTYPE BufferHdrOut = {0};
		OMX_BUFFERHEADERTYPE *pBufferHdrOut = &BufferHdrOut;
		BufferHdrOut.nFlags = OMX_BUFFERFLAG_EOS;
		write(pAppData->decILComp->outPortParams->opBufPipe[1], &pBufferHdrOut, sizeof (pBufferHdrOut));
		DBG_LOG(DBGLVL_SETUP, ("[Waiting for completion of dec-stream-out thread..."));
		pthread_join (pAppData->decILComp->outDataStrmThrdId, (void **) &ret_value);
		DBG_LOG(DBGLVL_SETUP, ("Wait for dec-stream-out complete.]"));
	} else 	{
		compStopStream(pAppData->decILComp);
		compStopStream(pAppData->scILComp);
	} 

	compSetSateIdle(pAppData->decILComp , 0);

	if(pAppData->nDestType == DEST_TYPE_NULL) {
		// Do nothing
	} else  {
		compSetSateIdle(pAppData->scILComp, 0);
	}

  /* change the state to loded */
	compDeinitResource(pAppData->decILComp, 0);

	if(pAppData->nDestType == DEST_TYPE_NULL) {
		// Do nothing
	} else {
		compDeinitResource(pAppData->scILComp, 0);
	}

WaitReaderTthread:
	if(pAppData->thrdidReaderTask) {
		DBG_LOG(DBGLVL_SETUP, ("Waiting for completion of dec-stream-in thread..."));
		pthread_join (pAppData->thrdidReaderTask, (void **) &ret_value);
		DBG_LOG(DBGLVL_SETUP,("Wait for dec-stream-in complete."));
	}

	DBG_LOG(DBGLVL_SETUP, ("IL Client deinitialized"));

	if(pAppData->strmCallback) {
		pAppData->strmCallback(pAppData->pAppCtx, STRM_EVENT_EOS);
	}
	DBG_LOG(DBGLVL_SETUP, (" vidchainStart:Exit"));

EXIT:
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return (res);
}

int IsDataInPipeline(IL_Client *pAppData)
{
	return (pAppData->rendered + pAppData->dropped);
}

void vidchainStop(StrmCompIf *pComp)
{
	void *ret_value;
	int nTimeout = 3000000;
	IL_Client *pAppData = (IL_Client *)pComp->pCtx;

	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	pAppData->nUiCmd = STRM_CMD_STOP;
	
	IL_ClientAbortFillBitStreamData(pAppData);
	vidchainStopInternal(pComp);

	DBG_LOG(DBGLVL_SETUP, ("Issuing STRM_CMD_STOP dec-output thread..."));
	pAppData->decILComp->nUiCmd = STRM_CMD_STOP;

	DBG_LOG(DBGLVL_SETUP, ("[Wait for BitStreamReadTask to stop(Timeout=%dms) ...",nTimeout/1000));
	while(pAppData->fStreaming == OMX_TRUE && nTimeout > 0) {
		usleep(1000);
		nTimeout -= 1000;
	}
	
	DBG_LOG(DBGLVL_SETUP, ("Wait for BitStreamReadTask complete. Streaming=%d (Timeout remainig=%dms]", pAppData->fStreaming, nTimeout/1000));

	if(pAppData->fStreaming == OMX_TRUE) {
		DBG_LOG(DBGLVL_SETUP, ("Cancelling IL_ClientInputBitStreamReadTask"));
		pthread_cancel(pAppData->thrdidReaderTask);
	}
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
}

// Statistics Dump
// TODO: Move to a different file
#include <sys/stat.h>
#include <sys/ioctl.h>


void DumpOmxInPortStats( IL_CLIENT_INPORT_PARAMS *pPort)
{
	char szTmp[128] = {0};
	DBG_PRINT("     ==== In Port Statistics =======\n");
	DBG_PRINT("     nBufferCountActual=%d\n",pPort->nBufferCountActual);
	DBG_PRINT("     nBufferSize=%d\n",pPort->nBufferSize);
	DBG_PRINT("     frames to M3=%d\n",pPort->nFramesToM3);
	DBG_PRINT("     frames from M3=%d\n",pPort->nFramesFromM3);
}

void DumpOmxOutPortStats( IL_CLIENT_OUTPORT_PARAMS *pPort)
{
	char szTmp[128] = {0};
	DBG_PRINT("     === Out Port Statistics =======\n");
	DBG_PRINT("     nBufferCountActual=%d\n",pPort->nBufferCountActual);
	DBG_PRINT("     nBufferSize=%d\n",pPort->nBufferSize);
	DBG_PRINT("     frames to M3=%d\n",pPort->nFramesToM3);
	DBG_PRINT("     frames from M3=%d\n",pPort->nFramesFromM3);
}

void DumpOmxCompStats( IL_CLIENT_COMP_PRIVATE *pILComp)
{
	char szTmp[128] = {0};
	DBG_PRINT("\n====== %s Statistics =======\n", pILComp->comp_name);
	if(pILComp->inPortParams)
		DumpOmxInPortStats(pILComp->inPortParams);
	if(pILComp->outPortParams)
		DumpOmxOutPortStats(pILComp->outPortParams);

}


void DumpAppStats( OMX_PTR ptrAppData)
{
	IL_Client *pApp = (IL_Client *)ptrAppData;
	DBG_PRINT("\n\n======  Application Statistics =======\n");
	DBG_PRINT("Frames Rendered=%d Dropped=%d\n", pApp->rendered, pApp->dropped);

	//DumpParserStat(&pApp->H264Parser);
	//Demux_DumpStat(&pApp->Demux);
	DumpOmxCompStats(pApp->decILComp);
	DumpOmxCompStats(pApp->scILComp);
	DBG_PRINT("\n======================================\n");
}

extern char             *gIniFile;

#define DECODER_SECTION "decode"
#define DEBUG_SECTION   "debug"

static InitDefaults(IL_Client *pAppData)
{
	DBG_LOG(DBGLVL_SETUP, ("Initializing defaults from %s", gIniFile));
	

	pAppData->IL_CLIENT_DECODER_INPUT_BUFFER_COUNT = ini_getl(DECODER_SECTION, "DECODER_INPUT_BUFFER_COUNT", 0, gIniFile);
	pAppData->IL_CLIENT_DECODER_OUTPUT_BUFFER_COUNT = ini_getl(DECODER_SECTION, "DECODER_OUTPUT_BUFFER_COUNT", 0, gIniFile);
	pAppData->IL_CLIENT_SCALAR_INPUT_BUFFER_COUNT = pAppData->IL_CLIENT_DECODER_OUTPUT_BUFFER_COUNT;

	pAppData->IL_CLIENT_SCALAR_OUTPUT_BUFFER_COUNT = ini_getl(DECODER_SECTION, "SCALAR_OUTPUT_BUFFER_COUNT", 0, gIniFile);

	pAppData->IL_CLIENT_DECODER_MAX_FRAME_RATE = ini_getl(DECODER_SECTION, "DECODER_MAX_FRAME_RATE", 0, gIniFile);

#ifdef EN_ENCODER
	pAppData->IL_CLIENT_ENC_INPUT_BUFFER_COUNT = ini_getl(DECODER_SECTION, "ENC_INPUT_BUFFER_COUNT", 10, gIniFile); 
#endif

	pAppData->nJitterLatency = ini_getl(DECODER_SECTION, "JITTER_LATENCY", 0, gIniFile);
	pAppData->nSyncMaxWaitRunning = ini_getl(DECODER_SECTION, "VSYNC_MAX_WAIT_RUNNING", 30000, gIniFile);
	pAppData->nSyncMaxWaitStartup = ini_getl(DECODER_SECTION, "VSYNC_MAX_WAIT_STARTUP", 500000, gIniFile);
	//gDbgLevel = ini_getl(DEBUG_SECTION, "A8_DEBUG_LEVEL", 2, gIniFile);
	pAppData->nDropFraction	 = ini_getl(DECODER_SECTION, "VSYNC_DROP_FRACTION", 2, gIniFile);
	pAppData->IL_COMPONENT_DEBUG_LEVEL = ini_getl(DEBUG_SECTION, "M3_DEBUG_LEVEL", 0, gIniFile);
	pAppData->useDeiScalerAlways = ini_getl(DEBUG_SECTION, "USE_DEI_SCALER_ALWAYS", 0, gIniFile);
	pAppData->nDestType = DEST_TYPE_DISP;
	pAppData->prev_clk = 0;
	pAppData->prev_frame_count = 0;
	pAppData->nSessionId = 0;

}

void vidchainSetInputConn(StrmCompIf *pComp, int nConnNum, ConnCtxT *pConn)
{
	IL_Client *pAppData = (IL_Client *)pComp->pCtx;
	pAppData->pConnSrc = pConn;
}

void vidchainSetClkSrc(StrmCompIf *pComp, void *pClk)
{
	IL_Client *pAppData = (IL_Client *)pComp->pCtx;
	pAppData->pClk = pClk;
}

int vidchainDelete(StrmCompIf *pComp)
{
	IL_Client *pAppData = (IL_Client *)pComp->pCtx;
	if(pAppData) {
		IL_ClientDeInit (pAppData);
		free (pAppData);
	}
	return 0;
}

StrmCompIf *
vidchainCreate()
{
	StrmCompIf *pComp = (StrmCompIf *)malloc(sizeof(StrmCompIf));
	memset (pComp, 0x0, sizeof (StrmCompIf));
	IL_Client *pAppData = (IL_Client *) malloc (sizeof (IL_Client));
	memset (pAppData, 0x0, sizeof (IL_Client));
	InitDefaults(pAppData);

	pComp->pCtx = pAppData;

	pComp->Open= vidchainOpen;
	pComp->SetOption = vidcahinSetOption;
	pComp->SetInputConn= vidchainSetInputConn;
	pComp->SetOutputConn= NULL;
	pComp->SetClkSrc = vidchainSetClkSrc;
	pComp->GetClkSrc = NULL;
	pComp->Start = vidchainStart;
	pComp->Stop = vidchainStop;
	pComp->Close = NULL;
	pComp->Delete = vidchainDelete;
	return pComp;
}
