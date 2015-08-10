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
#include "vcap_omx_chain.h"
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
	OMX_HANDLETYPE pCapHandle, pctrlHandle, pTvpHandle,
					pEncHandle;
	OMX_COMPONENTTYPE *pComponent;
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
	IL_CLIENT_COMP_PRIVATE *capILComp;
	IL_CLIENT_COMP_PRIVATE *disILComp;

	int audchan_l;
	int audchan_r;  
	int nCaptureBuffers;
	int nDeiInputBuffers;
	int nDeiOutputBuffers;
	int nEncInputBuffers;
	int nEncOutputBuffers;
	int nDispInputBuffers;
	int  fStreaming;
	VIDEO_MIXER_T        *pVidMix;
} IL_ClientEnc;


#ifdef __cplusplus    /* matches __cplusplus construct above */
 }
#endif


//==================================================== UTIL ======================================


static void vcapchainSetOption (
	struct _StrmCompIf *pComp, 
	int                nCmd, 
	char               *pOptionData)
{
	IL_ClientEnc *pAppDataPtr = pComp->pCtx;
	if(nCmd == VCAP_CMD_SET_PARAMS ) {
		IL_VCAP_ARGS *pArgs = (IL_VCAP_ARGS *)pOptionData;
		/* update the user provided parameters */
		pAppDataPtr->nFrameRate = pArgs->frame_rate;
		pAppDataPtr->nBitRate = pArgs->bit_rate;
		pAppDataPtr->nEncodedFrms = pArgs->num_frames;
		pAppDataPtr->nDispWidth = pArgs->disp_width;
		pAppDataPtr->nDispHeight = pArgs->disp_height;
		strncpy ((char *) pAppDataPtr->mode, pArgs->mode, MAX_MODE_NAME_SIZE);  
  
		/* based on capture/display mode selected set width and height */
		if ((strcmp (pAppDataPtr->mode, "1080p") == 0) || (strcmp (pAppDataPtr->mode, "1080i") == 0))  {
			pAppDataPtr->nInputHeight = 1080;
			pAppDataPtr->nInputWidth =  1920; 
		} else  if (strcmp (pAppDataPtr->mode, "720p") == 0) {
			pAppDataPtr->nInputHeight = 720;
			pAppDataPtr->nInputWidth =  1280; 
		} else	{
			ERROR ("In correct Mode selected!! \n");
		}
		pAppDataPtr->displayId = pArgs->display_id;
		pAppDataPtr->audchan_l = pArgs->audchan_l;
		pAppDataPtr->audchan_r = pArgs->audchan_r;
	}
}
/* ========================================================================== */
/**
* IL_ClientInit() : This function is to allocate and initialize the application
*                   data structure. It is just to maintain application control.
*
* @param pAppData          : appliaction / client data Handle 
* @param width             : stream width
* @param height            : stream height
* @param frameRate         : encode frame rate
* @param bitrate           : encoder bit rate
* @param numFrames         : encoded number of frames
* @param displayId         : display instance id
*
*  @return      
*
*
*/
/* ========================================================================== */

static void IL_ClientInit (IL_ClientEnc *pAppDataPtr)
{
	int i;
	IL_CLIENT_INPORT_PARAMS *inPortParamsPtr;
	IL_CLIENT_OUTPORT_PARAMS *outPortParamsPtr;


	pAppDataPtr->capILComp = CreateILCompWrapper(0, 1, OMX_VFCC_OUTPUT_PORT_START_INDEX, 0, 0,
				pAppDataPtr->nCaptureBuffers, pAppDataPtr->nInputWidth * pAppDataPtr->nInputHeight * 2);

  // TODO: correct buffer sizes
	pAppDataPtr->deiILComp = CreateILCompWrapper(1, 2, OMX_VFPC_OUTPUT_PORT_START_INDEX, pAppDataPtr->nDeiInputBuffers, pAppDataPtr->nInputWidth * pAppDataPtr->nInputHeight * 2,
				pAppDataPtr->nDeiOutputBuffers, pAppDataPtr->nDispWidth * pAppDataPtr->nDispHeight * 2);
	// Overwrite dei second ouput corresponding to encoder
	outPortParamsPtr = pAppDataPtr->deiILComp->outPortParams + 1;
	outPortParamsPtr->nBufferSize =	(pAppDataPtr->nEncHeight * pAppDataPtr->nEncWidth * 3) >> 1;
	outPortParamsPtr->nBufferCountActual = pAppDataPtr->nEncInputBuffers;

	pAppDataPtr->encILComp = CreateILCompWrapper(1, 1, OMX_VIDENC_OUTPUT_PORT, pAppDataPtr->nEncInputBuffers, (pAppDataPtr->nInputWidth * pAppDataPtr->nInputHeight * 3) >> 2,
				pAppDataPtr->nEncOutputBuffers, (pAppDataPtr->nEncWidth * pAppDataPtr->nEncHeight * 3) >> 2);

	pAppDataPtr->disILComp = CreateILCompWrapper(1, 0, 0, 	pAppDataPtr->nDispInputBuffers, pAppDataPtr->nDispHeight * pAppDataPtr->nDispWidth * 2, 0, 0);

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

static void IL_ClientDeInit (IL_ClientEnc * pAppData)
{
  int i;
  IL_CLIENT_INPORT_PARAMS *inPortParamsPtr;
  IL_CLIENT_OUTPORT_PARAMS *outPortParamsPtr;
  
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	if(pAppData->capILComp)
		DeleteILCompWrapper(pAppData->capILComp);
	if(pAppData->disILComp)
		DeleteILCompWrapper(pAppData->disILComp);
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

static OMX_ERRORTYPE IL_ClientEncUseInitialOutputResources (IL_CLIENT_COMP_PRIVATE 
                                                       *thisComp)
{
  OMX_ERRORTYPE err = OMX_ErrorNone;
  unsigned int i = 0;

  for (i = 0; i < thisComp->outPortParams->nBufferCountActual; i++)
  {
    /* Pass the output buffer to the component */
    err = OMX_FillThisBuffer (thisComp->handle,
                              thisComp->outPortParams->pOutBuff[i]);
  }

  return err;
}


static OMX_ERRORTYPE IL_ClientSetCaptureParams (IL_ClientEnc *pAppData)
{

  OMX_PARAM_VFCC_HWPORT_PROPERTIES sHwPortParam;

  OMX_PARAM_VFCC_HWPORT_ID sHwPortId;

  OMX_CONFIG_VFCC_FRAMESKIP_INFO sCapSkipFrames;

  OMX_PARAM_CTRL_VIDDECODER_INFO sVidDecParam;

  OMX_PARAM_BUFFER_MEMORYTYPE memTypeCfg;

  OMX_PARAM_PORTDEFINITIONTYPE paramPort;

  OMX_ERRORTYPE eError = OMX_ErrorNone;

  OMX_INIT_PARAM (&paramPort);

  /* set input height/width and color format */
  paramPort.nPortIndex = OMX_VFCC_OUTPUT_PORT_START_INDEX;
  OMX_GetParameter (pAppData->pCapHandle, OMX_IndexParamPortDefinition,
                    &paramPort);
  paramPort.nPortIndex = OMX_VFCC_OUTPUT_PORT_START_INDEX;
  paramPort.format.video.nFrameWidth = pAppData->nInputWidth;
  paramPort.format.video.nFrameHeight = pAppData->nInputHeight;
  paramPort.format.video.nStride = pAppData->nInputWidth;
  paramPort.nBufferCountActual = pAppData->nCaptureBuffers;
  paramPort.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
  /* Capture output in 420 format */
  paramPort.format.video.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
  paramPort.nBufferSize =
    (paramPort.format.video.nStride * pAppData->nInputHeight * 3) >> 1;

  if (strcmp ((char *) pAppData->mode, "1080i") == 0) {
  paramPort.format.video.nStride = pAppData->nInputWidth << 1;
  paramPort.format.video.eColorFormat = OMX_COLOR_FormatYCbYCr;
  paramPort.nBufferSize =
    (paramPort.format.video.nStride * pAppData->nInputHeight) >> 1;
  }

  DBG_MSG ("Buffer Size computed: %d\n", (int) paramPort.nBufferSize);
  DBG_MSG ("set input port params (width = %d, height = %d)", (int) pAppData->nInputWidth, (int) pAppData->nInputHeight);
  OMX_SetParameter (pAppData->pCapHandle, OMX_IndexParamPortDefinition,
                    &paramPort);

  /* Setting Memory type at output port to Raw Memory */
  OMX_INIT_PARAM (&memTypeCfg);
  memTypeCfg.nPortIndex = OMX_VFCC_OUTPUT_PORT_START_INDEX;
  memTypeCfg.eBufMemoryType = OMX_BUFFER_MEMORY_DEFAULT;
  eError =
    OMX_SetParameter (pAppData->pCapHandle, OMX_TI_IndexParamBuffMemType,
                      &memTypeCfg);

  if (eError != OMX_ErrorNone)
    ERROR ("failed to set memory Type at output port\n");

  OMX_INIT_PARAM (&sHwPortId);
  /* capture on EIO card is component input at VIP1 port */
  sHwPortId.eHwPortId = OMX_VIDEO_CaptureHWPortVIP1_PORTA;
  eError = OMX_SetParameter (pAppData->pCapHandle,
                             (OMX_INDEXTYPE) OMX_TI_IndexParamVFCCHwPortID,
                             (OMX_PTR) & sHwPortId);

  OMX_INIT_PARAM (&sHwPortParam);

  sHwPortParam.eCaptMode = OMX_VIDEO_CaptureModeSC_NON_MUX;
  sHwPortParam.eVifMode = OMX_VIDEO_CaptureVifMode_16BIT;
  sHwPortParam.eInColorFormat = OMX_COLOR_FormatYCbYCr;
  sHwPortParam.eScanType = OMX_VIDEO_CaptureScanTypeProgressive;
  sHwPortParam.nMaxHeight = pAppData->nInputHeight;
#ifndef OMX_05_02_00_30
  sHwPortParam.bFieldMerged   = OMX_FALSE;
#endif
  if (strcmp ((char *) pAppData->mode, "1080i") == 0) {
    sHwPortParam.eScanType = OMX_VIDEO_CaptureScanTypeInterlaced;
    sHwPortParam.nMaxHeight = pAppData->nInputHeight >> 1;
#ifndef OMX_05_02_00_30
    sHwPortParam.bFieldMerged   = OMX_FALSE;
#endif
    }
  
  sHwPortParam.nMaxWidth = pAppData->nInputWidth;
  sHwPortParam.nMaxChnlsPerHwPort = 1;

  eError = OMX_SetParameter (pAppData->pCapHandle,
                             (OMX_INDEXTYPE)
                             OMX_TI_IndexParamVFCCHwPortProperties,
                             (OMX_PTR) & sHwPortParam);

  if (pAppData->nFrameRate == 30)
  {
    OMX_INIT_PARAM (&sCapSkipFrames);
    DBG_MSG (" applying skip mask \n");

    sCapSkipFrames.frameSkipMask = 0x2AAAAAAA;
    eError = OMX_SetConfig (pAppData->pCapHandle,
                            (OMX_INDEXTYPE) OMX_TI_IndexConfigVFCCFrameSkip,
                            (OMX_PTR) & sCapSkipFrames);
  }

#if 0
  /* Set parameters for TVP controller */

  OMX_INIT_PARAM (&sHwPortId);
  /* capture on EIO card is component input at VIP1 port */
  sHwPortId.eHwPortId = OMX_VIDEO_CaptureHWPortVIP1_PORTA;
  eError = OMX_SetParameter (pAppData->pTvpHandle,
                             (OMX_INDEXTYPE) OMX_TI_IndexParamVFCCHwPortID,
                             (OMX_PTR) & sHwPortId);
  OMX_INIT_PARAM (&sHwPortParam);
  sHwPortParam.eCaptMode = OMX_VIDEO_CaptureModeSC_NON_MUX;
  sHwPortParam.eVifMode = OMX_VIDEO_CaptureVifMode_16BIT;
  sHwPortParam.eInColorFormat = OMX_COLOR_FormatYCbYCr;
  sHwPortParam.eScanType = OMX_VIDEO_CaptureScanTypeProgressive;
  sHwPortParam.bFieldMerged   = OMX_FALSE;
  sHwPortParam.nMaxHeight = pAppData->nInputHeight;

   if (strcmp ((char *) pAppData->mode, "1080i") == 0) {
    sHwPortParam.eScanType = OMX_VIDEO_CaptureScanTypeInterlaced;
    sHwPortParam.nMaxHeight = pAppData->nInputHeight >> 1;
    sHwPortParam.bFieldMerged   = OMX_FALSE;
   }
  
  sHwPortParam.nMaxWidth = pAppData->nInputWidth;
  sHwPortParam.nMaxChnlsPerHwPort = 1;

  eError = OMX_SetParameter (pAppData->pTvpHandle,
                             (OMX_INDEXTYPE)
                             OMX_TI_IndexParamVFCCHwPortProperties,
                             (OMX_PTR) & sHwPortParam);

  OMX_INIT_PARAM (&sVidDecParam);

  /* set the mode based on capture/display device */
  if (strcmp ((char *) pAppData->mode, "1080p") == 0)
  {
    sVidDecParam.videoStandard =  OMX_VIDEO_DECODER_STD_1080P_60;
  }
  else if (strcmp ((char *) pAppData->mode, "1080i") == 0)
  {
    sVidDecParam.videoStandard =  OMX_VIDEO_DECODER_STD_1080I_60;
  }
  else if (strcmp ((char *) pAppData->mode, "720p") == 0)
  {
    sVidDecParam.videoStandard =  OMX_VIDEO_DECODER_STD_720P_60;
  }
  else
  {
    ERROR ("Incorrect Display Mode configured!!\n");
  }
  
  /* setting TVP7002 component input */
  sVidDecParam.videoDecoderId = OMX_VID_DEC_TVP7002_DRV;

  sVidDecParam.videoSystemId = OMX_VIDEO_DECODER_VIDEO_SYSTEM_AUTO_DETECT;

  eError = OMX_SetParameter (pAppData->pTvpHandle,
                             (OMX_INDEXTYPE) OMX_TI_IndexParamCTRLVidDecInfo,
                             (OMX_PTR) & sVidDecParam);
#endif

  if (eError != OMX_ErrorNone)
    ERROR ("failed to set Ctrl Vid dec info \n");

  return (eError);
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
		pAppData->nDispInputBuffers,
		OMX_COLOR_FormatYCbYCr,

		pAppData->nEncWidth,
		pAppData->nEncHeight,
		pAppData->nEncWidth,
		(pAppData->nEncWidth * pAppData->nEncHeight * 3) >> 2,
		pAppData->nEncOutputBuffers,
		OMX_COLOR_FormatYUV420SemiPlanar,
		&pAppData->pCb
		);
}


static void IL_ClientOutputBitStreamWriteTask (void *threadsArg)
{
  //unsigned int dataRead = 0;
  OMX_ERRORTYPE err = OMX_ErrorNone;
  IL_CLIENT_COMP_PRIVATE *encILComp = NULL;
  OMX_BUFFERHEADERTYPE *pBufferOut = NULL;
  static unsigned int frameCounter = 0;

  encILComp = ((IL_ClientEnc *) threadsArg)->encILComp;

  /* use the initial i/p buffers and make empty this buffer calls */
  err = IL_ClientEncUseInitialOutputResources (encILComp);

  while (1)
  {
    /* Read filled buffer pointer from the pipe */
    read (encILComp->outPortParams->opBufPipe[0],
          &pBufferOut, sizeof (pBufferOut));

    /* write data to output file */
    fwrite (pBufferOut->pBuffer,
            sizeof (char),
            pBufferOut->nFilledLen, ((IL_ClientEnc *) threadsArg)->fOut);
    frameCounter++;
    if((frameCounter == encILComp->numFrames) || (encILComp->nUiCmd == STRM_CMD_STOP/*gILClientExit == OMX_TRUE*/))
    {
      frameCounter = 0;
      semp_post(encILComp->eos);
      pthread_exit(encILComp);
    }

    /* Pass the input buffer to the component */
    err = OMX_FillThisBuffer (encILComp->handle, pBufferOut);

    if (OMX_ErrorNone != err)
    {
      /* put back the frame in pipe and wait for state change */
      write (encILComp->outPortParams->opBufPipe[1],
             &pBufferOut, sizeof (pBufferOut));
      DBG_MSG (" waiting for action from IL Client \n");

      /* since in this example we are changing states in other thread it will
         return error for giving ETB/FTB calls in non-execute state. Since
         example is shutting down, we exit the thread */

      pthread_exit (encILComp);

    }
  }

}



/* ========================================================================== */
/**
* IL_ClientProcessPipeCmdEBD() : This function passes the bufefrs to component
* for consuming. This empty buffer will go to other component to be reused at 
* output port.
* @param thisComp        : Handle to a particular component
* @param pipeMsg         : message structure, which is written in response to 
*                          callbacks
*
*/




/* ========================================================================== */
/**
* Capture_Encode_Example() : This method is the IL Client implementation for 
* connecting capture, dei and display, and Encode OMX components. This function
*  creates configures, and connects the components.
* it manages the buffer communication.
*
* @param args         : parameters( widt,height,frame rate etc) for this function
*
*  @return      
*  OMX_ErrorNone = Successful 
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */

/* Main IL Client application to create , intiate and connect components */
int vcapchainStart(StrmCompIf *pComp)
{
	IL_ClientEnc *pAppData = (IL_ClientEnc *)pComp->pCtx;
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_U32 i, j;
	OMX_S32 ret_value;
	IL_CLIENT_PIPE_MSG pipeMsg;
	IL_CLIENT_OUTPORT_PARAMS *outPortParamsPtr = NULL;

  
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	/* Initialize application specific data structures and buffer management
		data */
	IL_ClientInit (pAppData);
  
	pAppData->fStreaming = 1;
#ifdef EN_ENCODE
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

	eError = OMX_GetHandle (&pAppData->pCapHandle, (OMX_STRING) "OMX.TI.VPSSM3.VFCC", pAppData->capILComp, &pAppData->pCb);

	DBG_MSG (" capture component is created \n");

	if ((eError != OMX_ErrorNone) || (pAppData->pCapHandle == NULL)){
		printf ("Error in Get Handle function : %s \n", IL_ClientErrorToStr (eError));
		goto EXIT;
	}

	pAppData->capILComp->handle = pAppData->pCapHandle;

	IL_ClientSetCaptureParams (pAppData);

	DBG_MSG ("enable capture output port \n");
	OMX_SendCommand (pAppData->pCapHandle, OMX_CommandPortEnable,
                   OMX_VFCC_OUTPUT_PORT_START_INDEX, NULL);

	semp_pend (pAppData->capILComp->port_sem);

	InitDeiScaler(pAppData);

#ifdef EN_ENCODE
/******************************************************************************/
  encoderInit (pAppData, pAppData->encILComp, pAppData->nEncodedFrms, 1920, 1080, 6, 6, pAppData->nFrameRate, pAppData->nBitRate);
  pAppData->pEncHandle = pAppData->encILComp->handle;
/******************************************************************************/
#endif

	displayInit(pAppData->displayId, pAppData, pAppData->disILComp,  &pAppData->pCb, pAppData->nDispWidth, pAppData->nDispHeight, pAppData->nDispInputBuffers, &pAppData->pctrlHandle);
	

	IL_ClientConnectComponents (pAppData->capILComp,
                              OMX_VFCC_OUTPUT_PORT_START_INDEX,
                              pAppData->deiILComp,
                              OMX_VFPC_INPUT_PORT_START_INDEX);

#ifdef EN_ENCODE
  DBG_MSG (" connect call for dei-encoder \n ");

  IL_ClientConnectComponents (pAppData->deiILComp,
                              OMX_VFPC_OUTPUT_PORT_START_INDEX + 1,
                              pAppData->encILComp, OMX_VIDENC_INPUT_PORT);
#endif

  DBG_MSG (" connect call for dei-display \n ");

  /* Connect the dei to display, even ports are concted to encoder, while odd
     ports would be connected to display */
  IL_ClientConnectComponents (pAppData->deiILComp,
                              OMX_VFPC_OUTPUT_PORT_START_INDEX,
                              pAppData->disILComp,
                              OMX_VFDC_INPUT_PORT_START_INDEX);


  eError =
    OMX_SendCommand (pAppData->pCapHandle, OMX_CommandStateSet,
                     OMX_StateIdle, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error in SendCommand()-OMX_StateIdle State set : %s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  /* Allocate I/O Buffers; componnet would allocated buffers and would return
     the buffer header containing the pointer to buffer */
  for (i = 0; i < pAppData->capILComp->outPortParams->nBufferCountActual; i++)
  {
    eError = OMX_AllocateBuffer (pAppData->pCapHandle,
                                 &pAppData->capILComp->outPortParams->
                                 pOutBuff[i],
                                 OMX_VFCC_OUTPUT_PORT_START_INDEX, pAppData,
                                 pAppData->capILComp->outPortParams->
                                 nBufferSize);

    if (eError != OMX_ErrorNone)
    {
      printf
        ("Capture: Error in OMX_AllocateBuffer() : %s \n",
         IL_ClientErrorToStr (eError));
      goto EXIT;
    }
  }
  DBG_MSG (" Capture outport buffers allocated \n ");

  semp_pend (pAppData->capILComp->done_sem);

  DBG_MSG (" Capture is in IDLE state \n");
/******************************************************************************/

  eError =
    OMX_SendCommand (pAppData->deiILComp->handle, OMX_CommandStateSet,
                     OMX_StateIdle, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error in SendCommand()-OMX_StateIdle State set : %s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  /* Since DEI is connected to capture output, buffers would be used from
     capture output port */
  for (i = 0; i < pAppData->deiILComp->inPortParams->nBufferCountActual; i++)
  {

    eError = OMX_UseBuffer (pAppData->deiILComp->handle,
                            &pAppData->deiILComp->inPortParams->pInBuff[i],
                            OMX_VFPC_INPUT_PORT_START_INDEX,
                            pAppData->deiILComp,
                            pAppData->capILComp->outPortParams->nBufferSize,
                            pAppData->capILComp->outPortParams->
                            pOutBuff[i]->pBuffer);

    if (eError != OMX_ErrorNone)
    {
      printf ("Error in OMX_UseBuffer()-input Port State set : %s \n",
              IL_ClientErrorToStr (eError));
      goto EXIT;
    }
  }
  DBG_MSG (" Dei input port use buffer done \n ");

  /* DEI is dual o/p port OMX component; allocate buffers on both ports */
  for (j = 0; j < pAppData->deiILComp->numOutport; j++)
  {
    outPortParamsPtr = pAppData->deiILComp->outPortParams + j;
    /* buffer alloaction for output port */
    for (i = 0; i < outPortParamsPtr->nBufferCountActual; i++)
    {
      eError = OMX_AllocateBuffer (pAppData->deiILComp->handle,
                                   &outPortParamsPtr->pOutBuff[i],
                                   OMX_VFPC_OUTPUT_PORT_START_INDEX + j,
                                   pAppData, outPortParamsPtr->nBufferSize);
      if (eError != OMX_ErrorNone)
      {
        printf
          ("Error in OMX_AllocateBuffer()-Output Port State set : %s \n",
           IL_ClientErrorToStr (eError));
        goto EXIT;
      }
    }
  }
  DBG_MSG (" DEI outport buffers allocated \n ");

  /* Wait for initialization to complete.. Wait for Idle stete of component
     after all buffers are alloacted componet would chnage to idle */

  semp_pend (pAppData->deiILComp->done_sem);

  DBG_MSG (" DEI is in IDLE state \n");

#ifdef EN_ENCODE
/*******************************************************************************/

  /* OMX_SendCommand expecting OMX_StateIdle, after this command component
     would create codec, and will wait for all buffers to be allocated */
  eError =
    OMX_SendCommand (pAppData->pEncHandle, OMX_CommandStateSet,
                     OMX_StateIdle, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error in SendCommand()-OMX_StateIdle State set : %s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }
  /* since encoder is connected to dei, buffers are supplied by dei to
     encoder, so encoder does not allocate the buffers. However it is informed
     to use the buffers created by dei. encode component would create only
     buffer headers corresponding to these buffers */

  for (i = 0; i < pAppData->encILComp->inPortParams->nBufferCountActual; i++)
  {

    outPortParamsPtr = pAppData->deiILComp->outPortParams + 1;

    eError = OMX_UseBuffer (pAppData->pEncHandle,
                            &pAppData->encILComp->inPortParams->pInBuff[i],
                            OMX_VIDENC_INPUT_PORT,
                            pAppData->encILComp,
                            outPortParamsPtr->nBufferSize,
                            outPortParamsPtr->pOutBuff[i]->pBuffer);

    if (eError != OMX_ErrorNone)
    {
      printf ("Error in encode OMX_UseBuffer(): %s \n",
              IL_ClientErrorToStr (eError));
      goto EXIT;
    }
  }
  DBG_MSG (" encoder input port use buffer done \n ");

  /* in SDK conventionally output port allocates the buffers, encode would
     create the buffers which would be consumed by filewrite thread */
  /* buffer alloaction for output port */
  for (i = 0; i < pAppData->encILComp->outPortParams->nBufferCountActual; i++)
  {
    eError = OMX_AllocateBuffer (pAppData->pEncHandle,
                                 &pAppData->encILComp->outPortParams->
                                 pOutBuff[i], OMX_VIDENC_OUTPUT_PORT,
                                 pAppData,
                                 pAppData->encILComp->outPortParams->
                                 nBufferSize);
    if (eError != OMX_ErrorNone)
    {
      printf ("Error in OMX_AllocateBuffer()-Output Port State set : %s \n",
              IL_ClientErrorToStr (eError));
      goto EXIT;
    }
  }

  DBG_MSG (" encoder outport buffers allocated \n ");

  semp_pend (pAppData->encILComp->done_sem);

  DBG_MSG (" Encoder state IDLE \n ");
/******************************************************************************/
#endif

  /* control component does not allocate any data buffers, It's interface is
     though as it is omx componenet */
  eError =
    OMX_SendCommand (pAppData->pctrlHandle, OMX_CommandStateSet,
                     OMX_StateIdle, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error in SendCommand()-OMX_StateIdle State set : %s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  semp_pend (pAppData->disILComp->done_sem);

  DBG_MSG (" ctrl-dc state IDLE \n ");

  eError =
    OMX_SendCommand (pAppData->disILComp->handle, OMX_CommandStateSet,
                     OMX_StateIdle, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error in SendCommand()-OMX_StateIdle State set : %s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }
  /* Since display has only input port and buffers are already created by DEI
     component, only use_buffer call is used at input port. there is no output
     port in the display component */
  for (i = 0; i < pAppData->disILComp->inPortParams->nBufferCountActual; i++)
  {

    outPortParamsPtr = pAppData->deiILComp->outPortParams;

    eError = OMX_UseBuffer (pAppData->disILComp->handle,
                            &pAppData->disILComp->inPortParams->pInBuff[i],
                            OMX_VFDC_INPUT_PORT_START_INDEX,
                            pAppData->disILComp,
                            outPortParamsPtr->nBufferSize,
                            outPortParamsPtr->pOutBuff[i]->pBuffer);

    if (eError != OMX_ErrorNone)
    {
      printf ("Error in Display OMX_UseBuffer()- %s \n",
              IL_ClientErrorToStr (eError));
      goto EXIT;
    }
  }
  DBG_MSG (" display buffers allocated \n waiting for IDLE");
  semp_pend (pAppData->disILComp->done_sem);

  DBG_MSG (" display state IDLE \n ");
/******************************************************************************/

  /* change state tho execute, so that component can accept buffers from IL
     client. Please note the ordering of components is from consumer to
     producer component i.e. capture-dei-encoder/display */
  eError =
    OMX_SendCommand (pAppData->pctrlHandle, OMX_CommandStateSet,
                     OMX_StateExecuting, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error in SendCommand()-OMX_StateIdle State set : %s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  semp_pend (pAppData->disILComp->done_sem);

  DBG_MSG (" display control state execute \n ");

  /* change state to execute so that buffers processing can start */
  eError =
    OMX_SendCommand (pAppData->disILComp->handle, OMX_CommandStateSet,
                     OMX_StateExecuting, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Executing State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  semp_pend (pAppData->disILComp->done_sem);

  DBG_MSG (" display state execute \n ");

#ifdef EN_ENCODE
/******************************************************************************/

  /* change state to execute so that buffers processing can start */
  eError =
    OMX_SendCommand (pAppData->pEncHandle, OMX_CommandStateSet,
                     OMX_StateExecuting, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Executing State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  semp_pend (pAppData->encILComp->done_sem);

  DBG_MSG (" encoder state execute \n ");
/******************************************************************************/
#endif

  /* change state to execute so that buffers processing can start */
  eError =
    OMX_SendCommand (pAppData->deiILComp->handle, OMX_CommandStateSet,
                     OMX_StateExecuting, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Executing State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  semp_pend (pAppData->deiILComp->done_sem);

  DBG_MSG (" dei state execute \n ");
/******************************************************************************/

  /* change state to execute so that buffers processing can start */
  eError =
    OMX_SendCommand (pAppData->pCapHandle, OMX_CommandStateSet,
                     OMX_StateExecuting, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Executing State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  semp_pend (pAppData->capILComp->done_sem);

  DBG_MSG (" capture state execute \n ");

#if 0
  eError =
    OMX_SendCommand (pAppData->pTvpHandle, OMX_CommandStateSet,
                     OMX_StateExecuting, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Executing State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  semp_pend (pAppData->capILComp->done_sem);

  DBG_MSG (" capture control ( TVP ) state execute \n ");
#endif



#ifdef EN_ENCODE
  /* Create thread for writing bitstream and passing the buffers to encoder
     component */
  pthread_attr_init (&pAppData->encILComp->ThreadAttr);

  if (0 !=
      pthread_create (&pAppData->encILComp->outDataStrmThrdId,
                      &pAppData->encILComp->ThreadAttr,
                      (ILC_StartFcnPtr) IL_ClientOutputBitStreamWriteTask, pAppData))
  {
    printf ("Create_Task failed !");
    goto EXIT;
  }

  DBG_MSG (" file write thread created \n ");

  pthread_attr_init (&pAppData->encILComp->ThreadAttr);

  /* These threads are created for each component to pass the buffers to each
     other. this thread function reads the buffers from pipe and feeds it to
     component or for processed buffers, passes the buffers to connected
     component */
  if (0 !=
      pthread_create (&pAppData->encILComp->connDataStrmThrdId,
                      &pAppData->encILComp->ThreadAttr,
                      (ILC_StartFcnPtr) IL_ClientConnInConnOutTask, pAppData->encILComp))
  {
    printf ("Create_Task failed !");
    goto EXIT;
  }

  DBG_MSG (" encode connect thread created \n ");
#endif

  pthread_attr_init (&pAppData->deiILComp->ThreadAttr);

  if (0 !=
      pthread_create (&pAppData->deiILComp->connDataStrmThrdId,
                      &pAppData->deiILComp->ThreadAttr,
                      (ILC_StartFcnPtr) IL_ClientConnInConnOutTask, pAppData->deiILComp))
  {
    printf ("Create_Task failed !");
    goto EXIT;
  }

  DBG_MSG (" dei connect thread created \n ");

  pthread_attr_init (&pAppData->capILComp->ThreadAttr);

  if (0 !=
      pthread_create (&pAppData->capILComp->connDataStrmThrdId,
                      &pAppData->capILComp->ThreadAttr,
                      (ILC_StartFcnPtr) IL_ClientConnInConnOutTask, pAppData->capILComp))
  {
    printf ("Create_Task failed !");
    goto EXIT;
  }
  DBG_MSG (" capture connect thread created \n ");

  if (0 !=
      pthread_create (&pAppData->disILComp->connDataStrmThrdId,
                      &pAppData->disILComp->ThreadAttr,
                      (ILC_StartFcnPtr) IL_ClientConnInConnOutTask, pAppData->disILComp))
  {
    printf ("Create_Task failed !");
    goto EXIT;
  }
  DBG_MSG (" display connect thread created \n ");

  DBG_MSG (" executing the application now!! \n");

/******************************************************************************/
  /* Waiting for this semaphore to be posted by the bitstream write thread */
  semp_pend(pAppData->encILComp->eos);
/******************************************************************************/
  DBG_MSG(" tearing down the capture-encode example\n ");

  /* tear down sequence */

  /* change the state to idle */
  /* before changing state to idle, buffer communication to component should be 
     stoped , writing an exit message to threads */

  pipeMsg.cmd = IL_CLIENT_PIPE_CMD_EXIT;

  write (pAppData->deiILComp->localPipe[1],
         &pipeMsg, sizeof (IL_CLIENT_PIPE_MSG));

  write (pAppData->capILComp->localPipe[1],
         &pipeMsg, sizeof (IL_CLIENT_PIPE_MSG));

  write (pAppData->disILComp->localPipe[1],
         &pipeMsg, sizeof (IL_CLIENT_PIPE_MSG));

#ifdef EN_ENCODE
  write (pAppData->encILComp->localPipe[1],
         &pipeMsg, sizeof (IL_CLIENT_PIPE_MSG));
#endif

  /* change state to idle so that buffers processing would stop */
  eError =
    OMX_SendCommand (pAppData->pCapHandle, OMX_CommandStateSet,
                     OMX_StateIdle, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Idle State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  semp_pend (pAppData->capILComp->done_sem);
  DBG_MSG (" capture state idle \n ");

#if 0
  /* change state to idle so that buffers processing would stop */
  eError =
    OMX_SendCommand(pAppData->pTvpHandle, OMX_CommandStateSet,
                    OMX_StateIdle, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf("Error from SendCommand-Idle State set :%s \n",
           IL_ClientErrorToStr(eError));
    goto EXIT;
  }

  semp_pend(pAppData->capILComp->done_sem);
  DBG_MSG(" control tvp state idle \n ");
#endif

  /* change state to idle so that buffers processing can stop */
  eError =
    OMX_SendCommand (pAppData->deiILComp->handle, OMX_CommandStateSet,
                     OMX_StateIdle, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Idle State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  semp_pend (pAppData->deiILComp->done_sem);

  DBG_MSG (" DEI state idle \n ");

  /* change state to execute so that buffers processing can stop */
  eError =
    OMX_SendCommand (pAppData->disILComp->handle, OMX_CommandStateSet,
                     OMX_StateIdle, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Idle State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  semp_pend (pAppData->disILComp->done_sem);

  DBG_MSG (" display state idle \n ");

  /* change state to execute so that buffers processing can stop */
  eError =
    OMX_SendCommand(pAppData->pctrlHandle, OMX_CommandStateSet,
                    OMX_StateIdle, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf("Error from SendCommand-Idle State set :%s \n",
           IL_ClientErrorToStr(eError));
    goto EXIT;
  }

  semp_pend(pAppData->disILComp->done_sem);

  DBG_MSG(" display control state idle \n ");

#ifdef EN_ENCODE
  /* change state to execute so that buffers processing can stop */
  eError =
  OMX_SendCommand (pAppData->pEncHandle, OMX_CommandStateSet,
                   OMX_StateIdle, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Idle State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  semp_pend (pAppData->encILComp->done_sem);

  DBG_MSG (" Encoder state idle \n ");
#endif

/******************************************************************************/

  eError =
    OMX_SendCommand (pAppData->disILComp->handle, OMX_CommandStateSet,
                     OMX_StateLoaded, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Idle State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }
  /* During idle-> loaded state transition buffers need to be freed up */
  for (i = 0; i < pAppData->disILComp->inPortParams->nBufferCountActual; i++)
  {
    eError =
      OMX_FreeBuffer (pAppData->disILComp->handle,
                      OMX_VFDC_INPUT_PORT_START_INDEX,
                      pAppData->disILComp->inPortParams->pInBuff[i]);
    if (eError != OMX_ErrorNone)
    {
      printf ("Error in OMX_FreeBuffer : %s \n", IL_ClientErrorToStr (eError));
      goto EXIT;
    }
  }

  semp_pend (pAppData->disILComp->done_sem);

  DBG_MSG (" display state loaded \n ");

  /* control component does not alloc/free any data buffers, It's interface
     is though as it is omx componenet */
  eError =
    OMX_SendCommand(pAppData->pctrlHandle, OMX_CommandStateSet,
                    OMX_StateLoaded, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf("Error in SendCommand()-OMX_StateLoaded State set : %s \n",
           IL_ClientErrorToStr(eError));
    goto EXIT;
  }

  semp_pend(pAppData->disILComp->done_sem);

  DBG_MSG(" ctrl-dc state loaded \n ");

/******************************************************************************/

#ifdef EN_ENCODE
  /* change the encoder state to loded */
  eError =
    OMX_SendCommand (pAppData->pEncHandle, OMX_CommandStateSet,
                     OMX_StateLoaded, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Idle State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }
  /* During idle-> loaded state transition buffers need to be freed up */
  for (i = 0; i < pAppData->encILComp->inPortParams->nBufferCountActual; i++)
  {
    eError =
      OMX_FreeBuffer (pAppData->pEncHandle, OMX_VIDENC_INPUT_PORT,
                      pAppData->encILComp->inPortParams->pInBuff[i]);
    if (eError != OMX_ErrorNone)
    {
      printf ("Error in OMX_FreeBuffer : %s \n", IL_ClientErrorToStr (eError));
      goto EXIT;
    }
  }

  for (i = 0; i < pAppData->encILComp->outPortParams->nBufferCountActual; i++)
  {
    eError =
      OMX_FreeBuffer (pAppData->pEncHandle, OMX_VIDENC_OUTPUT_PORT,
                      pAppData->encILComp->outPortParams->pOutBuff[i]);
    if (eError != OMX_ErrorNone)
    {
      printf ("Error in OMX_FreeBuffer : %s \n", IL_ClientErrorToStr (eError));
      goto EXIT;
    }
  }

  semp_pend (pAppData->encILComp->done_sem);

  DBG_LOG(DBGLVL_TRACE, (" encoder state loaded"));
#endif
/******************************************************************************/

  eError =
    OMX_SendCommand (pAppData->deiILComp->handle, OMX_CommandStateSet,
                     OMX_StateLoaded, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Idle State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }
  /* During idle-> loaded state transition buffers need to be freed up */
  for (i = 0; i < pAppData->deiILComp->inPortParams->nBufferCountActual; i++)
  {
    eError =
      OMX_FreeBuffer (pAppData->deiILComp->handle,
                      OMX_VFPC_INPUT_PORT_START_INDEX,
                      pAppData->deiILComp->inPortParams->pInBuff[i]);
    if (eError != OMX_ErrorNone)
    {
      printf ("Error in OMX_FreeBuffer : %s \n", IL_ClientErrorToStr (eError));
      goto EXIT;
    }
  }

  /* DEI is dual o/p port OMX component; allocate buffers on both ports */
  for (j = 0; j < pAppData->deiILComp->numOutport; j++)
  {
    outPortParamsPtr = pAppData->deiILComp->outPortParams + j;
    /* buffer alloaction for output port */
    for (i = 0; i < outPortParamsPtr->nBufferCountActual; i++)
    {
      eError = OMX_FreeBuffer (pAppData->deiILComp->handle,
                               OMX_VFPC_OUTPUT_PORT_START_INDEX + j,
                               outPortParamsPtr->pOutBuff[i]);
      if (eError != OMX_ErrorNone)
      {
        printf ("Error in OMX_AllocateBuffer()-Output Port State set : %s \n",
                IL_ClientErrorToStr (eError));
        goto EXIT;
      } /* if (eError) */
    } /* for (i) */
  } /* for (j) */

  semp_pend (pAppData->deiILComp->done_sem);

  DBG_LOG(DBGLVL_TRACE, (" dei state loaded"));

/******************************************************************************/

  eError =
    OMX_SendCommand (pAppData->pCapHandle, OMX_CommandStateSet,
                     OMX_StateLoaded, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Idle State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }
  /* During idle-> loaded state transition buffers need to be freed up */
  for (i = 0; i < pAppData->capILComp->outPortParams->nBufferCountActual; i++)
  {
    eError =
      OMX_FreeBuffer (pAppData->pCapHandle,
                      OMX_VFCC_OUTPUT_PORT_START_INDEX,
                      pAppData->capILComp->outPortParams->pOutBuff[i]);
    if (eError != OMX_ErrorNone)
    {
      printf ("Error in OMX_FreeBuffer : %s \n", IL_ClientErrorToStr (eError));
      goto EXIT;
    }
  }

  semp_pend (pAppData->capILComp->done_sem);

  DBG_MSG (" capture state loaded \n ");

#if 0
  /* ctrl tvp component does not alloc/free any data buffers, It's interface
     is though as it is omx componenet */
  eError =
    OMX_SendCommand(pAppData->pTvpHandle, OMX_CommandStateSet,
                    OMX_StateLoaded, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf("Error in SendCommand()-OMX_StateLoaded State set : %s \n",
           IL_ClientErrorToStr(eError));
    goto EXIT;
  }

  semp_pend(pAppData->capILComp->done_sem);

  DBG_MSG(" ctrl-tvp state loaded \n ");
#endif 

/******************************************************************************/

  /* free handle for all component */

  eError = OMX_FreeHandle (pAppData->pCapHandle);
  if ((eError != OMX_ErrorNone))
  {
    printf ("Error in Free Handle function : %s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }
  DBG_MSG (" capture free handle \n");

#if 0
  eError = OMX_FreeHandle(pAppData->pTvpHandle);
  if ((eError != OMX_ErrorNone))
  {
    printf("Error in Free Handle function : %s \n",
           IL_ClientErrorToStr(eError));
    goto EXIT;
  }

  DBG_MSG(" ctrl-tvp free handle \n");
#endif

#ifdef EN_ENCODE
  eError = OMX_FreeHandle (pAppData->pEncHandle);
  if ((eError != OMX_ErrorNone))
  {
    printf ("Error in Free Handle function : %s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }
  DBG_MSG (" encoder free handle \n");
#endif

  eError = OMX_FreeHandle (pAppData->deiILComp->handle);
  if ((eError != OMX_ErrorNone))
  {
    printf ("Error in Free Handle function : %s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

	DBG_LOG(DBGLVL_TRACE, (" dei free handle"));

  eError = OMX_FreeHandle (pAppData->disILComp->handle);
  if ((eError != OMX_ErrorNone))
  {
    printf ("Error in Free Handle function : %s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

	  DBG_LOG(DBGLVL_TRACE, (" display free handle"));
	eError = OMX_FreeHandle(pAppData->pctrlHandle);
	if ((eError != OMX_ErrorNone))
	{
		printf("Error in Free Handle function : %s \n",
			IL_ClientErrorToStr(eError));
		goto EXIT;
	}

  DBG_LOG(DBGLVL_TRACE, (" ctrl-dc free handle"));
#ifdef EN_ENCODE
	if (pAppData->fOut != NULL)	{
		fclose(pAppData->fOut);
		pAppData->fOut = NULL;
	}
#endif
  /* terminate the threads */

#ifdef EN_ENCODE
  pthread_join (pAppData->encILComp->connDataStrmThrdId, (void **) &ret_value);
#endif
  DBG_LOG(DBGLVL_TRACE, ("Wait for deiILComp->connDataStrmThrdId"));
  pthread_join (pAppData->deiILComp->connDataStrmThrdId, (void **) &ret_value);

  DBG_LOG(DBGLVL_TRACE, ("Wait for capILComp->connDataStrmThrdId"));
  pthread_join (pAppData->capILComp->connDataStrmThrdId, (void **) &ret_value);

   DBG_LOG(DBGLVL_TRACE, ("Wait for disILComp->connDataStrmThrdId"));
  pthread_join (pAppData->disILComp->connDataStrmThrdId, (void **) &ret_value);

#ifdef EN_ENCODE
	DBG_LOG(DBGLVL_TRACE, ("Wait for encILComp->connDataStrmThrdId"));
	pthread_join(pAppData->encILComp->outDataStrmThrdId, (void **) &ret_value);
#endif

EXIT:
  DBG_LOG(DBGLVL_TRACE, ("Leave"));
  pAppData->fStreaming = 0;
  return (0);
}


void vcapchainStop(StrmCompIf *pComp)
{
	void *ret_value;
	int nTimeout = 3000000;
	IL_ClientEnc *pAppData = (IL_ClientEnc *)pComp->pCtx;

	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	//pAppData->nUiCmd = STRM_CMD_STOP;
	semp_post(pAppData->encILComp->eos);

	DBG_LOG(DBGLVL_SETUP, ("[Wait for IL_ClientInputBitStreamReadTask to stop(Timeout=%dms) ...",nTimeout/1000));
	while(pAppData->fStreaming == OMX_TRUE && nTimeout > 0) {
		usleep(1000);
		nTimeout -= 1000;
	}
	
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
}

int vcapchainnDelete(StrmCompIf *pComp)
{
	IL_ClientEnc *pAppData = (IL_ClientEnc *)pComp->pCtx;
	if(pAppData) {
		IL_ClientDeInit (pAppData);
		free (pAppData);
	}
	return 0;
}

extern char   *gIniFile;
#define DEBUG_SECTION   "debug"

StrmCompIf *
vcapchainCreate()
{
	StrmCompIf *pComp = (StrmCompIf *)malloc(sizeof(StrmCompIf));
	memset (pComp, 0x0, sizeof (StrmCompIf));
	IL_ClientEnc *pAppData = (IL_ClientEnc *) malloc (sizeof (IL_ClientEnc));
	memset (pAppData, 0x0, sizeof (IL_ClientEnc));
	pComp->pCtx = pAppData;

	pComp->Open= NULL;
	pComp->SetOption = vcapchainSetOption;
	pComp->SetInputConn= NULL;
	pComp->SetOutputConn= NULL;
	pComp->SetClkSrc = NULL;//vidchainSetClkSrc;
	pComp->GetClkSrc = NULL;
	pComp->Start = vcapchainStart;
	pComp->Stop = vcapchainStop;
	pComp->Close = NULL;
	pComp->Delete = vcapchainnDelete;
	return pComp;
}

