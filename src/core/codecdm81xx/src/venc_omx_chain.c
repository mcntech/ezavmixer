#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <xdc/std.h>

#define CODEC_H264ENC	// Ram: This definition is required when compiling this component as library
/*-------------------------program files -------------------------------------*/
#include "ti/omx/interfaces/openMaxv11/OMX_Core.h"
#include "ti/omx/interfaces/openMaxv11/OMX_Component.h"
#include "OMX_TI_Common.h"
#include "OMX_Video.h"
#include "OMX_TI_Video.h"
#include "venc_omx_chain.h"
#include <omx_venc.h>
#include <omx_vfpc.h>
#include <omx_vfdc.h>
#include <omx_vfcc.h>
#include <omx_ctrl.h>
#include <OMX_TI_Index.h>
#include "dec_platform_utils.h"
#include "ilclient_common.h"
#include "video_mixer.h"

/******************************************************************************\
 *      Includes
\******************************************************************************/
#include "semp.h"
#include "dbglog.h"
#include "minini.h"


/******************************************************************************/

#ifdef __cplusplus    /* required for headers that might */
extern "C"
{                               /* be compiled by a C++ compiler */
#endif

//#define IL_CLIENT_CAPTURE_OUTPUT_BUFFER_COUNT (12)
//#define IL_CLIENT_DEI_INPUT_BUFFER_COUNT      IL_CLIENT_CAPTURE_OUTPUT_BUFFER_COUNT
//#define IL_CLIENT_DEI_OUTPUT_BUFFER_COUNT     (8)
//#define IL_CLIENT_ENC_INPUT_BUFFER_COUNT      (8)
//#define IL_CLIENT_ENC_OUTPUT_BUFFER_COUNT     (8)
//#define IL_CLIENT_DISPLAY_INPUT_BUFFER_COUNT  IL_CLIENT_DEI_OUTPUT_BUFFER_COUNT

#define IL_CLIENT_MAX_NUM_IN_BUFS  16
#define IL_CLIENT_MAX_NUM_OUT_BUFS 16

#define OMX_TEST_INIT_STRUCT_PTR(_s_, _name_)                                  \
          memset((_s_), 0x0, sizeof(_name_));                                  \
          (_s_)->nSize = sizeof(_name_);                                       \
          (_s_)->nVersion.s.nVersionMajor = 0x1;                               \
          (_s_)->nVersion.s.nVersionMinor = 0x1;                               \
          (_s_)->nVersion.s.nRevision  = 0x0;                                  \
          (_s_)->nVersion.s.nStep   = 0x0;

/** Number of input buffers in the H264 Decoder IL Client */
#define NUM_OF_IN_BUFFERS 4

/** Number of output buffers in the H264 Decoder IL Client */
#define NUM_OF_OUT_BUFFERS 8
#define MAX_MODE_NAME_SIZE      16


/* ========================================================================== */
/** IL_ClientEnc is the structure definition for the  decoder->sc->display IL Client
*
* @param pHandle               OMX Handle  
* @param pComponent            Component Data structure
* @param pCb                   Callback function pointer
* @param eState                Current OMX state
* @param pInPortDef            Structure holding input port definition
* @param pOutPortDef           Structure holding output port definition
* @param eCompressionFormat    Format of the input data
* @param pH264                 <TBD>
* @param pInBuff               Input Buffer pointer
* @param pOutBuff              Output Buffer pointer
* @param IpBuf_Pipe            Input Buffer Pipe
* @param OpBuf_Pipe            Output Buffer Pipe
* @param fIn                   File pointer of input file
* @param fInFrmSz              File pointer of Frame Size file (unused)
* @param fOut                  Output file pointer
* @param ColorFormat           Input color format
* @param nInputWidth                Width of the input vector
* @param nInputHeight               Height of the input vector
* @param nEncodedFrm           Total number of encoded frames
* @param displayId             ID of the display device chosen
*/
/* ========================================================================== */
typedef struct IL_ClientEnc
{
	OMX_HANDLETYPE pctrlHandle;
	OMX_CALLBACKTYPE pCb;
	OMX_STATETYPE eState;
	OMX_U8 eCompressionFormat;
	FILE *fIn;
	FILE *fInFrmSz;
	FILE *fOut;
	OMX_COLOR_FORMATTYPE ColorFormat;
	OMX_U32 nInputWidth;
	OMX_U32 nInputHeight;
	OMX_U32 nEncWidth;
	OMX_U32 nEncHeight;

	OMX_U32 nDecStride;
	OMX_U32 nFrameRate;
	OMX_U32 nBitRate;
	OMX_U32 nEncodedFrms;
	OMX_U8 mode[MAX_MODE_NAME_SIZE];
	OMX_U32 displayId;
	int     nDispWidth;
	int     nDispHeight;
	void *fieldBuf;
	IL_CLIENT_COMP_PRIVATE *encILComp;
	IL_CLIENT_COMP_PRIVATE *deiILComp;
	IL_CLIENT_COMP_PRIVATE *extsrcILComp;
	IL_CLIENT_COMP_PRIVATE *extsinkILComp;
	ConnCtxT               *pEncOutConn;
	int                     nEncStrmUiCmd;
	int nDeiInputBuffers;
	int nDeiOutputBuffers;
	int nEncInputBuffers;
	int nEncOutputBuffers;
	int  fStreaming;
	int  fEncEoS;
	VIDEO_MIXER_T        *pVidMix;
} IL_ClientEnc;


#ifdef __cplusplus    /* matches __cplusplus construct above */
 }
#endif


//==================================================== UTIL ======================================


static void vencchainSetOption (
	struct _StrmCompIf *pComp, 
	int                nCmd, 
	char               *pOptionData)
{
	IL_ClientEnc *pAppData = pComp->pCtx;
	if(nCmd == VENC_CMD_SET_PARAMS ) {
		VENC_CHAIN_ARGS *pArgs = (VENC_CHAIN_ARGS *)pOptionData;
		/* update the user provided parameters */
		pAppData->nFrameRate = pArgs->frame_rate;
		pAppData->nBitRate = pArgs->bit_rate;
		pAppData->nEncodedFrms = pArgs->num_frames;

		pAppData->nDispWidth = pArgs->disp_width;
		pAppData->nDispHeight = pArgs->disp_height;

		pAppData->nEncWidth = pArgs->enc_width;
		pAppData->nEncHeight = pArgs->enc_height;
  
		pAppData->nInputHeight = pArgs->input_height;
		pAppData->nInputWidth =  pArgs->input_width;
		pAppData->displayId = pArgs->display_id;
		pAppData->nDeiInputBuffers = pArgs->input_buffers;
		pAppData->nDeiOutputBuffers = pArgs->output_buffers;
	}
}

static void vencchainInitInternal (IL_ClientEnc *pAppData)
{
	int i;
	IL_CLIENT_INPORT_PARAMS *inPortParamsPtr;
	IL_CLIENT_OUTPORT_PARAMS *outPortParamsPtr;

	TRACE_PROGRESS
	pAppData->deiILComp = CreateILCompWrapper(1, 2, OMX_VFPC_OUTPUT_PORT_START_INDEX, pAppData->nDeiInputBuffers, pAppData->nInputWidth * pAppData->nInputHeight * 2,
				pAppData->nDeiOutputBuffers, pAppData->nDispWidth * pAppData->nDispHeight * 2);
	compSetInportAllocationType(pAppData->deiILComp, 0, 1);
	// Overwrite dei second ouput corresponding to encoder
	outPortParamsPtr = pAppData->deiILComp->outPortParams + 1;
	outPortParamsPtr->nBufferSize =	(pAppData->nEncHeight * pAppData->nEncWidth * 3) >> 1;
	outPortParamsPtr->nBufferCountActual = pAppData->nEncInputBuffers;

	pAppData->encILComp = CreateILCompWrapper(1, 1, OMX_VIDENC_OUTPUT_PORT, pAppData->nEncInputBuffers, (pAppData->nEncWidth * pAppData->nEncHeight * 3) >> 1,
				pAppData->nEncOutputBuffers, (pAppData->nEncWidth * pAppData->nEncHeight));
	compSetInportAllocationType(pAppData->encILComp, 0, 1);

}

/* ========================================================================== */
/**
* IL_ClientInit() : This function is to deinitialize the application
*                   data structure.
*
* @param pAppData          : appliaction / client data Handle 
*  @return      
*
*
*/
/* ========================================================================== */

void IL_EncClientDeInit (IL_ClientEnc * pAppData)
{
  int i;
  IL_CLIENT_INPORT_PARAMS *inPortParamsPtr;
  IL_CLIENT_OUTPORT_PARAMS *outPortParamsPtr;
  
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	if(pAppData->deiILComp)
		DeleteILCompWrapper(pAppData->deiILComp);
	if(pAppData->encILComp)
		DeleteILCompWrapper(pAppData->encILComp);
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
}



/* ========================================================================== */
/**
* IL_ClientEncUseInitialOutputResources() :  This function gives initially all
*                                         output buffers to a component.
*                                         after consuming component would keep
*                                         in local pipe for connect thread use. 
*
* @param pAppdata   : application data structure
*
*  @return      
*  String conversion of the OMX_ERRORTYPE
*
*/
/* ========================================================================== */

static OMX_ERRORTYPE IL_ClientEncUseInitialOutputResources (
	IL_CLIENT_COMP_PRIVATE *thisComp)
{
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	OMX_ERRORTYPE err = OMX_ErrorNone;
	unsigned int i = 0;
	TRACE_PROGRESS
	DBG_LOG(DBGLVL_TRACE, ("num buffers=%d",thisComp->outPortParams->nBufferCountActual));
	for (i = 0; i < thisComp->outPortParams->nBufferCountActual; i++) {
		/* Pass the output buffer to the component */
		DBG_LOG(DBGLVL_TRACE, ("buffer=%d",i));
		err = OMX_FillThisBuffer (thisComp->handle,	thisComp->outPortParams->pOutBuff[i]);
	}
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
  return err;
}



/* ========================================================================== */
/**
* IL_ClientSetDeiParams() : Function to fill the port definition 
* structures and call the Set_Parameter function on to the DEI
* Component
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

static OMX_ERRORTYPE InitDeiScaler (IL_ClientEnc *pAppData)
{
	int fInInterlaced = 0;
	int fInFormat = OMX_COLOR_FormatYCbYCr;
	int nStride = pAppData->nInputWidth * 2;
	// TODO add more interlaced formats
//	if (strcmp ((char *) pAppData->mode, "1080i") == 0) {
//		fInInterlaced = 1;
//		fInFormat = OMX_COLOR_FormatYCbYCr;
//		nStride = pAppData->nInputWidth * 2;
//	}

	return scalerInit(	
				pAppData->deiILComp,
				fInInterlaced,
				1, //useDeiScalerAlways

				pAppData->nInputWidth,
				pAppData->nInputHeight,
				nStride,
				pAppData->nInputWidth *	pAppData->nInputHeight * 2,

				pAppData->nDeiInputBuffers,
				fInFormat,

				pAppData->nDispWidth,
				pAppData->nDispHeight,
				pAppData->nDispWidth * 2,
				pAppData->nDispWidth *	pAppData->nDispHeight * 2,
				pAppData->nDeiOutputBuffers,
				OMX_COLOR_FormatYCbYCr,

				pAppData->nEncWidth,
				pAppData->nEncHeight,
				pAppData->nEncWidth,
				(pAppData->nEncWidth * pAppData->nEncHeight * 3) >> 1,
				pAppData->nEncOutputBuffers,
				OMX_COLOR_FormatYUV420SemiPlanar,
				&pAppData->pCb
				);
}

/*
** Sends filled encoder buffers to the client ( pEncOutConn )
** 1. Reads buffer header from the pipe written by even handler
** 2. Send the data to the client
** 3. Supplies the buffer hdr back to IL component
*/
static void *IL_EncOutputBitStreamWriteTask (void *threadsArg)
{
	//unsigned int dataRead = 0;
	OMX_ERRORTYPE err = OMX_ErrorNone;
	IL_CLIENT_COMP_PRIVATE *encILComp = NULL;
	OMX_BUFFERHEADERTYPE *pBufferOut = NULL;
	IL_ClientEnc *pAppData = (IL_ClientEnc *)threadsArg;
	encILComp = pAppData->encILComp;
	CLOCK_T pts = 0;
	void *pClck = clkCreate(0);
	CLOCK_T clkNextFrame = 0;
	CLOCK_T clkStart = 0;
	CLOCK_T clkFrameDurationUs = (1000000.0 / pAppData->nFrameRate);
	/* use the initial i/p buffers and make empty this buffer calls */
	TRACE_PROGRESS
	err = IL_ClientEncUseInitialOutputResources (encILComp);
	TRACE_PROGRESS
	
	ClockStart(pClck, 0);

	while (pAppData->nEncStrmUiCmd != STRM_CMD_STOP)	{
		/* Read filled buffer pointer from the pipe */
		read (encILComp->outPortParams->opBufPipe[0], &pBufferOut, sizeof (pBufferOut));

		if(pBufferOut->nFlags & OMX_BUFFERFLAG_EOS) {
			DBG_LOG(DBGLVL_SETUP, ("Exiting due to EoS"));
			goto Exit;
		}

		encILComp->frameCounter++;
		if(pAppData->pEncOutConn) {
			unsigned long ulFlags = 0;
			if(pAppData->fEncEoS) {
				ulFlags = OMX_BUFFERFLAG_EOS;
			}

			while(pAppData->pEncOutConn->IsFull(pAppData->pEncOutConn) && pAppData->nEncStrmUiCmd != STRM_CMD_STOP){
				DBG_LOG(DBGLVL_WAITLOOP,("Waiting for free buffer"))
	#ifdef WIN32
				Sleep(1);
	#else
				usleep(1000);
	#endif
				ShowStat(encILComp);
			}
			if(pAppData->nEncStrmUiCmd == STRM_CMD_STOP) {
				goto Exit;
			}
			pts = pBufferOut->nTimeStamp;
			if(pts == 0) {
				pts = ClockGetTime(NULL);
			}
			pAppData->pEncOutConn->Write(pAppData->pEncOutConn, pBufferOut->pBuffer, pBufferOut->nFilledLen,  ulFlags, pts);

		}
#if 0
		/* write data to output file */
		fwrite (pBufferOut->pBuffer,  sizeof (char),  pBufferOut->nFilledLen, ((IL_ClientEnc *) threadsArg)->fOut);
		if((encILComp->frameCounter == encILComp->numFrames) || (encILComp->nUiCmd == STRM_CMD_STOP/*gILClientExit == OMX_TRUE*/)) 	{
			encILComp->frameCounter = 0;
			semp_post(encILComp->eos);
			pthread_exit(encILComp);
		}
#else
		//DBG_LOG(DBGLVL_TRACE, ("Enc Frame %d len=%d",encILComp->frameCounter, pBufferOut->nFilledLen));
		//DumpHex(pBufferOut->pBuffer, 14);
		clkNextFrame = encILComp->frameCounter * clkFrameDurationUs;
		WaitForClock(pClck, clkNextFrame, 50000);

		ShowStat(encILComp);
#endif

		/* Pass the input buffer to the component */
		err = OMX_FillThisBuffer (encILComp->handle, pBufferOut);

		if (OMX_ErrorNone != err) {
			/* put back the frame in pipe and wait for state change */
			DBG_MSG (" !!! Error %d!!!\n", err);
			write (encILComp->outPortParams->opBufPipe[1],  &pBufferOut, sizeof (pBufferOut));
			DBG_MSG (" waiting for action from IL Client \n");

			/* since in this example we are changing states in other thread it will
			return error for giving ETB/FTB calls in non-execute state. Since
			example is shutting down, we exit the thread */

			pthread_exit (encILComp);

		}
	}
Exit:
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return NULL;
}

int vencchainOpen(StrmCompIf *pComp, const char *szResource)
{
	IL_ClientEnc *pAppData = (IL_ClientEnc *)pComp->pCtx;
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_S32 ret_value;
	  
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	vencchainInitInternal (pAppData);
	pAppData->fStreaming = 1;

#if 0
	printf (" opening file \n");
	/* Open the file of data to be rendered.  */
	pAppData->fOut = fopen (args->output_file, "wb");

	if (pAppData->fOut == NULL) 	{
		printf ("Error: failed to open the file %s for writing \n",
				args->output_file);
		goto EXIT;
	}
#endif

	pAppData->pCb.EventHandler = IL_ClientCbEventHandler;
	pAppData->pCb.EmptyBufferDone = IL_ClientCbEmptyBufferDone;
	pAppData->pCb.FillBufferDone = IL_ClientCbFillBufferDone;

	IL_ClientConnectComponents (pAppData->deiILComp, OMX_VFPC_OUTPUT_PORT_START_INDEX + 1,	pAppData->encILComp, OMX_VIDENC_INPUT_PORT);

EXIT:
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return (0);
}

int AllocateResource(StrmCompIf *pComp)
{
	IL_ClientEnc *pAppData = (IL_ClientEnc *)pComp->pCtx;
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	
	TRACE_PROGRESS

	InitDeiScaler(pAppData);
	TRACE_PROGRESS
	encoderInit(pAppData, pAppData->encILComp, &pAppData->pCb, pAppData->nEncWidth, pAppData->nEncHeight, pAppData->nEncInputBuffers, pAppData->nEncOutputBuffers, pAppData->nEncodedFrms, pAppData->nFrameRate, pAppData->nBitRate);
	TRACE_PROGRESS
	compInitResource(pAppData->deiILComp, 0);
	TRACE_PROGRESS
	compSetInportAllocationType(pAppData->encILComp, 0, 1);
	compInitResource(pAppData->encILComp, 0);


	return 0;	  
}

int vencchainStart(StrmCompIf *pComp)
{
	IL_ClientEnc *pAppData = (IL_ClientEnc *)pComp->pCtx;
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_S32 ret_value;

	DBG_LOG(DBGLVL_TRACE, ("Enter"));
  	compSetStateExec(pAppData->encILComp, 0);
	compSetStateExec(pAppData->deiILComp, 0);

	/* Create thread for writing bitstream and passing the buffers to encoder  component */
	pthread_attr_init (&pAppData->encILComp->ThreadAttr);
	pAppData->nEncStrmUiCmd = STRM_CMD_RUN;
	if (0 !=  pthread_create (&pAppData->encILComp->outDataStrmThrdId, &pAppData->encILComp->ThreadAttr,
                      (ILC_StartFcnPtr) IL_EncOutputBitStreamWriteTask, pAppData))  {
		printf ("Create_Task failed !");
		goto EXIT;
	}

	compStartStream(pAppData->encILComp, (ILC_StartFcnPtr)IL_ClientConnInConnOutTask);
	compStartStream(pAppData->deiILComp, (ILC_StartFcnPtr)IL_ClientConnInConnOutTask);


EXIT:
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return (0);
}


int vencchainStopInternal(StrmCompIf *pComp)
{
	IL_ClientEnc *pAppData = (IL_ClientEnc *)pComp->pCtx;
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_S32 ret_value;
	IL_CLIENT_PIPE_MSG pipeMsg;
	  
//	semp_pend(pAppData->encILComp->eos);

	/* tear down sequence */

	/* change the state to idle */
	/* before changing state to idle, buffer communication to component should be 
		stoped , writing an exit message to threads */

	pipeMsg.cmd = IL_CLIENT_PIPE_CMD_EXIT;
	compStopStream(pAppData->deiILComp);
	compStopStream(pAppData->encILComp);
	
	{
		//TODO: use dynamic memory and save pointer in component
		OMX_BUFFERHEADERTYPE BufferHdrOut = {0};
		OMX_BUFFERHEADERTYPE *pBufferHdrOut = &BufferHdrOut;
		BufferHdrOut.nFlags = OMX_BUFFERFLAG_EOS;
		pAppData->nEncStrmUiCmd = STRM_CMD_STOP;
		write (pAppData->encILComp->outPortParams->opBufPipe[1], &pBufferHdrOut, sizeof (pBufferHdrOut));
		DBG_LOG(DBGLVL_TRACE, ("Wait for encILComp->outDataStrmThrdId:Begin"));
		pthread_join(pAppData->encILComp->outDataStrmThrdId, (void **) &ret_value);
		DBG_LOG(DBGLVL_TRACE, ("Wait for encILComp->outDataStrmThrdId:Complete"));
	}
	compSetSateIdle(pAppData->deiILComp, 0);
	compSetSateIdle(pAppData->encILComp, 0);

	compDeinitResource(pAppData->encILComp, 0);
	compDeinitResource(pAppData->deiILComp, 0);
EXIT:
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	pAppData->fStreaming = 0;
	return (0);
}

void vencchainStop(StrmCompIf *pComp)
{
	void *ret_value;
	int nTimeout = 3000000;
	IL_ClientEnc *pAppData = (IL_ClientEnc *)pComp->pCtx;

	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	//pAppData->nUiCmd = STRM_CMD_STOP;
	//semp_post(pAppData->encILComp->eos);
	vencchainStopInternal(pComp);

	DBG_LOG(DBGLVL_SETUP, ("[Wait for IL_ClientInputBitStreamReadTask to stop(Timeout=%dms) ...",nTimeout/1000));
	while(pAppData->fStreaming == OMX_TRUE && nTimeout > 0) {
		usleep(1000);
		nTimeout -= 1000;
	}
	
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
}

int vencchainnDelete(StrmCompIf *pComp)
{
	IL_ClientEnc *pAppData = (IL_ClientEnc *)pComp->pCtx;
	if(pAppData) {
		IL_EncClientDeInit (pAppData);
		free (pAppData);
	}
	return 0;
}

int SetInputConn2(StrmCompIf *pChain, int nInSlot, void *pExtSrcComp, int pExtSrcOutPort)
{
	IL_ClientEnc *pAppData = (IL_ClientEnc *)pChain->pCtx;
	IL_ClientConnectComponents((IL_CLIENT_COMP_PRIVATE *)pExtSrcComp, pExtSrcOutPort, pAppData->deiILComp, OMX_VFPC_INPUT_PORT_START_INDEX);
	return 0;
}

int SetOutputConn2(StrmCompIf *pChain, int nOutSlot, void *pExtSinkComp, int nExtSinkInputPort)
{
	int nOutPort = nOutSlot + OMX_VFPC_OUTPUT_PORT_START_INDEX;
	IL_ClientEnc *pAppData = (IL_ClientEnc *)pChain->pCtx;
	IL_ClientConnectComponents( pAppData->deiILComp, nOutPort, (IL_CLIENT_COMP_PRIVATE *)pExtSinkComp, nExtSinkInputPort);
	return 0;
}

int SetEncoderOutputCon(struct _StrmCompIf *pChain, int nConnNum, ConnCtxT *pConn)
{
	IL_ClientEnc *pAppData = (IL_ClientEnc *)pChain->pCtx;
	pAppData->pEncOutConn = pConn;
	return 0;
}

extern char   *gIniFile;
#define DEBUG_SECTION   "debug"
#define ENCODER_SECTION "encode"

static InitDefaults(IL_ClientEnc *pAppData)
{
	pAppData->nEncInputBuffers = ini_getl(ENCODER_SECTION, "ENCODER_INPUT_BUFFER_COUNT", 6, gIniFile);
	pAppData->nEncOutputBuffers = ini_getl(ENCODER_SECTION, "ENCODER_OUTPUT_BUFFER_COUNT", 6, gIniFile);
	
	// TODO: Make it configurable
	pAppData->nDeiOutputBuffers = 6;
	pAppData->nDeiInputBuffers = 6;
}

StrmCompIf *
vencchainCreate()
{
	StrmCompIf *pComp = (StrmCompIf *)malloc(sizeof(StrmCompIf));
	memset (pComp, 0x0, sizeof (StrmCompIf));
	IL_ClientEnc *pAppData = (IL_ClientEnc *) malloc (sizeof (IL_ClientEnc));
	memset (pAppData, 0x0, sizeof (IL_ClientEnc));
	pComp->pCtx = pAppData;

	InitDefaults(pAppData);

	pComp->Open= vencchainOpen;
	pComp->AllocateResource = AllocateResource;
	pComp->SetOption = vencchainSetOption;
	pComp->SetInputConn= NULL;
	pComp->SetOutputConn= SetEncoderOutputCon;
	pComp->SetInputConn2 = SetInputConn2;
	pComp->SetOutputConn2 = SetOutputConn2;
	pComp->SetClkSrc = NULL;//vidchainSetClkSrc;
	pComp->GetClkSrc = NULL;
	pComp->Start = vencchainStart;
	pComp->Stop = vencchainStop;
	pComp->Close = NULL;
	pComp->Delete = vencchainnDelete;
	return pComp;
}

