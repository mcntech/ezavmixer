/*--------------------- system and platform files ----------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <xdc/std.h>

#define CODEC_H264ENC      // Ram: This definition is required when compiling this component as library
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
#include <omx_vswmosaic.h>
#include "dec_platform_utils.h"
#include "ilclient_common.h"

/******************************************************************************\
 *      Includes
\******************************************************************************/
#include "semp.h"
#include "dbglog.h"

#define CROP_START_X    0
#define CROP_START_Y    0

/* ========================================================================== */
/**
* IL_ClientErrorToStr() : Function to map the OMX error enum to string
*
* @param error   : OMX Error type
*
*  @return      
*  String conversion of the OMX_ERRORTYPE
*
*/
/* ========================================================================== */

OMX_STRING IL_ClientErrorToStr (OMX_ERRORTYPE error)
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


OMX_ERRORTYPE ConfigureDisplay(
	int            displayId,
	OMX_HANDLETYPE pDisHandle,
	OMX_HANDLETYPE pctrlHandle,
	int            nWidth,
	int            nHeight,
	int            nBufferCount
	)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_PARAM_BUFFER_MEMORYTYPE memTypeCfg;
	OMX_PARAM_PORTDEFINITIONTYPE paramPort;
	//OMX_PARAM_VFPC_NUMCHANNELPERHANDLE sNumChPerHandle;
	OMX_PARAM_VFDC_DRIVERINSTID driverId;
	OMX_PARAM_VFDC_CREATEMOSAICLAYOUT mosaicLayout;
	OMX_CONFIG_VFDC_MOSAICLAYOUT_PORT2WINMAP port2Winmap;
	DBG_LOG(DBGLVL_TRACE,("Enter"));
	DBG_LOG(DBGLVL_SETUP, ("displayId=%d nInWidth=%d nInHeight=%d nBufferCount=%d",displayId, nWidth, nHeight, nBufferCount));

	OMX_INIT_PARAM (&paramPort);

	/* set input height/width and color format */
	paramPort.nPortIndex = OMX_VFDC_INPUT_PORT_START_INDEX;
	OMX_GetParameter (pDisHandle, OMX_IndexParamPortDefinition, &paramPort);
	paramPort.nPortIndex = OMX_VFDC_INPUT_PORT_START_INDEX;
	paramPort.format.video.nFrameWidth = nWidth;
	paramPort.format.video.nFrameHeight = nHeight;
	paramPort.format.video.nStride = nWidth * 2;
	paramPort.nBufferCountActual = nBufferCount;
	paramPort.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
	paramPort.format.video.eColorFormat = OMX_COLOR_FormatYCbYCr;

	paramPort.nBufferSize = paramPort.format.video.nStride * nHeight;
	paramPort.nBufferSize = paramPort.format.video.nStride * nHeight;
  
	OMX_SetParameter (pDisHandle, OMX_IndexParamPortDefinition,  &paramPort);

  /* --------------------------------------------------------------------------*
     Supported display IDs by VFDC and DC are below The names will be renamed in
     future releases as some of the driver names & interfaces will be changed in
     future @ param OMX_VIDEO_DISPLAY_ID_HD0: 422P On-chip HDMI @ param
     OMX_VIDEO_DISPLAY_ID_HD1: 422P HDDAC component output @ param
     OMX_VIDEO_DISPLAY_ID_SD0: 420T/422T SD display (NTSC): Not supported yet.
     ------------------------------------------------------------------------ */
  /* set the parameter to the disaply component to 1080P @60 mode */
	OMX_INIT_PARAM (&driverId);
	/* Configured to use on-chip HDMI */
	driverId.nDrvInstID = OMX_VIDEO_DISPLAY_ID_HD0;
	driverId.eDispVencMode = OMX_DC_MODE_1080P_60;
	
	TRACE_PROGRESS
	if (1 == displayId)  {
		/* Configured to use LCD Display */
		driverId.nDrvInstID = OMX_VIDEO_DISPLAY_ID_HD1;
		driverId.eDispVencMode = DISPLAY_VENC_MODE;
	}
	eError = OMX_SetParameter (pDisHandle, (OMX_INDEXTYPE) OMX_TI_IndexParamVFDCDriverInstId,  &driverId);
	if (eError != OMX_ErrorNone)  {
		ERROR ("failed to set driver mode to 1080P@60\n");
	}

  /* set the parameter to the disaply controller component to 1080P @60 mode */
	OMX_INIT_PARAM (&driverId);
  /* Configured to use on-chip HDMI */

	/* Configured to use on-chip HDMI */
	driverId.nDrvInstID = OMX_VIDEO_DISPLAY_ID_HD0;
	driverId.eDispVencMode = OMX_DC_MODE_1080P_60;
	if (1 == displayId)  {
		/* Configured to use LCD Display */
		driverId.nDrvInstID = OMX_VIDEO_DISPLAY_ID_HD1;
		driverId.eDispVencMode = DISPLAY_VENC_MODE;
	}
	eError = OMX_SetParameter (pctrlHandle, (OMX_INDEXTYPE) OMX_TI_IndexParamVFDCDriverInstId, &driverId);
	if (eError != OMX_ErrorNone) {
		ERROR ("failed to set driver mode to 1080P@60\n");
	}

	if (1 == displayId) {
		disp_IL_ClientSetSecondaryDisplayParams(pDisHandle, pctrlHandle);
	}

	/* set mosaic layout info */

	OMX_INIT_PARAM (&mosaicLayout);
	/* Configuring the first (and only) window */
	mosaicLayout.sMosaicWinFmt[0].winStartX = 0;
	mosaicLayout.sMosaicWinFmt[0].winStartY = 0;
	// TODO: Verify that following 3 lines are correct for HDMI for future use
	mosaicLayout.sMosaicWinFmt[0].winWidth = nWidth;
	mosaicLayout.sMosaicWinFmt[0].winHeight = nHeight;
	mosaicLayout.sMosaicWinFmt[0].pitch[VFDC_YUV_INT_ADDR_IDX] = nWidth * 2;
	mosaicLayout.sMosaicWinFmt[0].dataFormat = VFDC_DF_YUV422I_YVYU;
	mosaicLayout.sMosaicWinFmt[0].bpp = VFDC_BPP_BITS16;
	mosaicLayout.sMosaicWinFmt[0].priority = 0;
	mosaicLayout.nDisChannelNum = 0;
	/* Only one window in this layout, hence setting it to 1 */
	mosaicLayout.nNumWindows = 1;

	if (1 == displayId) {
		/* For secondary Display, start the window at (0,0), since it is 
		   scaled to display device size */
		mosaicLayout.sMosaicWinFmt[0].winStartX = 0;
		mosaicLayout.sMosaicWinFmt[0].winStartY = 0;
		mosaicLayout.sMosaicWinFmt[0].winWidth = nWidth;
		mosaicLayout.sMosaicWinFmt[0].winHeight = nHeight;
		mosaicLayout.sMosaicWinFmt[0].pitch[VFDC_YUV_INT_ADDR_IDX] = nWidth * 2;
	}    
	eError = OMX_SetParameter (pDisHandle, (OMX_INDEXTYPE) OMX_TI_IndexParamVFDCCreateMosaicLayout,  &mosaicLayout);
	if (eError != OMX_ErrorNone){
		ERROR ("failed to set mosaic window parameter\n");
	}

	/* map OMX port to window */
	OMX_INIT_PARAM (&port2Winmap);
	/* signifies the layout id this port2win mapping refers to */
	port2Winmap.nLayoutId = 0;
	/* Just one window in this layout, hence setting the value to 1 */
	port2Winmap.numWindows = 1;
	/* Only 1st input port used here */
	port2Winmap.omxPortList[0] = OMX_VFDC_INPUT_PORT_START_INDEX + 0;
	eError = OMX_SetConfig (pDisHandle, (OMX_INDEXTYPE)OMX_TI_IndexConfigVFDCMosaicPort2WinMap, &port2Winmap);
	if (eError != OMX_ErrorNone)	{
		ERROR ("failed to map port to windows\n");
	}

	/* Setting Memory type at input port to Raw Memory */
	DBG_MSG ("setting input and output memory type to default");
	OMX_INIT_PARAM (&memTypeCfg);
	memTypeCfg.nPortIndex = OMX_VFPC_INPUT_PORT_START_INDEX;
	memTypeCfg.eBufMemoryType = OMX_BUFFER_MEMORY_DEFAULT;
	eError = OMX_SetParameter (pDisHandle, OMX_TI_IndexParamBuffMemType,	&memTypeCfg);

	if (eError != OMX_ErrorNone)	{
		ERROR ("failed to set memory Type at input port\n");
	}
	DBG_LOG(DBGLVL_TRACE,("Leave"));
	return (eError);
}

/* ========================================================================== */
/**
* IL_ClientSetSwMosaicParams() : Function to fill the port definition 
* structures and call the Set_Parameter function on to the scalar
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

OMX_ERRORTYPE IL_ClientSetSwMosaicParams (
	OMX_HANDLETYPE pVswmosaicHandle,
	int            nNumWindows,
	WINDOW_PARAM_T *pWindParam,
	WINDOW_PARAM_T *pWindOutParam
	)
{
	int i;
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_PARAM_BUFFER_MEMORYTYPE memTypeCfg;
	OMX_PARAM_PORTDEFINITIONTYPE paramPort;
#ifdef OMX_05_02_00_30
	OMX_PARAM_VSWMOSAIC_CREATEMOSAICLAYOUT   sMosaic;
#else
	OMX_CONFIG_VSWMOSAIC_CREATEMOSAICLAYOUT  sMosaic;
#endif
	WINDOW_PARAM_T *pWind;
	OMX_INIT_PARAM (&memTypeCfg);
	memTypeCfg.eBufMemoryType = OMX_BUFFER_MEMORY_DEFAULT;

	DBG_LOG(DBGLVL_SETUP, ("nNumWindows=%d", nNumWindows));
	for ( i = 0; i < nNumWindows; i++) {
		memTypeCfg.nPortIndex = OMX_VSWMOSAIC_INPUT_PORT_START_INDEX + i;
		eError = OMX_SetParameter (pVswmosaicHandle, OMX_TI_IndexParamBuffMemType, &memTypeCfg);
		if (eError != OMX_ErrorNone){
			ERROR ("failed to set memory Type at input port\n");
		}
	}

	/* Setting Memory type at output port to Raw Memory */
	OMX_INIT_PARAM (&memTypeCfg);
	memTypeCfg.nPortIndex = OMX_VSWMOSAIC_OUTPUT_PORT_START_INDEX;
	memTypeCfg.eBufMemoryType = OMX_BUFFER_MEMORY_DEFAULT;
	eError = OMX_SetParameter (pVswmosaicHandle, OMX_TI_IndexParamBuffMemType, &memTypeCfg);

	if (eError != OMX_ErrorNone)  {
		ERROR ("failed to set memory Type at output port\n");
	}
	pWind = pWindParam;
	for ( i = 0; i < nNumWindows; i++) {
		/* set input height/width and color format */
		OMX_INIT_PARAM (&paramPort);
		paramPort.nPortIndex = OMX_VSWMOSAIC_INPUT_PORT_START_INDEX + i;

		OMX_GetParameter (pVswmosaicHandle, OMX_IndexParamPortDefinition, &paramPort);
		paramPort.nPortIndex = OMX_VFPC_INPUT_PORT_START_INDEX + i;
		paramPort.format.video.nFrameWidth  = pWind->nWidth;//SWMOSAIC_WINDOW_WIDTH; 
		paramPort.format.video.nFrameHeight = pWind->nHeight;//SWMOSAIC_WINDOW_HEIGHT;
		/* swmosaic is connceted to scalar, whose stride is different than width*/
		paramPort.format.video.nStride = pWind->nStride;//SWMOSAIC_WINDOW_WIDTH * 2;
		paramPort.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
		paramPort.format.video.eColorFormat = OMX_COLOR_FormatYCbYCr;
		paramPort.nBufferSize = (paramPort.format.video.nStride * paramPort.format.video.nFrameHeight);

		paramPort.nBufferAlignment = 0;
		paramPort.bBuffersContiguous = 0;
		paramPort.nBufferCountActual = pWind->nBufferCount;

		DBG_LOG(DBGLVL_SETUP, ("Inport window=%d  (width=%u, height=%u stride=%u size=%u buffers=%d)",	i, (unsigned int)paramPort.format.video.nFrameWidth, (unsigned int)paramPort.format.video.nFrameHeight, paramPort.format.video.nStride, paramPort.nBufferSize, paramPort.nBufferCountActual));
		OMX_SetParameter (pVswmosaicHandle, OMX_IndexParamPortDefinition, &paramPort);
		pWind++;
	}
  
	/* set output height/width and color format */
	OMX_INIT_PARAM (&paramPort);
	paramPort.nPortIndex = OMX_VSWMOSAIC_OUTPUT_PORT_START_INDEX;
	OMX_GetParameter (pVswmosaicHandle, OMX_IndexParamPortDefinition, &paramPort);

	paramPort.nPortIndex = OMX_VSWMOSAIC_OUTPUT_PORT_START_INDEX;
	paramPort.format.video.nFrameWidth  = pWindOutParam->nWidth;  //HD_WIDTH;
	paramPort.format.video.nFrameHeight = pWindOutParam->nHeight; //HD_HEIGHT;
	paramPort.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
	paramPort.format.video.eColorFormat = OMX_COLOR_FormatYCbYCr;
	paramPort.nBufferAlignment = 0;
	paramPort.bBuffersContiguous = 0;
	paramPort.nBufferCountActual = pWindOutParam->nBufferCount;//IL_CLIENT_SCALAR_OUTPUT_BUFFER_COUNT;
	paramPort.format.video.nStride = pWindOutParam->nStride;//HD_WIDTH * 2;
	paramPort.nBufferSize = paramPort.format.video.nStride * paramPort.format.video.nFrameHeight;

	DBG_LOG(DBGLVL_SETUP, ("OutPort (width=%d, height=%d stride=%d size=%d buffers=%d)", (int) paramPort.format.video.nFrameWidth, (int) paramPort.format.video.nFrameHeight, paramPort.format.video.nStride,paramPort.nBufferSize, pWindOutParam->nBufferCount));
      

	OMX_SetParameter (pVswmosaicHandle, OMX_IndexParamPortDefinition,	&paramPort);

	OMX_INIT_PARAM (&sMosaic);
	sMosaic.nPortIndex  = OMX_VSWMOSAIC_OUTPUT_PORT_START_INDEX;
  
	sMosaic.nOpWidth    = pWindOutParam->nWidth;//HD_WIDTH;  /* Width in pixels  */
	sMosaic.nOpHeight   = pWindOutParam->nHeight;//HD_HEIGHT; /* Height in pixels */
	sMosaic.nOpPitch    = pWindOutParam->nStride; //HD_WIDTH*2; /* Pitch in bytes   */

	DBG_LOG(DBGLVL_SETUP, ("Mosaic: Width=%d, Height=%d, pitch=%d\n", (int) sMosaic.nOpWidth,(int) sMosaic.nOpHeight, (int) sMosaic.nOpPitch));

	sMosaic.nNumWindows = nNumWindows;
  
	pWind = pWindParam;
	for ( i = 0; i < nNumWindows; i++) {
		sMosaic.sMosaicWinFmt[i].dataFormat = OMX_COLOR_FormatYCbYCr;
		sMosaic.sMosaicWinFmt[i].nPortIndex = i;
		sMosaic.sMosaicWinFmt[i].pitch[0]   = pWind->nStride;
		sMosaic.sMosaicWinFmt[i].winStartX  = pWind->nStartX;
		sMosaic.sMosaicWinFmt[i].winStartY  = pWind->nStartY;
		sMosaic.sMosaicWinFmt[i].winWidth   = pWind->nWidth;
		sMosaic.sMosaicWinFmt[i].winHeight  = pWind->nHeight;
		DBG_LOG(DBGLVL_SETUP, ("Mosaic Window port=%d  (x=%d y=%d w=%u, h=%u pitch=%u",i, pWind->nStartX, pWind->nStartY, pWind->nWidth, pWind->nHeight, pWind->nStride));
		pWind++;
	}
	eError = OMX_SetConfig (pVswmosaicHandle, OMX_TI_IndexConfigVSWMOSAICCreateMosaicLayout, &sMosaic);
	if (eError != OMX_ErrorNone) {
		ERROR ("Failed to set OMX_TI_IndexConfigVSWMOSAICCreateMosaicLayout for output \n");
	} else {
		printf ("Mosaic layout configuration:: Successful \n");
	}

	return (eError);
}

OMX_ERRORTYPE IL_ClientInitSwMosaic( 
	IL_CLIENT_COMP_PRIVATE *vswmosaicILComp,
	int                    nNumWindows,
	WINDOW_PARAM_T         *pWindInParam,
	WINDOW_PARAM_T         *pWindOutParam,
	OMX_CALLBACKTYPE       *pvswmosaicCb
	)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_HANDLETYPE         pHandle = NULL;
	int i;

	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	TRACE_PROGRESS

	eError =  OMX_GetHandle (&pHandle, "OMX.TI.VPSSM3.VSWMOSAIC", vswmosaicILComp, pvswmosaicCb);
	TRACE_PROGRESS
	if (eError != OMX_ErrorNone) {
		ERROR ("failed to get handle\n");
	}
	strcpy(vswmosaicILComp->comp_name, "OMX.TI.VPSSM3.VSWMOSAIC");
	TRACE_PROGRESS

	vswmosaicILComp->handle = pHandle;

	TRACE_PROGRESS
    IL_ClientSetSwMosaicParams (pHandle, nNumWindows, pWindInParam, pWindOutParam);

	for (i = 0; i < nNumWindows; i++) {
		TRACE_PROGRESS
		OMX_SendCommand (pHandle, OMX_CommandPortEnable, OMX_VSWMOSAIC_INPUT_PORT_START_INDEX + i, NULL);
		TRACE_PROGRESS
		semp_pend (vswmosaicILComp->port_sem);
	}
	TRACE_PROGRESS
	OMX_SendCommand (pHandle, OMX_CommandPortEnable, OMX_VSWMOSAIC_OUTPUT_PORT_START_INDEX, NULL);
	TRACE_PROGRESS
	semp_pend (vswmosaicILComp->port_sem);
	TRACE_PROGRESS
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

OMX_ERRORTYPE IL_ClientSinkUseInitialOutputResources (IL_CLIENT_COMP_PRIVATE *thisComp)
{
	OMX_ERRORTYPE err = OMX_ErrorNone;
	unsigned int i = 0;
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	DBG_LOG(DBGLVL_SETUP, (" outbuffers=%d", thisComp->outPortParams->nBufferCountActual));
	for (i = 0; i < thisComp->outPortParams->nBufferCountActual; i++)	{
		/* Pass the output buffer to the component */
		TRACE_PROGRESS
		err = OMX_FillThisBuffer (thisComp->handle, thisComp->outPortParams->pOutBuff[i]);
	}
	DBG_LOG(DBGLVL_TRACE, ("Leave"));

  return err;
}

/*
** Used for decode only chain
*/
void IL_ClientSinkTask (void *threadsArg)
{
	//unsigned int dataRead = 0;
	OMX_ERRORTYPE err = OMX_ErrorNone;
	IL_CLIENT_COMP_PRIVATE *pILComp = (IL_CLIENT_COMP_PRIVATE *)threadsArg;
	OMX_BUFFERHEADERTYPE *pBufferOut = NULL;

	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	/* use the initial i/p buffers and make empty this buffer calls */
	err = IL_ClientSinkUseInitialOutputResources(pILComp);

	while (1){
		/* Read filled buffer pointer from the pipe */
		read (pILComp->outPortParams->opBufPipe[0],	&pBufferOut, sizeof (pBufferOut));
		// Ignore data for now
		// fwrite (pBufferOut->pBuffer, sizeof (char), pBufferOut->nFilledLen, ((IL_Client *) threadsArg)->fOut);

		if(pILComp->nUiCmd == STRM_CMD_STOP){
			pILComp->frameCounter = 0;
			//semp_post(pILComp->eos);
			break;
		}

		/* Pass the input buffer to the component */
		err = OMX_FillThisBuffer (pILComp->handle, pBufferOut);
		pILComp->frameCounter++;
		ShowStat(pILComp);
		if (OMX_ErrorNone != err) {
			/* put back the frame in pipe and wait for state change */
			write (pILComp->outPortParams->opBufPipe[1], &pBufferOut, sizeof (pBufferOut));
			printf (" waiting for action from IL Client \n");
			DBG_LOG(DBGLVL_TRACE, ("Error %d. Exiting", err));
			break;
		}
	}
	pthread_exit(pILComp);
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
}

void ShowStat(IL_CLIENT_COMP_PRIVATE *thisComp)
{
	if(gDbgLevel >= DBGLVL_STAT) {
		char szClck[256];
		char szPts[256];
		char szMclk[256];

		CLOCK_T clk = ClockGetInternalTime(thisComp->pClk);
		if(clk - thisComp->statPrevTime >= TIME_SECOND) {
			Clock2HMSF(clk, szClck, 255);
			Clock2HMSF(thisComp->crnt_pts, szPts, 255);
			CLOCK_T mclk = ClockGetTime(thisComp->pClk);
			Clock2HMSF(mclk, szMclk, 255);
			DBG_PRINT("<%s:%s:Strm=%d frames=%d rate=%0.2f mclk=%s pts=%s bufffullness=%d>\n", thisComp->comp_name, szClck, thisComp->nSessionId, thisComp->frameCounter, 1.0 * (thisComp->frameCounter - thisComp->prevframeCounter) / ((clk - thisComp->statPrevTime) / TIME_SECOND), szMclk, szPts);
			thisComp->prevframeCounter = thisComp->frameCounter;
			thisComp->statPrevTime = clk;
		}
	}
}

/* ========================================================================== */
/**
* IL_ClientUtilGetSelfBufHeader() : This util function is to get buffer header
*                                   specific to one component, from the buffer
*                                   received from other component  .
*
* @param thisComp   : application component data structure
* @param pBuffer    : OMX buffer pointer
* @param type       : it is to identfy teh port type
* @param portIndex  : port number of the component
* @param pBufferOut : components buffer header correponding to pBuffer
*
*  @return      
*  String conversion of the OMX_ERRORTYPE
*
*/
/* ========================================================================== */

OMX_ERRORTYPE IL_ClientUtilGetSelfBufHeader (IL_CLIENT_COMP_PRIVATE *thisComp,
                                             OMX_U8 *pBuffer,
                                             ILCLIENT_PORT_TYPE type,
                                             OMX_U32 portIndex,
                                             OMX_BUFFERHEADERTYPE **pBufferOut)
{
	int i;
	IL_CLIENT_INPORT_PARAMS *inPortParamsPtr;
	IL_CLIENT_OUTPORT_PARAMS *outPortParamsPtr;
	OMX_ERRORTYPE eError = OMX_ErrorNone;

	/* Check for input port buffer header queue */
	if (type == ILCLIENT_INPUT_PORT)
	{
		DBG_LOG(DBGLVL_FRAME, ("%s: port=%d buffers=%d",thisComp->comp_name, portIndex, inPortParamsPtr->nBufferCountActual));
		inPortParamsPtr = thisComp->inPortParams + portIndex;
		for (i = 0; i < inPortParamsPtr->nBufferCountActual; i++)
		{
			if (pBuffer == inPortParamsPtr->pInBuff[i]->pBuffer)
			{
				*pBufferOut = inPortParamsPtr->pInBuff[i];
			}
		}
	}
  /* Check for output port buffer header queue */
	else
	{
		DBG_LOG(DBGLVL_FRAME, ("%s: port=%d buffers=%d",thisComp->comp_name, portIndex, outPortParamsPtr->nBufferCountActual));
		outPortParamsPtr =thisComp->outPortParams + portIndex - thisComp->startOutportIndex;
		for (i = 0; i < outPortParamsPtr->nBufferCountActual; i++)
		{
			if (pBuffer == outPortParamsPtr->pOutBuff[i]->pBuffer)
			{
				*pBufferOut = outPortParamsPtr->pOutBuff[i];
			}
		}
	}

  return (eError);
}

/* ========================================================================== */
/**
* IL_ClientProcessPipeCmdETB() : This function passes the buffers to component
* for consuming. This buffer will come from other component as an output. To 
* consume it, IL client finds its buffer header (for consumer component), and 
* calls ETB call.
* @param thisComp        : Handle to a particular component
* @param pipeMsg         : message structure, which is written in response to 
*                          callbacks
*
*/
/* ========================================================================== */

OMX_ERRORTYPE IL_ClientProcessPipeCmdETB (
	IL_CLIENT_COMP_PRIVATE *thisComp,
	IL_CLIENT_PIPE_MSG *pipeMsg)
{
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_BUFFERHEADERTYPE *pBufferIn = NULL;
	IL_CLIENT_INPORT_PARAMS *inPortParamsPtr;

	DBG_LOG(DBGLVL_FRAME, ("%s:",thisComp->comp_name));
	DBG_LOG(DBGLVL_FRAME, ("%s: port=%d filled=%d",thisComp->comp_name, pipeMsg->bufHeader.nInputPortIndex, pipeMsg->bufHeader.nFilledLen));

	/* search its own buffer header based on submitted by connected comp */
	IL_ClientUtilGetSelfBufHeader (thisComp, pipeMsg->bufHeader.pBuffer,	ILCLIENT_INPUT_PORT,
									pipeMsg->bufHeader.nInputPortIndex,	&pBufferIn);

	DBG_LOG(DBGLVL_FRAME, ("%s: pBufferIn=%p pBufferIn=%p",thisComp->comp_name, pBufferIn));
	/* populate buffer header */
	pBufferIn->nFilledLen = pipeMsg->bufHeader.nFilledLen;
	pBufferIn->nOffset = pipeMsg->bufHeader.nOffset;
	pBufferIn->nTimeStamp = pipeMsg->bufHeader.nTimeStamp;
	pBufferIn->nFlags = 0;
	if(pipeMsg->bufHeader.nFlags & OMX_BUFFERFLAG_EOS) {
		pBufferIn->nFlags = OMX_BUFFERFLAG_EOS;
	}
	//pBufferIn->hMarkTargetComponent = pipeMsg->bufHeader.hMarkTargetComponent;
	//pBufferIn->pMarkData = pipeMsg->bufHeader.pMarkData;
	pBufferIn->nTickCount = 0;

	inPortParamsPtr = thisComp->inPortParams + pBufferIn->nInputPortIndex;

	if(pBufferIn == NULL) {
		DBG_LOG(DBGLVL_ERROR, ("%s: Error:ETB(Filed2) can not find mapped buffer for %p !!!", thisComp->comp_name, pipeMsg->bufHeader.pBuffer));
		err = OMX_ErrorResourcesLost;
		goto Exit;
	}

	if(inPortParamsPtr->fInterlace == OMX_TRUE && !(pBufferIn->nFlags & OMX_BUFFERFLAG_EOS)) {
		OMX_BUFFERHEADERTYPE *pBufferIn2;
		/* search its own buffer header based on submitted by connected comp */
		OMX_S32 nSecondFieldOffset = inPortParamsPtr->nSecondFieldOffset;
		OMX_U8 *pBuffSecondFiled = pipeMsg->bufHeader.pBuffer + nSecondFieldOffset;

		pBufferIn->nFlags |= OMX_TI_BUFFERFLAG_VIDEO_FRAME_TYPE_INTERLACE;


		IL_ClientUtilGetSelfBufHeader (thisComp, pBuffSecondFiled,	ILCLIENT_INPUT_PORT,
										pipeMsg->bufHeader.nInputPortIndex,	&pBufferIn2);
		if(pBufferIn2) {
			pBufferIn2->nTimeStamp = pBufferIn->nTimeStamp;
			pBufferIn2->nOffset = pBufferIn->nOffset;
			pBufferIn2->nFlags = pBufferIn->nFlags  | OMX_TI_BUFFERFLAG_VIDEO_FRAME_TYPE_INTERLACE_BOTTOM;

			pBufferIn2->nFilledLen = pBufferIn->nFilledLen;
			//pBufferIn2->nFilledLen = (((pBufferIn->nFilledLen + pBufferIn->nOffset) * 3) >> 2) - pBufferIn->nOffset;

			//pBufferIn2->hMarkTargetComponent = pipeMsg->bufHeader.hMarkTargetComponent;
			//pBufferIn2->pMarkData = pipeMsg->bufHeader.pMarkData;
		} else {
			DBG_LOG(DBGLVL_ERROR, ("%s: Error:ETB(Filed2) can not find mapped buffer for %p !!!", thisComp->comp_name, pBuffSecondFiled));
		}

		if (1/*pipeMsg->bufHeader.nFlags & OMX_TI_BUFFERFLAG_VIDEO_FRAME_TYPE_INTERLACE_TOP_FIRST*/) 
		{
			DBG_LOG(DBGLVL_FRAME, ("%s: ETB(Filed1)=%d offset=%d filled=%d buf=%p", thisComp->comp_name, inPortParamsPtr->nFramesToM3, pBufferIn->nOffset, pBufferIn->nFilledLen, pipeMsg->bufHeader.pBuffer));
			err = OMX_EmptyThisBuffer (thisComp->handle, pBufferIn);
			inPortParamsPtr->nFramesToM3++;

			DBG_LOG(DBGLVL_FRAME, ("%s: ETB(Filed2)=%d offset=%d filled=%d buf=%p", thisComp->comp_name, inPortParamsPtr->nFramesToM3, pBufferIn2->nOffset, pBufferIn2->nFilledLen, pBuffSecondFiled));
			err = OMX_EmptyThisBuffer (thisComp->handle, pBufferIn2);
			inPortParamsPtr->nFramesToM3++;
		} else {
			DBG_LOG(DBGLVL_FRAME, ("%s: ETB(Filed2)=%d offset=%d filled=%d buf=%p", thisComp->comp_name, inPortParamsPtr->nFramesToM3, pBufferIn2->nOffset, pBufferIn2->nFilledLen, pBuffSecondFiled));
			err = OMX_EmptyThisBuffer (thisComp->handle, pBufferIn2);
			inPortParamsPtr->nFramesToM3++;

			DBG_LOG(DBGLVL_FRAME, ("%s: ETB(Filed1)=%d offset=%d filled=%d buf=%p", thisComp->comp_name, inPortParamsPtr->nFramesToM3, pBufferIn->nOffset, pBufferIn->nFilledLen, pipeMsg->bufHeader.pBuffer));
			err = OMX_EmptyThisBuffer (thisComp->handle, pBufferIn);
			inPortParamsPtr->nFramesToM3++;
		}
#ifdef EN_SCALER_EOS  // Work around for scaler EOS causing hang. This needs to be enabled after finding the fix for the issue.
	} else {
#else
	} else if(!(pBufferIn->nFlags & OMX_BUFFERFLAG_EOS)){
#endif
		DBG_LOG(DBGLVL_FRAME, ("%s: ETB=%d offset=%d filled=%d buf=%p", thisComp->comp_name, inPortParamsPtr->nFramesToM3, pBufferIn->nOffset, pBufferIn->nFilledLen, pipeMsg->bufHeader.pBuffer));
		err = OMX_EmptyThisBuffer (thisComp->handle, pBufferIn);
		inPortParamsPtr->nFramesToM3++;
	} 


Exit:
	return (err);
}

/* ========================================================================== */
/**
* IL_ClientProcessPipeCmdFTB() : This function passes the buffers to component
* for consuming. This buffer will come from other component as consumed at input
* To  consume it, IL client finds its buffer header (for consumer component),
* and calls FTB call.
* @param thisComp        : Handle to a particular component
* @param pipeMsg         : message structure, which is written in response to 
*                          callbacks
*
*/
/* ========================================================================== */

OMX_ERRORTYPE IL_ClientProcessPipeCmdFTB (IL_CLIENT_COMP_PRIVATE *thisComp,
                                          IL_CLIENT_PIPE_MSG *pipeMsg)
{
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_BUFFERHEADERTYPE *pBufferOut = NULL;
	IL_CLIENT_OUTPORT_PARAMS *outPortParamsPtr;

	DBG_LOG(DBGLVL_FRAME, ("%s:",thisComp->comp_name));
	DBG_LOG(DBGLVL_FRAME, ("%s: port=%d",thisComp->comp_name, pipeMsg->bufHeader.nOutputPortIndex));

	if(outPortParamsPtr->fInterlace == OMX_TRUE){
		/* search its own buffer header based on submitted by connected comp */
		IL_ClientUtilGetSelfBufHeader (thisComp, pipeMsg->bufHeader.pBuffer, ILCLIENT_OUTPUT_PORT,
										pipeMsg->bufHeader.nOutputPortIndex, &pBufferOut);
		DBG_LOG(DBGLVL_FRAME, ("%s: port=%d pBufferOut=%p",thisComp->comp_name, pipeMsg->bufHeader.nOutputPortIndex, pBufferOut));
		
		outPortParamsPtr->nFieldCount++;
		if(outPortParamsPtr->nFieldCount == 2) {
			if(pBufferOut == NULL) {
				pBufferOut = outPortParamsPtr->pFirstFieldBuf;
			}
			if(pBufferOut) {
				DBG_LOG(DBGLVL_FRAME, ("%s: FTB=%d.", thisComp->comp_name, outPortParamsPtr->nFramesToM3));
				err = OMX_FillThisBuffer (thisComp->handle, pBufferOut);
				outPortParamsPtr->pFirstFieldBuf = 0;
				outPortParamsPtr->nFieldCount = 0;
				outPortParamsPtr->nFramesToM3++;
			} else {
				// Error
			}
		} else /*if(outPortParamsPtr->nFieldCount == 1)*/ {
			outPortParamsPtr->pFirstFieldBuf = pBufferOut;
		}
	} else {
		/* search its own buffer header based on submitted by connected comp */
		IL_ClientUtilGetSelfBufHeader (thisComp, pipeMsg->bufHeader.pBuffer, ILCLIENT_OUTPUT_PORT,
										pipeMsg->bufHeader.nOutputPortIndex, &pBufferOut);

		DBG_LOG(DBGLVL_FRAME, ("%s: port=%d pBufferOut=%p",thisComp->comp_name, pipeMsg->bufHeader.nOutputPortIndex, pBufferOut));

		outPortParamsPtr = thisComp->outPortParams + (pBufferOut->nOutputPortIndex - thisComp->startOutportIndex);

		/* call etb to the component */
		DBG_LOG(DBGLVL_FRAME, ("%s: FTB=%d.", thisComp->comp_name, outPortParamsPtr->nFramesToM3));

		err = OMX_FillThisBuffer (thisComp->handle, pBufferOut);
		outPortParamsPtr->nFramesToM3++;
	}  
  return (err);
}

/* ========================================================================== */
/**
* IL_ClientCbEmptyBufferDone() : This method is the callback implementation to 
* handle EBD events from the OMX Derived component
*
* @param hComponent        : Handle to the component
* @param ptrAppData        : app pointer, which was passed during the getHandle
* @param pBuffer           : buffer header, for the buffer which is consumed
*
*  @return      
*  OMX_ErrorNone = Successful 
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */

OMX_ERRORTYPE IL_ClientCbEmptyBufferDone (OMX_HANDLETYPE hComponent,
                                          OMX_PTR ptrAppData,
                                          OMX_BUFFERHEADERTYPE *pBuffer)
{
	IL_CLIENT_COMP_PRIVATE *thisComp = (IL_CLIENT_COMP_PRIVATE *) ptrAppData;
	IL_CLIENT_INPORT_PARAMS *inPortParamsPtr;
	IL_CLIENT_PIPE_MSG localPipeMsg;

	OMX_ERRORTYPE eError = OMX_ErrorNone;
	int retVal = 0;

	inPortParamsPtr = thisComp->inPortParams + pBuffer->nInputPortIndex;

	/* if the buffer is from file i/o, write the free buffer header into ipbuf
		pipe, else keep it in its local pipe. From local pipe It would be given to 
		remote component as "consumed buffer " */
	inPortParamsPtr->nFramesFromM3++;

	if(gDbgLevel >= DBGLVL_FRAME){
		DBG_PRINT ("%s:%s%d:%s Frame=%d\n", __FILE__, __FUNCTION__,__LINE__,thisComp->comp_name, inPortParamsPtr->nFramesFromM3);
	}

	pthread_mutex_lock (&thisComp->ebd_mutex);

	if (inPortParamsPtr->connInfo.remotePipe[0] == NULL)  {
		
		/* write the empty buffer pointer to input pipe */
		retVal = write (inPortParamsPtr->ipBufPipe[1], &pBuffer, sizeof (pBuffer));

		if (sizeof (pBuffer) != retVal)	{
			DBG_PRINT ("Error writing into Input buffer i/p Pipe!\n");
			eError = OMX_ErrorNotReady;
			goto Exit;
		}
	}  else  {
		/* Create a message that EBD is done and this buffer is ready to be
		recycled. This message will be read in buffer processing thread and
		remote component will be indicated about its status */
		localPipeMsg.cmd = IL_CLIENT_PIPE_CMD_EBD;
		localPipeMsg.pbufHeader = pBuffer;
		retVal = write (thisComp->localPipe[1],  &localPipeMsg, sizeof (IL_CLIENT_PIPE_MSG));

		if (sizeof (IL_CLIENT_PIPE_MSG) != retVal)    {
			DBG_PRINT ("Error writing into local Pipe!\n");
			eError = OMX_ErrorNotReady;
			goto Exit;
		}
  }

Exit:
  pthread_mutex_unlock (&thisComp->ebd_mutex);
  return eError;
}

/* ========================================================================== */
/**
* IL_ClientCbFillBufferDone() : This method is the callback implementation to 
* handle FBD events from the OMX Derived component
*
* @param hComponent        : Handle to the component
* @param ptrAppData        : app pointer, which was passed during the getHandle
* @param pBuffer           : buffer header, for the buffer which is produced
*
*  @return      
*  OMX_ErrorNone = Successful 
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */

OMX_ERRORTYPE IL_ClientCbFillBufferDone (OMX_HANDLETYPE hComponent,
                                         OMX_PTR ptrAppData,
                                         OMX_BUFFERHEADERTYPE *pBuffer)
{
	IL_CLIENT_COMP_PRIVATE *thisComp = (IL_CLIENT_COMP_PRIVATE *) ptrAppData;
	IL_CLIENT_OUTPORT_PARAMS *outPortParamsPtr;
	IL_CLIENT_PIPE_MSG localPipeMsg;

	OMX_ERRORTYPE eError = OMX_ErrorNone;
	int retVal = 0;

	/* get the pipe corrsponding to this port, portIndex is part of bufferheader
     structure */
	outPortParamsPtr = thisComp->outPortParams + (pBuffer->nOutputPortIndex - thisComp->startOutportIndex);
	outPortParamsPtr->nFramesFromM3++;
  /* if the buffer is from file i/o, write the free buffer header into outbuf
     pipe, else keep it in its local pipe. From local pipe It would be given to 
     remote component as "filled buffer " */

	if(gDbgLevel >= DBGLVL_FRAME) {
		DBG_PRINT ("%s:%s%d:%s Frame=%d\n", __FILE__, __FUNCTION__,__LINE__,thisComp->comp_name, outPortParamsPtr->nFramesFromM3);
	}

	pthread_mutex_lock (&thisComp->fbd_mutex);
	
	if (outPortParamsPtr->connInfo.remotePipe[0] == NULL)  {
		/* write the empty buffer pointer to input pipe */
		retVal = write (outPortParamsPtr->opBufPipe[1], &pBuffer, sizeof (pBuffer));

		if (sizeof (pBuffer) != retVal)	{
			DBG_PRINT ("Error writing to Input buffer i/p Pipe!\n");
			eError = OMX_ErrorNotReady;
			goto Exit;
		}
	} else  {
		/* Create a message that FBD is done and this buffer is ready to be used by 
			other compoenent. This message will be read in buffer processing thread
			and and remote component will be indicated about its status */
		localPipeMsg.cmd = IL_CLIENT_PIPE_CMD_FBD;
		localPipeMsg.pbufHeader = pBuffer;
		retVal = write (thisComp->localPipe[1], &localPipeMsg, sizeof (IL_CLIENT_PIPE_MSG));

		if (sizeof (IL_CLIENT_PIPE_MSG) != retVal)  {
			DBG_PRINT ("Error writing to local Pipe!\n");
			eError = OMX_ErrorNotReady;
			goto Exit;
		}
	}
Exit:
	pthread_mutex_unlock (&thisComp->fbd_mutex);
	return eError;
}


OMX_ERRORTYPE IL_ClientProcessPipeCmdEBD (IL_CLIENT_COMP_PRIVATE *thisComp,
                                          IL_CLIENT_PIPE_MSG *pipeMsg)
{
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_BUFFERHEADERTYPE *pBufferIn;
	IL_CLIENT_PIPE_MSG remotePipeMsg;
	IL_CLIENT_INPORT_PARAMS *inPortParamsPtr;
	int retVal = 0;

	pBufferIn = pipeMsg->pbufHeader;

	/* find the input port structure (pipe) */
	inPortParamsPtr = thisComp->inPortParams + pBufferIn->nInputPortIndex;

	remotePipeMsg.cmd = IL_CLIENT_PIPE_CMD_FTB;
	remotePipeMsg.bufHeader.pBuffer = pBufferIn->pBuffer;
	remotePipeMsg.bufHeader.nOutputPortIndex =	inPortParamsPtr->connInfo.remotePort;

	/* write the fill buffer message to remote pipe */
	retVal = write (inPortParamsPtr->connInfo.remotePipe[1],
					&remotePipeMsg, sizeof (IL_CLIENT_PIPE_MSG));

	if (sizeof (IL_CLIENT_PIPE_MSG) != retVal)	{
		printf ("Error writing to remote Pipe!\n");
		err = OMX_ErrorNotReady;
		return err;
	}

  return (err);
}

/* ========================================================================== */
/**
* IL_ClientProcessPipeCmdFBD() : This function passes the bufefrs to component
* for consuming. This buffer will go to other component to be consumed at input
* port.
* @param thisComp        : Handle to a particular component
* @param pipeMsg         : message structure, which is written in response to 
*                          callbacks
*
*/
/* ========================================================================== */

OMX_ERRORTYPE IL_ClientProcessPipeCmdFBD (IL_CLIENT_COMP_PRIVATE *thisComp,
                                          IL_CLIENT_PIPE_MSG *pipeMsg)
{
	OMX_ERRORTYPE err = OMX_ErrorNone;
	OMX_BUFFERHEADERTYPE *pBufferOut;
	IL_CLIENT_PIPE_MSG remotePipeMsg;
	IL_CLIENT_OUTPORT_PARAMS *outPortParamsPtr;
	int retVal = 0;
	pBufferOut = pipeMsg->pbufHeader;
  
	remotePipeMsg.cmd = IL_CLIENT_PIPE_CMD_ETB;
	remotePipeMsg.bufHeader.pBuffer = pBufferOut->pBuffer;

	outPortParamsPtr = thisComp->outPortParams + (pBufferOut->nOutputPortIndex - thisComp->startOutportIndex);

	/* populate buffer header */
	remotePipeMsg.bufHeader.nFilledLen = pBufferOut->nFilledLen;
	remotePipeMsg.bufHeader.nOffset = pBufferOut->nOffset;
	remotePipeMsg.bufHeader.nTimeStamp = pBufferOut->nTimeStamp;
	remotePipeMsg.bufHeader.nFlags = pBufferOut->nFlags;
	remotePipeMsg.bufHeader.hMarkTargetComponent = pBufferOut->hMarkTargetComponent;
	remotePipeMsg.bufHeader.pMarkData = pBufferOut->pMarkData;
	remotePipeMsg.bufHeader.nTickCount = pBufferOut->nTickCount;
	remotePipeMsg.bufHeader.nInputPortIndex = outPortParamsPtr->connInfo.remotePort;

	/* write the fill buffer message to remote pipe */
	retVal = write (outPortParamsPtr->connInfo.remotePipe[1], &remotePipeMsg, sizeof (IL_CLIENT_PIPE_MSG));

	if (sizeof (IL_CLIENT_PIPE_MSG) != retVal)  {
		DBG_LOG(DBGLVL_ERROR, ("Error writing to remote Pipe!"));
		err = OMX_ErrorNotReady;
	}
	return (err);
}


/* ========================================================================== */
/**
* IL_ClientUseInitialOutputResources() :  This function gives initially all
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

OMX_ERRORTYPE
  IL_ClientUseInitialOutputResources (IL_CLIENT_COMP_PRIVATE *thisComp)

{
	OMX_ERRORTYPE err = OMX_ErrorNone;
	unsigned int i = 0, j;
	IL_CLIENT_OUTPORT_PARAMS *outPortParamPtr = NULL;
	OMX_PARAM_PORTDEFINITIONTYPE param;

	memset (&param, 0, sizeof (param));

	OMX_INIT_PARAM (&param);

	/* Give output buffers to component which is limited by no of output buffers
		available. Rest of the data will be written on the callback from output
		data write thread */
	for (j = 0; j < thisComp->numOutport; j++)	{

		param.nPortIndex = j + thisComp->startOutportIndex;
		OMX_GetParameter (thisComp->handle, OMX_IndexParamPortDefinition, &param);
		outPortParamPtr = thisComp->outPortParams + j;

		if (OMX_TRUE == param.bEnabled)	{
			if (outPortParamPtr->connInfo.remotePipe[0] != 0) {
				for (i = 0; i < thisComp->outPortParams->nBufferCountActual; i++) {
					/* Pass the output buffer to the component */
					outPortParamPtr->nFramesToM3++;
					err = OMX_FillThisBuffer (thisComp->handle, outPortParamPtr->pOutBuff[i]);

				} /* for (i) */
			} /* if (outPortParamPtr->...) */
		} /* if (OMX_TRUE...) */
	} /* for (j) */

  return err;
}

/* ========================================================================== */
/**
* IL_ClientConnInConnOutTask() : This task function is for passing buffers from
* one component to other connected component. This functions reads from local
* pipe of a particular component , and takes action based on the message in the
* pipe. This pipe is written by callback ( EBD/FBD) function from component and
* from other component threads, which writes into this pipe for buffer 
* communication.
*
* @param threadsArg        : Handle to a particular component
*
*/
/* ========================================================================== */

void IL_ClientConnInConnOutTask (void *threadsArg)
{
	IL_CLIENT_PIPE_MSG pipeMsg;
	IL_CLIENT_COMP_PRIVATE *thisComp = (IL_CLIENT_COMP_PRIVATE *) threadsArg;
	OMX_ERRORTYPE err = OMX_ErrorNone;

  /* Initially pipes will not have any buffers, so components needs to be given 
     empty buffers for output ports. Input bufefrs are given by other
     component, or file read task */
	IL_ClientUseInitialOutputResources (thisComp);

	for (;;){
		/* Read from its own local Pipe */
		DBG_LOG(DBGLVL_FRAME, ("%s:wait ", thisComp->comp_name));
		read (thisComp->localPipe[0], &pipeMsg, sizeof (IL_CLIENT_PIPE_MSG));
		DBG_LOG(DBGLVL_FRAME, ("%s:msg=%d ", thisComp->comp_name, pipeMsg.cmd));
		/* check the function type */
		switch (pipeMsg.cmd)
		{
			case IL_CLIENT_PIPE_CMD_EXIT:
				DBG_LOG(DBGLVL_TRACE, ("%s: Thread exiting", thisComp->comp_name));
				pthread_exit (thisComp);
			break;
			case IL_CLIENT_PIPE_CMD_ETB:
				err = IL_ClientProcessPipeCmdETB (thisComp, &pipeMsg);
				/* If not in proper state, bufers may not be accepted by component */
				if (OMX_ErrorNone != err)	{
					// Exit by generating self exit message
					DBG_PRINT ("%s: Error exiting at %s %d (error=0x%x)\n", thisComp->comp_name, __FUNCTION__, __LINE__, err);
					write (thisComp->localPipe[1], &pipeMsg, sizeof (IL_CLIENT_PIPE_MSG));
					DBG_PRINT (" ETB: wait \n");
					pthread_exit (thisComp);
				}
			break;
			case IL_CLIENT_PIPE_CMD_FTB:
				err = IL_ClientProcessPipeCmdFTB (thisComp, &pipeMsg);
  				if (OMX_ErrorNone != err)  {
					DBG_PRINT ("%s: Error exiting at %s %d (error=0x%x)\n", thisComp->comp_name, __FUNCTION__, __LINE__, err);
					write (thisComp->localPipe[1], &pipeMsg, sizeof (IL_CLIENT_PIPE_MSG));
					DBG_PRINT (" FTB: wait \n");
					pthread_exit (thisComp);
				}
			break;
  
			case IL_CLIENT_PIPE_CMD_EBD:
				IL_ClientProcessPipeCmdEBD (thisComp, &pipeMsg);
  			break;

			case IL_CLIENT_PIPE_CMD_FBD:
				IL_ClientProcessPipeCmdFBD (thisComp, &pipeMsg);
			break;

			default:
			break;
		} /* switch () */
	} /* for (;;) */
}

/* ========================================================================== */
/**
* IL_ScalerToDipslayTask() : This task function is for passing buffers from
* scaler to display and drop the frames, if too late. 
* This pipe is written by callback ( EBD/FBD) function from component and
* from other component threads, which writes into this pipe for buffer 
* communication.
*
* @param threadsArg        : Handle to a particular component
*
*/
/* ========================================================================== */

void IL_DisplayTask (void *threadsArg)
{
	IL_CLIENT_PIPE_MSG pipeMsg;
	IL_CLIENT_COMP_PRIVATE *thisComp = (IL_CLIENT_COMP_PRIVATE *) threadsArg;
	OMX_ERRORTYPE err = OMX_ErrorNone;

	/* Initially pipes will not have any buffers, so components needs to be given 
		empty buffers for output ports. Input bufefrs are given by other
		component, or file read task */
	IL_ClientUseInitialOutputResources (thisComp);
	int fDrop = 0;

	for (;;)  {
		/* Read from its own local Pipe */
		read (thisComp->localPipe[0], &pipeMsg, sizeof (IL_CLIENT_PIPE_MSG));

    /* check the function type */

	switch (pipeMsg.cmd)    {

		case IL_CLIENT_PIPE_CMD_EXIT:
			DBG_LOG(DBGLVL_TRACE, ("%s: Thread exiting", thisComp->comp_name));
			pthread_exit (thisComp);
        break;

		case IL_CLIENT_PIPE_CMD_ETB:
			//DBG_PRINT (" DispTask: IL_CLIENT_PIPE_CMD_ETB \n");
			thisComp->crnt_pts = pipeMsg.bufHeader.nTimeStamp;
			if(thisComp->fSync) {
				CLOCK_T nMaxWait = thisComp->frameCounter < 2 ? thisComp->nSyncMaxWaitRunning : thisComp->nSyncMaxWaitStartup;
				DoSync(thisComp, thisComp->crnt_pts, &fDrop, nMaxWait, thisComp->nSyncMaxLateness);
				if(fDrop && thisComp->nDropFraction > 0 && (thisComp->frameCounter + thisComp->frameCounterDrop) % thisComp->nDropFraction == 0) {
					fDrop = 0;
				}
			}
			if(fDrop) {
				char szMsg1[128] = {0};
				char szMsg2[128] = {0};
			
				CLOCK_T clock = ClockGetTime(thisComp->pClk);
				//pipeMsg.cmd = IL_CLIENT_PIPE_CMD_EBD;
				//IL_ClientProcessPipeCmdEBD (thisComp, &pipeMsg);
				DropBuffer(thisComp, &pipeMsg);
				thisComp->frameCounterDrop++;


				Clock2HMSF(clock, szMsg1, 127);
				Clock2HMSF(thisComp->crnt_pts, szMsg2, 127);
				DBG_PRINT("frameCounter=%d Droped=%d clk=%s pts=%s\n", thisComp->frameCounter, thisComp->frameCounterDrop, szMsg1, szMsg2);
			} else {
				//WaitForClock(thisComp->pClk, thisComp->crnt_pts, TIME_SECOND);
				thisComp->frameCounter++;
				err = IL_ClientProcessPipeCmdETB (thisComp, &pipeMsg);
				/* If not in proper state, bufers may not be accepted by component */
				if (OMX_ErrorNone != err)  {
				  write (thisComp->localPipe[1], &pipeMsg, sizeof (IL_CLIENT_PIPE_MSG));
				  DBG_PRINT (" ETB: wait \n");
				  /* since in this example we are changing states in other thread it
					 will return error for giving ETB/FTB calls in non-execute state.
					 Since example is shutting down, we exit the thread */
				  pthread_exit (thisComp);
				  /* if error is incorrect state operation, wait for state to change */
				  /* waiting mechanism should be implemented here */
				}
			}  
			ShowStat(thisComp);
        break;
  
			case IL_CLIENT_PIPE_CMD_EBD:
				//DBG_PRINT (" DispTask: IL_CLIENT_PIPE_CMD_EBD \n");
				IL_ClientProcessPipeCmdEBD (thisComp, &pipeMsg);
			break;

			case IL_CLIENT_PIPE_CMD_FTB:
			case IL_CLIENT_PIPE_CMD_FBD:
				DBG_PRINT (" Error Shoud not come here!! \n");
			break;

			default:
			break;
		} /* switch () */
	} /* for (;;) */
}

/* ========================================================================== */
/**
* IL_ClientConnectComponents() : This util function is to update the pipe
*                                information of other connected comonnet, so that
*                                buffers can be passed to connected component.
*
* @param handleCompPrivA   : application component data structure for producer
* @param compAPortOut      : port of producer comp
* @param handleCompPrivB   : application component data structure for consumer
* @param compBPortIn       : port number of the consumer component
*
*  @return      
*  String conversion of the OMX_ERRORTYPE
*
*/
/* ========================================================================== */

OMX_ERRORTYPE IL_ClientConnectComponents (
	IL_CLIENT_COMP_PRIVATE   *handleCompPrivA,
	unsigned int             compAPortOut,
	IL_CLIENT_COMP_PRIVATE	*handleCompPrivB,
	unsigned int             compBPortIn)
{
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	IL_CLIENT_OUTPORT_PARAMS *outPortParamPtr = NULL;
	IL_CLIENT_INPORT_PARAMS *inPortParamPtr = NULL;

	DBG_LOG(DBGLVL_TRACE, ("Connecting %s:%d %s:%d", handleCompPrivA->comp_name, compAPortOut, handleCompPrivB->comp_name, compBPortIn));
	/* update the input port connect structure */
	outPortParamPtr = handleCompPrivA->outPortParams + compAPortOut - handleCompPrivA->startOutportIndex;
	inPortParamPtr = handleCompPrivB->inPortParams + compBPortIn;

	/* update input port component pipe info with connected port */
	inPortParamPtr->connInfo.remoteClient = handleCompPrivA;
	inPortParamPtr->connInfo.remotePort = compAPortOut;
	inPortParamPtr->connInfo.remotePipe[0] = handleCompPrivA->localPipe[0];
	inPortParamPtr->connInfo.remotePipe[1] = handleCompPrivA->localPipe[1];

	/* update output port component pipe info with connected port */
	outPortParamPtr->connInfo.remoteClient = handleCompPrivB;
	outPortParamPtr->connInfo.remotePort = compBPortIn;
	outPortParamPtr->connInfo.remotePipe[0] = handleCompPrivB->localPipe[0];
	outPortParamPtr->connInfo.remotePipe[1] = handleCompPrivB->localPipe[1];
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return eError;
}

IL_CLIENT_INPORT_PARAMS *compGetConnectedCompInPortParams (
	IL_CLIENT_COMP_PRIVATE	*pComp,
	int                      nOutPort
)
{
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	IL_CLIENT_INPORT_PARAMS *inPortParamPtr = NULL;
	IL_CLIENT_OUTPORT_PARAMS *outPortParamPtr = pComp->outPortParams + nOutPort - pComp->startOutportIndex;
	if(outPortParamPtr->fUsePeerBuffer) {
		IL_CLIENT_COMP_PRIVATE	*handleCompPrivB = outPortParamPtr->connInfo.remoteClient;
		if(handleCompPrivB) {
			unsigned int            compBPortIn = outPortParamPtr->connInfo.remotePort;
			inPortParamPtr = handleCompPrivB->inPortParams + compBPortIn;
			DBG_LOG(DBGLVL_SETUP, ("outPortParamPtr=%p Outport=%d inPortParamPtr=%p compBPortIn=%d", outPortParamPtr, inPortParamPtr, nOutPort - pComp->startOutportIndex, compBPortIn));
		}
	}
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return inPortParamPtr;
}

IL_CLIENT_OUTPORT_PARAMS *compGetConnectedCompOutPortParams(
	IL_CLIENT_COMP_PRIVATE   *pComp,
	int             nInPort)
{
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	DBG_LOG(DBGLVL_TRACE, ("inPortParamPtr Start=%p nInPort=%d", pComp->inPortParams, nInPort));
	IL_CLIENT_INPORT_PARAMS *inPortParamPtr = pComp->inPortParams + nInPort;
	IL_CLIENT_OUTPORT_PARAMS *outPortParamPtr = NULL;

	DBG_LOG(DBGLVL_TRACE, ("inPortParamPtr=%p fUsePeerBuffer=%d", inPortParamPtr, inPortParamPtr->fUsePeerBuffer));
	/* update the input port connect structure */
	if(inPortParamPtr->fUsePeerBuffer)	{
		IL_CLIENT_COMP_PRIVATE   *handleCompPrivA = inPortParamPtr->connInfo.remoteClient;
		if(handleCompPrivA) {
			unsigned int             compAPortOut = inPortParamPtr->connInfo.remotePort;
			outPortParamPtr = handleCompPrivA->outPortParams + compAPortOut - handleCompPrivA->startOutportIndex;
			DBG_LOG(DBGLVL_SETUP, ("inPortParamPtr=%p nInPort=%d outPortParamPtr=%p compAPortOut=%d", inPortParamPtr, nInPort, outPortParamPtr, compAPortOut - handleCompPrivA->startOutportIndex));
		}
	} else {
		DBG_LOG(DBGLVL_TRACE, ("Not using connected comp port params"));
	}
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return outPortParamPtr;
}

/* ========================================================================== */
/**
* IL_ClientCbEventHandler() : This method is the event handler implementation to 
* handle events from the OMX Derived component
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
OMX_ERRORTYPE IL_ClientCbEventHandler (
	OMX_HANDLETYPE     hComponent,
	OMX_PTR            ptrCompCtx,
	OMX_EVENTTYPE      eEvent,
	OMX_U32            nData1, 
	OMX_U32            nData2,
	OMX_PTR            pEventData)
{
	IL_CLIENT_COMP_PRIVATE *comp;

	comp = ptrCompCtx;
	DBG_MSG("event=%d", eEvent);

	if (eEvent == OMX_EventCmdComplete)  {
		if (nData1 == OMX_CommandStateSet)	{
			switch ((int) nData2)
			{
				case OMX_StateInvalid:
				  DBG_MSG ("%s:OMX_StateInvalid", comp->comp_name);
				  break;
				case OMX_StateLoaded:
				  DBG_MSG ("%s:OMX_StateLoaded", comp->comp_name);
				  break;
				case OMX_StateIdle:
				  DBG_MSG ("%s:OMX_StateIdle", comp->comp_name);
				  break;
				case OMX_StateExecuting:
				  DBG_MSG ("%s:OMX_StateExecuting", comp->comp_name);
				  break;
				case OMX_StatePause:
				  DBG_MSG ("%s:OMX_StatePause", comp->comp_name);
				  break;
				case OMX_StateWaitForResources:
				  DBG_MSG ("%s:OMX_StateWaitForResources", comp->comp_name);
				  break;
			}
			/* post an semaphore, so that in IL Client we can confirm the state    change */
			semp_post (comp->done_sem);
		}  
		else if (nData1 == OMX_CommandFlush) {
			DBG_MSG("%s: OMX_CommandFlush completed", comp->comp_name);
			semp_post (comp->done_sem);
		}
		// TODO: Check if this bug is causing the problem
		//else if (OMX_CommandPortEnable || OMX_CommandPortDisable)
		else if (nData1 == OMX_CommandPortEnable || nData1 == OMX_CommandPortDisable)	{
				DBG_MSG("%s: Enable/Disable Event", comp->comp_name);
				semp_post (comp->port_sem);
		}
	}
	else if (eEvent == OMX_EventBufferFlag)	{
		DBG_MSG("OMX_EventBufferFlag");
		if ((int) nData2 == OMX_BUFFERFLAG_EOS)	{
			DBG_MSG("%s: got EOS event", comp->comp_name );
			semp_post(comp->eos);
		}
	}
	else if (eEvent == OMX_EventError)	{
		DBG_MSG("%s: unrecoverable error: %s (0x%lx)!!!", comp->comp_name, IL_ClientErrorToStr (nData1), nData1);
	}
	else  {
		DBG_MSG("%s:unhandled event, param1 = %i, param2 = %i", comp->comp_name, (int) nData1, (int) nData2);
	}

	return OMX_ErrorNone;
}

IL_CLIENT_COMP_PRIVATE *CreateILCompWrapper(
	int numInPort, 
	int numOutPort, 
	int nOutPortIndex, 
	int nInPortBuffCount, 
	int nInPortBuffSize,
	int nOutPortBuffCount, 
	int nOutPortBuffSize)
{
	int i;
	IL_CLIENT_INPORT_PARAMS *inPortParamsPtr;
	IL_CLIENT_OUTPORT_PARAMS *outPortParamsPtr;
	/* alloacte data structure for each component used in this IL Cleint */
	IL_CLIENT_COMP_PRIVATE *pComp = (IL_CLIENT_COMP_PRIVATE *) malloc (sizeof (IL_CLIENT_COMP_PRIVATE));
	memset (pComp, 0x0, sizeof (IL_CLIENT_COMP_PRIVATE));

	/* these semaphores are used for tracking the callbacks received from
		component */
	pComp->eos = malloc (sizeof (semp_t));
	semp_init (pComp->eos, 0);

	pComp->done_sem = malloc (sizeof (semp_t));
	semp_init (pComp->done_sem, 0);

	pComp->port_sem = malloc (sizeof (semp_t));
	semp_init (pComp->port_sem, 0);

	pthread_mutex_init (&pComp->ebd_mutex, NULL);
	pthread_mutex_init (&pComp->fbd_mutex, NULL);

	pComp->numInport = numInPort;
	if(numInPort  > 0) {

		pComp->inPortParams = 	malloc (sizeof (IL_CLIENT_INPORT_PARAMS) * pComp->numInport);
		memset (pComp->inPortParams, 0x0, sizeof (IL_CLIENT_INPORT_PARAMS) * pComp->numInport);

		for (i = 0; i < pComp->numInport; i++) {
			inPortParamsPtr = pComp->inPortParams + i;
			inPortParamsPtr->nBufferCountActual = nInPortBuffCount;
			inPortParamsPtr->nBufferSize = nInPortBuffSize;

			/* this pipe will not be used in this application, as scalar does not read
				/ write into file */
			pipe ((int *) inPortParamsPtr->ipBufPipe);
		}
	}
	pComp->numOutport = numOutPort;
	pComp->startOutportIndex = nOutPortIndex;

	if(numOutPort > 0) {
		pComp->outPortParams = 	malloc (sizeof (IL_CLIENT_OUTPORT_PARAMS) * pComp->numOutport);
		memset (pComp->outPortParams, 0x0, sizeof (IL_CLIENT_OUTPORT_PARAMS) * pComp->numOutport);

		for (i = 0; i < pComp->numOutport; i++) {
			outPortParamsPtr = pComp->outPortParams + i;
			outPortParamsPtr->nBufferCountActual = nOutPortBuffCount;
			outPortParamsPtr->nBufferSize = nOutPortBuffSize;

			pipe ((int *) outPortParamsPtr->opBufPipe);
		}
	}
	/* each componet will have local pipe to take bufffes from other component or 
		its own consumed buffer, so that it can be passed to other conected
		components */
	pipe ((int *) pComp->localPipe);

	return pComp;
}

/*
** Create component with different size for each input port
** Used for SWMOSAIC
*/
IL_CLIENT_COMP_PRIVATE *CreateILMultiInputCompWrapper(
	int numInPort, 
	int numOutPort, 
	int nOutPortIndex, 
	WINDOW_PARAM_T *listInputWindows,
	int nOutPortBuffCount, 
	int nOutPortBuffSize)
{
	int i;
	IL_CLIENT_INPORT_PARAMS *inPortParamsPtr;
	IL_CLIENT_OUTPORT_PARAMS *outPortParamsPtr;
	/* alloacte data structure for each component used in this IL Cleint */
	IL_CLIENT_COMP_PRIVATE *pComp = (IL_CLIENT_COMP_PRIVATE *) malloc (sizeof (IL_CLIENT_COMP_PRIVATE));
	memset (pComp, 0x0, sizeof (IL_CLIENT_COMP_PRIVATE));

	/* these semaphores are used for tracking the callbacks received from
		component */
	pComp->eos = malloc (sizeof (semp_t));
	semp_init (pComp->eos, 0);

	pComp->done_sem = malloc (sizeof (semp_t));
	semp_init (pComp->done_sem, 0);

	pComp->port_sem = malloc (sizeof (semp_t));
	semp_init (pComp->port_sem, 0);

	pthread_mutex_init (&pComp->ebd_mutex, NULL);
	pthread_mutex_init (&pComp->fbd_mutex, NULL);

	pComp->numInport = numInPort;
	if(numInPort  > 0) {
		WINDOW_PARAM_T *pWnd = listInputWindows;
		pComp->inPortParams = 	malloc (sizeof (IL_CLIENT_INPORT_PARAMS) * pComp->numInport);
		memset (pComp->inPortParams, 0x0, sizeof (IL_CLIENT_INPORT_PARAMS) * pComp->numInport);

		for (i = 0; i < pComp->numInport; i++) {
			inPortParamsPtr = pComp->inPortParams + i;
			inPortParamsPtr->nBufferCountActual = pWnd->nBufferCount;
			inPortParamsPtr->nBufferSize = pWnd->nWidth * pWnd->nHeight * 2;

			/* this pipe will not be used in this application, as scalar does not read
				/ write into file */
			pipe ((int *) inPortParamsPtr->ipBufPipe);
			pWnd++;
		}
	}
	pComp->numOutport = numOutPort;
	pComp->startOutportIndex = nOutPortIndex;

	if(numOutPort > 0) {
		pComp->outPortParams = 	malloc (sizeof (IL_CLIENT_OUTPORT_PARAMS) * pComp->numOutport);
		memset (pComp->outPortParams, 0x0, sizeof (IL_CLIENT_OUTPORT_PARAMS) * pComp->numOutport);

		for (i = 0; i < pComp->numOutport; i++) {
			outPortParamsPtr = pComp->outPortParams + i;
			outPortParamsPtr->nBufferCountActual = nOutPortBuffCount;
			outPortParamsPtr->nBufferSize = nOutPortBuffSize;

			pipe ((int *) outPortParamsPtr->opBufPipe);
		}
	}
	/* each componet will have local pipe to take bufffes from other component or 
		its own consumed buffer, so that it can be passed to other conected
		components */
	pipe ((int *) pComp->localPipe);

	return pComp;
}

int compSetOutPortParam(IL_CLIENT_COMP_PRIVATE *pComp, int nOutPortOffset, int nBuffers, int nBufferSize)
{
	DBG_LOG(DBGLVL_SETUP, ("nOutPortOffset=%d nBuffers=%d nBufferSize=%d",nOutPortOffset, nBuffers, nBufferSize));
	IL_CLIENT_OUTPORT_PARAMS *pOoutPortParams = pComp->outPortParams + nOutPortOffset;
	pOoutPortParams->nBufferCountActual = nBuffers;
	pOoutPortParams->nBufferSize = nBufferSize;

}

void DeleteILCompWrapper(IL_CLIENT_COMP_PRIVATE *pComp)
{
	int i;
	IL_CLIENT_INPORT_PARAMS *inPortParamsPtr;
	IL_CLIENT_OUTPORT_PARAMS *outPortParamsPtr;
	close_pipe ((int) pComp->localPipe);

	for (i = 0; i < pComp->numInport; i++)	{
		inPortParamsPtr = pComp->inPortParams + i;
		/* this pipe will not be used in this application, as scalar does not read
			/ write into file */
		close_pipe ((int) inPortParamsPtr->ipBufPipe);
	}

	for (i = 0; i < pComp->numOutport; i++)	{
		outPortParamsPtr = pComp->outPortParams + i;
		/* this pipe will not be used in this application, as scalar does not read
			/ write into file */
		close_pipe ((int) outPortParamsPtr->opBufPipe);
	}

	pthread_mutex_destroy(&pComp->ebd_mutex);
	pthread_mutex_destroy(&pComp->fbd_mutex);
	free (pComp->inPortParams);
	free (pComp->outPortParams);

	semp_deinit (pComp->eos);
	free (pComp->eos);

	semp_deinit (pComp->done_sem);
	free (pComp->done_sem);

	semp_deinit (pComp->port_sem);
	free (pComp->port_sem);
	free (pComp);
}

int compSetStateExec(IL_CLIENT_COMP_PRIVATE_T *pComp, OMX_HANDLETYPE ctrlHandle)
{
	DBG_LOG (DBGLVL_TRACE, ("Enter %s"));

	OMX_ERRORTYPE eError = OMX_ErrorNone;
	DBG_LOG(DBGLVL_SETUP, ("component %s", pComp->comp_name));
	if(ctrlHandle) {
		eError =   OMX_SendCommand (ctrlHandle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
		if (eError != OMX_ErrorNone)  {
			DBG_LOG(DBGLVL_ERROR, ("Error in SendCommand()-OMX_StateExecuting State set : %s", IL_ClientErrorToStr (eError)));
			return -1;
		}
		DBG_LOG(DBGLVL_SETUP, ("Ctrl:Execute state : wait..."));

		if (semp_timedpend(pComp->done_sem, 1000) == -1) {
			DBG_LOG(DBGLVL_ERROR, ("Ctrl:Execute state : Failed!"));
			NotifyOmxFatalError(pComp->pParent, -1);
			return -1;
		}
		DBG_LOG(DBGLVL_SETUP, ("Ctrl:Execute state : complete"));
	}
	/* change state to execute so that buffers processing can start */
	eError = OMX_SendCommand (pComp->handle, OMX_CommandStateSet,
                     OMX_StateExecuting, NULL);
	if (eError != OMX_ErrorNone) {
		DBG_LOG(DBGLVL_ERROR, ("Error from SendCommand-Executing State set :%s", IL_ClientErrorToStr (eError)));
		return -1;
	}
	DBG_LOG(DBGLVL_SETUP, ("%s:Execute state : wait...",pComp->comp_name));

	if (semp_timedpend(pComp->done_sem, 1000) == -1) {
		DBG_LOG(DBGLVL_ERROR, ("Execute state : Failed!"));
		NotifyOmxFatalError(pComp->pParent, -1);
		return -1;
	}
	DBG_LOG(DBGLVL_SETUP, ("%s:Execute state : complete",pComp->comp_name));
	DBG_LOG (DBGLVL_TRACE, ("Leave"));
	return 0;
}

int compSetSateIdle(IL_CLIENT_COMP_PRIVATE_T *pComp, OMX_HANDLETYPE ctrlHandle)
{
	DBG_LOG (DBGLVL_TRACE, ("Enter"));

	OMX_ERRORTYPE eError = OMX_ErrorNone;
	DBG_LOG (DBGLVL_SETUP, ("%s: OMX_StateIdle", pComp->comp_name));
	eError =  OMX_SendCommand (pComp->handle, OMX_CommandStateSet,OMX_StateIdle, NULL);
	if (eError != OMX_ErrorNone){
		DBG_LOG(DBGLVL_ERROR, ("Error from SendCommand-Idle State set :%s", IL_ClientErrorToStr (eError)));
		return -1;
	}
	DBG_LOG(DBGLVL_SETUP,("%s:[state idle:Wait", pComp->comp_name));
	semp_pend (pComp->done_sem);
	DBG_LOG(DBGLVL_SETUP,("%s:[state idle:Done]",pComp->comp_name));

	if(ctrlHandle) {
		eError = OMX_SendCommand (ctrlHandle, OMX_CommandStateSet, OMX_StateIdle, NULL);
		if (eError != OMX_ErrorNone) {
			DBG_LOG(DBGLVL_ERROR, ("Error in SendCommand()-OMX_StateIdle State set : %s", IL_ClientErrorToStr (eError)));
			return -1;
		}

		DBG_LOG(DBGLVL_SETUP, ("[control state idle:wait"));
		semp_pend (pComp->done_sem);
		DBG_LOG(DBGLVL_SETUP, ("control state idle:done]"));
	}
	DBG_LOG (DBGLVL_TRACE, ("Leave"));
	return 0;
}

/* ========================================================================== */
/**
* IL_DecClientSetScalarParams() : Function to fill the port definition 
* structures and call the Set_Parameter function on to the scalar
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

OMX_ERRORTYPE ConfigureScaler (
	OMX_HANDLETYPE  pHandle,
	int  nInputWidth,
	int  nInputHeight,
	int  nInputStride,
	int  nAlinedInputBufferSize,
	int  nInputBuffers,
	int  nInputFormat, //OMX_COLOR_FormatYUV420SemiPlanar

	int  nOutputWidth,
	int  nOutputHeight,
	int  nOutputStride,
	int  nAlinedOutputBufferSize,
	int  nOutputBuffers,
	int  nOutputFormat //OMX_COLOR_FormatYUV420SemiPlanar
	)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_PARAM_BUFFER_MEMORYTYPE memTypeCfg;
	OMX_PARAM_PORTDEFINITIONTYPE paramPort;
	OMX_PARAM_VFPC_NUMCHANNELPERHANDLE sNumChPerHandle;
	OMX_CONFIG_ALG_ENABLE algEnable;
	OMX_CONFIG_VIDCHANNEL_RESOLUTION chResolution;

	DBG_LOG(DBGLVL_SETUP, ("nInputWidth=%d nInputHeight=%d nInputStride=%d nAlinedInputBufferSize=%d nInputBuffers=%d, nInputFormat=%d",nInputWidth, nInputHeight, nInputStride, nAlinedInputBufferSize, nInputBuffers, nInputFormat));
	DBG_LOG(DBGLVL_SETUP, ("nOutputWidth=%d nOutputHeight=%d nOutputStride=%d nAlinedOutputBufferSize=%d nOutputBuffers=%d, nOutputFormat=%d",nOutputWidth, nOutputHeight, nOutputStride, nAlinedOutputBufferSize, nOutputBuffers, nOutputFormat));

	OMX_INIT_PARAM (&memTypeCfg);
	memTypeCfg.nPortIndex = OMX_VFPC_INPUT_PORT_START_INDEX;
	memTypeCfg.eBufMemoryType = OMX_BUFFER_MEMORY_DEFAULT;
	eError = OMX_SetParameter (pHandle, OMX_TI_IndexParamBuffMemType,  &memTypeCfg);

	if (eError != OMX_ErrorNone) {
		ERROR ("failed to set memory Type at input port\n");
	}

	/* Setting Memory type at output port to Raw Memory */
	OMX_INIT_PARAM (&memTypeCfg);
	memTypeCfg.nPortIndex = OMX_VFPC_OUTPUT_PORT_START_INDEX;
	memTypeCfg.eBufMemoryType = OMX_BUFFER_MEMORY_DEFAULT;
	eError = OMX_SetParameter (pHandle, OMX_TI_IndexParamBuffMemType, &memTypeCfg);

	if (eError != OMX_ErrorNone) {
		ERROR ("failed to set memory Type at output port\n");
	}

	/* set input height/width and color format */
	OMX_INIT_PARAM (&paramPort);
	paramPort.nPortIndex = OMX_VFPC_INPUT_PORT_START_INDEX;

	OMX_GetParameter (pHandle, OMX_IndexParamPortDefinition, &paramPort);
	paramPort.nPortIndex = OMX_VFPC_INPUT_PORT_START_INDEX;
	paramPort.format.video.nFrameWidth = nInputWidth;
	paramPort.format.video.nFrameHeight = nInputHeight;

	/* Scalar is connceted to H264 decoder, whose stride is different than width*/
	paramPort.format.video.nStride = nInputStride;
	paramPort.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
	paramPort.format.video.eColorFormat = nInputFormat;
	paramPort.nBufferSize = nAlinedInputBufferSize;//(paramPort.format.video.nStride *nInputHeight * 3) >> 1;

	paramPort.nBufferAlignment = 0;
	paramPort.bBuffersContiguous = 0;
	paramPort.nBufferCountActual = nInputBuffers;

	DBG_LOG(DBGLVL_SETUP, ("Input Port Def width=%d, height=%d, buffsize=%d nInputBuffers=%d", paramPort.format.video.nFrameWidth, paramPort.format.video.nFrameHeight, paramPort.nBufferSize, nInputBuffers));
	eError = OMX_SetParameter (pHandle, OMX_IndexParamPortDefinition, &paramPort);
	if(eError != OMX_ErrorNone) {
		ERROR(" Invalid INPUT color formats for Scalar \n");
		return eError;
	}

	/* set output height/width and color format */
	OMX_INIT_PARAM (&paramPort);
	paramPort.nPortIndex = OMX_VFPC_OUTPUT_PORT_START_INDEX;
	OMX_GetParameter (pHandle, OMX_IndexParamPortDefinition,	&paramPort);

	paramPort.nPortIndex = OMX_VFPC_OUTPUT_PORT_START_INDEX;
	paramPort.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
	paramPort.format.video.eColorFormat = nOutputFormat;
	paramPort.nBufferAlignment = 0;
	paramPort.bBuffersContiguous = 0;
	paramPort.nBufferCountActual = nOutputBuffers;

	/*For the case of On-chip HDMI as display device*/
	paramPort.format.video.nFrameWidth = nOutputWidth;
	paramPort.format.video.nFrameHeight = nOutputHeight;
	paramPort.format.video.nStride = nOutputStride;

	paramPort.nBufferSize =	nAlinedOutputBufferSize;//paramPort.format.video.nStride * paramPort.format.video.nFrameHeight;
	DBG_LOG(DBGLVL_SETUP, ("Out Port Def width=%d, height=%d, buffsize=%d nOutputBuffers=%d", paramPort.format.video.nFrameWidth, paramPort.format.video.nFrameHeight, paramPort.nBufferSize, nOutputBuffers));
	eError = OMX_SetParameter (pHandle, OMX_IndexParamPortDefinition, &paramPort);
	if(eError != OMX_ErrorNone)	{
		ERROR(" Invalid OUTPUT color formats for Scalar \n");
		return eError;
	}

	OMX_INIT_PARAM (&sNumChPerHandle);
	sNumChPerHandle.nNumChannelsPerHandle = 1;
	eError = OMX_SetParameter (pHandle,	(OMX_INDEXTYPE) OMX_TI_IndexParamVFPCNumChPerHandle, &sNumChPerHandle);
	if (eError != OMX_ErrorNone)	{
		ERROR ("failed to set num of channels\n");
	}

	/* set VFPC input and output resolution information */
	DBG_MSG ("set input resolution \n");

	OMX_INIT_PARAM (&chResolution);
	chResolution.Frm0Width = nInputWidth;
	chResolution.Frm0Height = nInputHeight;
	chResolution.Frm0Pitch = nInputStride;
	chResolution.Frm1Width = 0;
	chResolution.Frm1Height = 0;
	chResolution.Frm1Pitch = 0;
	chResolution.FrmStartX = CROP_START_X;
	chResolution.FrmStartY = CROP_START_Y;
	chResolution.FrmCropWidth = nInputWidth;
	chResolution.FrmCropHeight = nInputHeight;
	chResolution.eDir = OMX_DirInput;
	chResolution.nChId = 0;

	eError = OMX_SetConfig (pHandle,(OMX_INDEXTYPE) OMX_TI_IndexConfigVidChResolution, &chResolution);
	if (eError != OMX_ErrorNone) {
		ERROR ("failed to set input channel resolution\n");
	}

	DBG_MSG ("set output resolution \n");
	OMX_INIT_PARAM (&chResolution);
	chResolution.Frm1Width = 0;
	chResolution.Frm1Height = 0;
	chResolution.Frm1Pitch = 0;
	chResolution.FrmStartX = 0;
	chResolution.FrmStartY = 0;
	chResolution.FrmCropWidth = 0;
	chResolution.FrmCropHeight = 0;
	chResolution.eDir = OMX_DirOutput;
	chResolution.nChId = 0;

	/* on secondary display, it is scaled to display size */  
	chResolution.Frm0Width  = nOutputWidth;
	chResolution.Frm0Height = nOutputHeight;
	chResolution.Frm0Pitch  =  nOutputStride;  
	eError = OMX_SetConfig (pHandle, (OMX_INDEXTYPE) OMX_TI_IndexConfigVidChResolution,	&chResolution);
	if (eError != OMX_ErrorNone) {
		ERROR ("failed to set output channel resolution\n");
	}

	/* disable algo bypass mode */
	OMX_INIT_PARAM (&algEnable);
	algEnable.nPortIndex = 0;
	algEnable.nChId = 0;
	algEnable.bAlgBypass = 0;

	eError = OMX_SetConfig (pHandle, (OMX_INDEXTYPE) OMX_TI_IndexConfigAlgEnable, &algEnable);
	if (eError != OMX_ErrorNone)
		ERROR ("failed to disable algo by pass mode\n");

	return (eError);
}

OMX_ERRORTYPE ConfigureDeinterlacer(
	OMX_HANDLETYPE  pDeiHandle,
	int             nInWidth,
	int             nInHeight,
	int             nInStride,
	int				nAlinedInputBufferSize,
	int             nInBuffers,
	int             fInFormat,
	int             fInInterlaced,
	int             nOut1Width,
	int             nOut1Height,
	int             nOut1Stride,
	int				nAlinedOutput1BufferSize,
	int             nOut1Buffers,
	int             fOut1Format,
	int             nOut2Width,
	int             nOut2Height,
	int             nOut2Stride,
	int				nAlinedOutput2BufferSize,
	int             nOut2Buffers,
	int             fOut2Format
	)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_PARAM_BUFFER_MEMORYTYPE memTypeCfg;
	OMX_PARAM_PORTDEFINITIONTYPE paramPort;
	OMX_PARAM_VFPC_NUMCHANNELPERHANDLE sNumChPerHandle;
	OMX_CONFIG_ALG_ENABLE algEnable;
	OMX_CONFIG_VIDCHANNEL_RESOLUTION chResolution;

	DBG_LOG(DBGLVL_SETUP, ("nInWidth=%d nInHeight=%d nInStride=%d fInInterlaced=%d nInBuffers=%d, fInFormat=%d",nInWidth, nInHeight, nInStride, fInInterlaced, nInBuffers, fInFormat));
	DBG_LOG(DBGLVL_SETUP, ("nOut1Width=%d nOut1Height=%d nOut1Buffers=%d nOut1Stride=%d nAlinedOutput1BufferSize=%d fOut1Format=%d",nOut1Width, nOut1Height, nOut1Buffers, nOut1Stride, nAlinedOutput1BufferSize, fOut1Format));
	DBG_LOG(DBGLVL_SETUP, ("nOut2Width=%d nOut2Height=%d nOut2Buffers=%d nOut2Stride=%d nAlinedOutput2BufferSize=%d fOut2Format=%d",nOut2Width, nOut2Height, nOut2Buffers, nOut2Stride, nAlinedOutput2BufferSize, fOut2Format));
	OMX_CONFIG_SUBSAMPLING_FACTOR sSubSamplinginfo = {0};
  
	OMX_INIT_PARAM(&sSubSamplinginfo);

	sSubSamplinginfo.nSubSamplingFactor = 1;
	eError = OMX_SetConfig ( pDeiHandle, ( OMX_INDEXTYPE ) ( OMX_TI_IndexConfigSubSamplingFactor ),	&sSubSamplinginfo );

	OMX_INIT_PARAM (&memTypeCfg);
	memTypeCfg.nPortIndex = OMX_VFPC_INPUT_PORT_START_INDEX;
	memTypeCfg.eBufMemoryType = OMX_BUFFER_MEMORY_DEFAULT;
	eError = OMX_SetParameter (pDeiHandle, OMX_TI_IndexParamBuffMemType, &memTypeCfg);

	if (eError != OMX_ErrorNone) {
		ERROR ("failed to set memory Type at input port\n");
	}

	/* Setting Memory type at output port to Raw Memory */
	OMX_INIT_PARAM (&memTypeCfg);
	memTypeCfg.nPortIndex = OMX_VFPC_OUTPUT_PORT_START_INDEX;
	memTypeCfg.eBufMemoryType = OMX_BUFFER_MEMORY_DEFAULT;
	eError = OMX_SetParameter (pDeiHandle, OMX_TI_IndexParamBuffMemType, &memTypeCfg);

	if (eError != OMX_ErrorNone) {
		ERROR ("failed to set memory Type at output port\n");
	}

	OMX_INIT_PARAM (&memTypeCfg);
	memTypeCfg.nPortIndex = OMX_VFPC_OUTPUT_PORT_START_INDEX + 1;
	memTypeCfg.eBufMemoryType = OMX_BUFFER_MEMORY_DEFAULT;
	eError = OMX_SetParameter (pDeiHandle, OMX_TI_IndexParamBuffMemType, &memTypeCfg);

	if (eError != OMX_ErrorNone) {
		ERROR ("failed to set memory Type at output port\n");
	}

	/* set input height/width and color format */
	OMX_INIT_PARAM (&paramPort);
	paramPort.nPortIndex = OMX_VFPC_INPUT_PORT_START_INDEX;

	OMX_GetParameter (pDeiHandle, OMX_IndexParamPortDefinition, &paramPort);
	paramPort.nPortIndex = OMX_VFPC_INPUT_PORT_START_INDEX;
	paramPort.format.video.nFrameWidth = nInWidth;
	paramPort.format.video.nStride = nInStride;
	paramPort.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;

	if (fInInterlaced) {
		paramPort.format.video.nFrameHeight = nInHeight >> 1;
	}  else {
		paramPort.format.video.nFrameHeight = nInHeight;
	}

	if (fInFormat == OMX_COLOR_FormatYCbYCr) {
		paramPort.format.video.eColorFormat = OMX_COLOR_FormatYCbYCr;
		paramPort.nBufferSize = (paramPort.format.video.nStride * nInHeight) >> 1;
	}  else {
		paramPort.format.video.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
		paramPort.nBufferSize =	nAlinedInputBufferSize;
	}

	paramPort.nBufferAlignment = 0;
	paramPort.bBuffersContiguous = 0;
	paramPort.nBufferCountActual = nInBuffers;
	DBG_MSG ("set input port params (width = %d, height = %d) \n", (int) nInWidth, (int)nInHeight);
	OMX_SetParameter (pDeiHandle, OMX_IndexParamPortDefinition, &paramPort);

	/* set output height/width and color format */
	OMX_INIT_PARAM (&paramPort);
	paramPort.nPortIndex = OMX_VFPC_OUTPUT_PORT_START_INDEX;
	OMX_GetParameter (pDeiHandle, OMX_IndexParamPortDefinition,	&paramPort);

	paramPort.nPortIndex = OMX_VFPC_OUTPUT_PORT_START_INDEX;
	/*For the case of On-chip HDMI as display device*/
	paramPort.format.video.nFrameWidth = nOut1Width;
	paramPort.format.video.nFrameHeight = nOut1Height;
	paramPort.format.video.nStride = nOut1Stride;

	paramPort.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
	paramPort.format.video.eColorFormat = fOut1Format;
	paramPort.nBufferAlignment = 0;
	paramPort.nBufferCountActual = nOut1Buffers;
	/* This port is connected to display and provides 422 o/p */
	paramPort.nBufferSize = nAlinedOutput1BufferSize;

	OMX_SetParameter (pDeiHandle, OMX_IndexParamPortDefinition, &paramPort);

	OMX_INIT_PARAM (&paramPort);
	paramPort.nPortIndex = OMX_VFPC_OUTPUT_PORT_START_INDEX + 1;
	OMX_GetParameter (pDeiHandle, OMX_IndexParamPortDefinition, &paramPort);

	paramPort.nPortIndex = OMX_VFPC_OUTPUT_PORT_START_INDEX + 1;
	paramPort.format.video.nFrameWidth = nOut2Width;
	paramPort.format.video.nFrameHeight = nOut2Height;
	paramPort.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
	paramPort.nBufferAlignment = 0;
	paramPort.nBufferCountActual = nOut2Buffers;
	paramPort.format.video.nStride = nOut2Width;

	/* This port is connected to encoder and provides 420 o/p */
	paramPort.nBufferSize = nAlinedOutput2BufferSize;

	OMX_SetParameter (pDeiHandle, OMX_IndexParamPortDefinition, &paramPort);

	OMX_INIT_PARAM (&sNumChPerHandle);
	sNumChPerHandle.nNumChannelsPerHandle = 1;
	eError = OMX_SetParameter (pDeiHandle, (OMX_INDEXTYPE)OMX_TI_IndexParamVFPCNumChPerHandle, &sNumChPerHandle);
	if (eError != OMX_ErrorNone) {
		ERROR ("failed to set num of channels\n");
	}

	OMX_INIT_PARAM (&chResolution);
	chResolution.Frm0Width = nInWidth;
	chResolution.FrmCropWidth = nInWidth;
	if (fInInterlaced) {
		chResolution.Frm0Height = nInHeight >> 1;
		chResolution.FrmCropHeight = nInHeight >> 1;
	} else {
		chResolution.Frm0Height = nInHeight;
		chResolution.FrmCropHeight = nInHeight;
	}
	chResolution.Frm0Pitch = nInStride;

	chResolution.Frm1Width = 0;
	chResolution.Frm1Height = 0;
	chResolution.Frm1Pitch = 0;
	chResolution.FrmStartX = 0;
	chResolution.FrmStartY = 0;


	chResolution.eDir = OMX_DirInput;
	chResolution.nChId = 0;

	eError = OMX_SetConfig (pDeiHandle,	(OMX_INDEXTYPE) OMX_TI_IndexConfigVidChResolution, &chResolution);
	if (eError != OMX_ErrorNone)	{
		ERROR ("failed to set input channel resolution\n");
	}

	DBG_MSG ("set output resolution");
	OMX_INIT_PARAM (&chResolution);


	/* on secondary display, it is scaled to display size */  
	chResolution.Frm0Width = nOut1Width;
	chResolution.Frm0Height = nOut1Height;
	chResolution.Frm0Pitch = nOut1Stride;  

	/* second output to encode */
	chResolution.Frm1Width  = nOut2Width;
	chResolution.Frm1Height = nOut2Height;
	chResolution.Frm1Pitch  = nOut2Stride;
	chResolution.FrmStartX  = 0;
	chResolution.FrmStartY  = 0;
	chResolution.FrmCropWidth = 0;
	chResolution.FrmCropHeight = 0;
	chResolution.eDir = OMX_DirOutput;
	chResolution.nChId = 0;

	eError = OMX_SetConfig (pDeiHandle, (OMX_INDEXTYPE) OMX_TI_IndexConfigVidChResolution,  &chResolution);
	if (eError != OMX_ErrorNone) {
		ERROR ("failed to set output channel resolution\n");
	}

	/* disable algo bypass mode */
	OMX_INIT_PARAM (&algEnable);
	algEnable.nPortIndex = 0;
	algEnable.nChId = 0;
	/* capture providing progressive input, alg is bypassed */
	algEnable.bAlgBypass = 1;
	if (fInInterlaced) {
		algEnable.bAlgBypass = 0;
	}

	eError = OMX_SetConfig (pDeiHandle, (OMX_INDEXTYPE) OMX_TI_IndexConfigAlgEnable, &algEnable);
	if (eError != OMX_ErrorNone) {
		ERROR ("failed to disable algo by pass mode\n");
	}

	return (eError);
}

int scalerInit(
	IL_CLIENT_COMP_PRIVATE_T *pComp,
	int                       deinterlace,
	int                       useDeiScalerAlways,
	int  nInputWidth,
	int  nInputHeight,
	int  nInputStride,
	int  nAlinedInputBufferSize,
	int  nInputBuffers,
	int  nInputFormat, //OMX_COLOR_FormatYUV420SemiPlanar

	int  nOutput1Width,
	int  nOutput1Height,
	int  nOutput1Stride,
	int  nAlinedouptu1BufferSize,
	int  nOutput1Buffers,
	int  nOutput1Format, //OMX_COLOR_FormatYUV420SemiPlanar

	int  nOutput2Width,
	int  nOutput2Height,
	int  nOutput2Stride,
	int  nAlinedouptu2BufferSize,
	int  nOutput2Buffers,
	int  nOutput2Format, //OMX_COLOR_FormatYUV420SemiPlanar

	OMX_CALLBACKTYPE          *pCb)
{
	int i;
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_HANDLETYPE            handle = NULL;
	DBG_LOG (DBGLVL_TRACE, ("Enter"));

	/* Create Scalar component, it creatd OMX compponnet for scalar writeback ,
		Int this client we are passing the same callbacks to all the component */

	DBG_LOG (DBGLVL_SETUP, ("%s:deinterlace=%d, useDeiScalerAlways=%d", pComp->comp_name, deinterlace, useDeiScalerAlways));
	DBG_LOG (DBGLVL_SETUP, ("nInputWidth=%d, nInputHeight=%d nInputBuffers=%d nInputStride=%d, nOutput1Format=%d",  nInputWidth, nInputHeight, nInputBuffers, nInputStride, nInputFormat));
	DBG_LOG (DBGLVL_SETUP, ("nOutput1Width=%d, nOutput1Height=%d nOutput1Buffers=%d nOutput1Stride=%d nOutput1Format=%d ",  nOutput1Width, nOutput1Height, nOutput1Buffers, nOutput1Stride, nOutput1Format));
	DBG_LOG (DBGLVL_SETUP, ("nOutput2Width=%d, nOutput2Height=%d nOutput2Buffers=%d nOutput2Stride=%d nOutput2Format=%d",  nOutput2Width, nOutput2Height, nOutput2Buffers, nOutput2Stride, nOutput2Format));

	if(deinterlace || useDeiScalerAlways) {
		strcpy(pComp->comp_name, "OMX.TI.VPSSM3.VFPC.DEIMDUALOUT");
		DBG_LOG (DBGLVL_TRACE, ("Create scaler %s", pComp->comp_name));
  		eError = OMX_GetHandle (&handle, (OMX_STRING) pComp->comp_name, pComp, pCb);
		if(eError == OMX_ErrorNone){
			pComp->handle = handle; 
			eError =  ConfigureDeinterlacer(handle,
							nInputWidth,  nInputHeight, nInputStride, nAlinedInputBufferSize, nInputBuffers, nInputFormat, deinterlace,
							nOutput1Width, nOutput1Height, nOutput1Stride, nAlinedouptu1BufferSize, nOutput1Buffers, nOutput1Format,
							nOutput2Width, nOutput2Height, nOutput2Stride, nAlinedouptu2BufferSize, nOutput2Buffers, nOutput2Format);
		}
	} else {
		strcpy(pComp->comp_name, "OMX.TI.VPSSM3.VFPC.INDTXSCWB");
		DBG_LOG (DBGLVL_TRACE, ("Create scaler %s", pComp->comp_name));
		eError = OMX_GetHandle (&handle, (OMX_STRING) pComp->comp_name, pComp, pCb);
		if(eError == OMX_ErrorNone){
			pComp->handle = handle; 
			eError =  ConfigureScaler(handle,
							nInputWidth,  nInputHeight, nInputStride, nAlinedInputBufferSize, nInputBuffers, nInputFormat,
							nOutput1Width, nOutput1Height, nOutput1Width*2, nAlinedouptu1BufferSize, nOutput1Buffers,nOutput1Format);

		}
	}

	if ((eError != OMX_ErrorNone) || (handle == NULL))	{
		DBG_LOG(DBGLVL_ERROR,("Error in Get Handle function : %s \n",	IL_ClientErrorToStr(eError)));
		goto EXIT;
	}
	

	DBG_LOG (DBGLVL_TRACE, ("Create scaler :Success(%p)", handle));

	DBG_LOG (DBGLVL_TRACE, ("[Enable scaler input port"));
	OMX_SendCommand (handle, OMX_CommandPortEnable,  OMX_VFPC_INPUT_PORT_START_INDEX, NULL);
	semp_pend (pComp->port_sem);
	DBG_LOG (DBGLVL_TRACE, ("Enable scaler input port : Success]"));

	for (i = 0; i < pComp->numOutport; i++)	{
		DBG_LOG (DBGLVL_TRACE, ("[Enable scaler output port:%d", OMX_VFPC_OUTPUT_PORT_START_INDEX + i));
		OMX_SendCommand (handle, OMX_CommandPortEnable,  OMX_VFPC_OUTPUT_PORT_START_INDEX + i, NULL);
		semp_pend (pComp->port_sem);
		DBG_LOG (DBGLVL_TRACE, ("Enable scaler ouptput port:%d:Success]", OMX_VFPC_OUTPUT_PORT_START_INDEX + i));
	}
EXIT:
	DBG_LOG (DBGLVL_TRACE, ("Leave"));
	return 0;
}


int scalerAllocateResource(
	IL_CLIENT_COMP_PRIVATE_T *pComp,
	int deinterlace
	)
{
	int i,j;
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	DBG_LOG (DBGLVL_TRACE, ("Enter"));
	DBG_LOG (DBGLVL_SETUP, ("%s:deinterlace=%d numOutport=%d",pComp->comp_name, deinterlace, pComp->numOutport));
	
	DBG_LOG(DBGLVL_SETUP, ("%s: Set idle state (%p)", pComp->comp_name, pComp->handle));
	eError = OMX_SendCommand (pComp->handle, OMX_CommandStateSet,	OMX_StateIdle, NULL);
	if (eError != OMX_ErrorNone)  {
		DBG_PRINT ("Error in SendCommand()-OMX_StateIdle State set : %s \n", IL_ClientErrorToStr (eError));
		return -1;
	}
	IL_CLIENT_OUTPORT_PARAMS *pInportPeerParam = compGetConnectedCompOutPortParams(pComp, 0);

	if(deinterlace) {
		// OMX_PARAM_PORTDEFINITIONTYPE param;
		// int t1, t2;
		int second_field_offset;
		// OMX_GetParameter (pAppData->decILComp->handle, OMX_IndexParamPortDefinition, &param);


		for (i=0; i < pInportPeerParam->nBufferCountActual; i++) {
			OMX_BUFFERHEADERTYPE *pBufHdr = pInportPeerParam->pOutBuff[i];
			OMX_U8 *pBuf1 = pBufHdr->pBuffer;
			//t1 = pBufHdr->nOffset / param.format.video.nStride;
			//t2 = t1 + t1 + ((param.format.video.nFrameHeight + 7) & 0xFFFFFFF8);
			//t1 = t2 * param.format.video.nStride;
			// Frame = Luma_Field1, Luma_Field2, Chrama_Field1, Chroma_Field2
			second_field_offset = (pBufHdr->nFilledLen + pBufHdr->nOffset ) / 3;

			int nFieldSize = pInportPeerParam->nBufferSize;
			pComp->inPortParams->nSecondFieldOffset = second_field_offset;

			
			OMX_U8 *pBuf2 = pBuf1 + second_field_offset;
			DBG_LOG (DBGLVL_SETUP, ("Scaler:InputPort:OMX_UseBuffer: i=%d buff=%p size=%d", i * 2, pBuf1, nFieldSize));
			eError = OMX_UseBuffer (pComp->handle,  &pComp->inPortParams->pInBuff[i * 2],   OMX_VFPC_INPUT_PORT_START_INDEX,
							pComp, nFieldSize, pBuf1);
			if (eError != OMX_ErrorNone)   {
				DBG_LOG(DBGLVL_ERROR, ("Error:Scaler:OMX_UseBuffer:Field1 buffer=%d error=%s", i * 2, IL_ClientErrorToStr (eError)));
				return -1;
			}
			DBG_LOG (DBGLVL_TRACE, ("Scaler:InputPort:OMX_UseBuffer: i=%d buff=%p size=%d", i * 2 + 1, pBuf2, nFieldSize));
			eError = OMX_UseBuffer (pComp->handle, &pComp->inPortParams->pInBuff[i * 2 + 1],   OMX_VFPC_INPUT_PORT_START_INDEX,
							pComp,  nFieldSize, pBuf2);
			if (eError != OMX_ErrorNone)   {
				DBG_LOG(DBGLVL_ERROR, ("Error:Scaler:OMX_UseBuffer:Field2 buffer=%d : error=%s", i * 2 + 1,	IL_ClientErrorToStr (eError)));
				return -1;
			}

		}
	} else {

		for (i = 0; i < pInportPeerParam->nBufferCountActual; i++) {
			DBG_LOG (DBGLVL_SETUP, ("Scaler:InputPort:OMX_UseBuffer:i=%d size=%d", i, pInportPeerParam->nBufferSize));
			eError = OMX_UseBuffer (pComp->handle,  &pComp->inPortParams->pInBuff[i],   OMX_VFPC_INPUT_PORT_START_INDEX,
								pComp, pInportPeerParam->nBufferSize, pInportPeerParam->pOutBuff[i]->pBuffer);
			if (eError != OMX_ErrorNone)   {
				DBG_LOG(DBGLVL_ERROR, ("Error:Scaler:InputtPort:OMX_UseBuffer(): buffer=%d : %s", i, IL_ClientErrorToStr (eError)));
				return -1;
			}
		}
	}

	// Allocate or Use output buffers
	{
		for (j=0; j < pComp->numOutport; j++) {
			IL_CLIENT_OUTPORT_PARAMS *outPortParamsPtr = pComp->outPortParams + j;

			IL_CLIENT_INPORT_PARAMS  *peerInPortParamsPtr = compGetConnectedCompInPortParams(pComp, pComp->startOutportIndex + j);
			if(peerInPortParamsPtr) {
				for (i = 0; i < peerInPortParamsPtr->nBufferCountActual; i++)	{
					DBG_LOG(DBGLVL_SETUP, ("UseBuffer: Outport=%d buffer=%d size=%d bufferp=%p", pComp->startOutportIndex + j, i, peerInPortParamsPtr->nBufferSize, peerInPortParamsPtr->pInBuff[i]->pBuffer));
					eError = OMX_UseBuffer (pComp->handle,&outPortParamsPtr->pOutBuff[i],  pComp->startOutportIndex + j, pComp,	peerInPortParamsPtr->nBufferSize, peerInPortParamsPtr->pInBuff[i]->pBuffer);
					if (eError != OMX_ErrorNone){
						DBG_LOG(DBGLVL_ERROR, ("!!!!! Error OMX_UseBuffer %d- %s !!!!!", i, IL_ClientErrorToStr (eError)));
						return -1;
					}
				}
			} else {
				for (i = 0; i < outPortParamsPtr->nBufferCountActual; i++)  {
					DBG_LOG(DBGLVL_SETUP, ("AllocateBuffer: Outport=%d buffer=%d size=%d", pComp->startOutportIndex + j, i, outPortParamsPtr->nBufferSize));
					eError = OMX_AllocateBuffer (pComp->handle,  &outPortParamsPtr->pOutBuff[i], pComp->startOutportIndex + j, pComp->pParent, outPortParamsPtr->nBufferSize);
					if (eError != OMX_ErrorNone) {
						DBG_LOG(DBGLVL_ERROR, ("Error in OMX_AllocateBuffer()-Output Port State set : %s \n", IL_ClientErrorToStr (eError)));
						return -1;
					}
				}
			}
		}

	}

	DBG_LOG(DBGLVL_ERROR, ("Scaler:Idle state : Wait..."));
	if (semp_timedpend(pComp->done_sem, 1000) == -1) {
		DBG_LOG(DBGLVL_ERROR, ("Scaler:Idle state : Failed!"));
		NotifyOmxFatalError(pComp->pParent, -1);
		return -1;
	}
	DBG_LOG(DBGLVL_SETUP, ("Scaler:Idle state : Complete!"));

	DBG_LOG (DBGLVL_TRACE, ("Leave"));
	return 0;
}

int displayInit(
	int                       displayId,
	void                      *pParent,
	IL_CLIENT_COMP_PRIVATE    *pComp,
	OMX_CALLBACKTYPE          *pCb,
	int                       nWidth,
	int                       nHeight,
	int                       nBufferCount,
    OMX_HANDLETYPE            *pCtrlHandle
	)
{
	DBG_LOG (DBGLVL_TRACE, ("Enter"));
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_HANDLETYPE            handle;
	OMX_HANDLETYPE            ctrlhandle;

	strcpy(pComp->comp_name,  "OMX.TI.VPSSM3.VFDC");
	DBG_LOG (DBGLVL_TRACE, ("Create %s", pComp->comp_name));
	
	eError =  OMX_GetHandle (&handle, pComp->comp_name,  pComp, pCb);
	if (eError != OMX_ErrorNone) {
		ERROR ("failed to get handle\n");
		return -1;
	}
	DBG_LOG (DBGLVL_TRACE, ("Create %: Success", pComp->comp_name));

	pComp->handle = handle;
	pComp->pParent = pParent;

	DBG_LOG (DBGLVL_TRACE, ("Create OMX.TI.VPSSM3.CTRL.DC"));
	eError =   OMX_GetHandle (&ctrlhandle, "OMX.TI.VPSSM3.CTRL.DC",	pComp, pCb);
	if (eError != OMX_ErrorNone) {
		ERROR ("failed to get handle\n");	
		return -1;
	}
	*pCtrlHandle = ctrlhandle;
	DBG_LOG (DBGLVL_TRACE, ("Create OMX.TI.VPSSM3.CTRL.DC: Success"));
	/* omx calls are made in this function for setting the parameters for display 
		component, For clarity purpose it is written as separate function */

	//eError = IL_DecClientSetDisplayParams(pAppData);
	//if ((eError != OMX_ErrorNone))  {    
	//	return -1;
	//}
	ConfigureDisplay(displayId, handle, ctrlhandle, nWidth, nHeight, nBufferCount);

	/* as per openmax specs all the ports should be enabled by default but EZSDK
		OMX component does not enable it hence we manually need to enable it */

	DBG_LOG (DBGLVL_TRACE, ("Display:Enable input port"));
	OMX_SendCommand (handle, OMX_CommandPortEnable, OMX_VFDC_INPUT_PORT_START_INDEX, NULL);

	/* wait for port to get enabled, event handler would be notified from the
		component after enabling the port, which inturn would post this semaphore */
	semp_pend (pComp->port_sem);
	DBG_LOG (DBGLVL_TRACE, ("Display:Enable input port:Success"));
	DBG_LOG (DBGLVL_TRACE, ("Leave"));
	return 0;
}



OMX_ERRORTYPE IL_ClientSetEncodeParams (
	IL_CLIENT_COMP_PRIVATE *encILComp,
	int  nWidth,
	int  nHeight,
	int  nInputBuffers,
	int  nOutBuffers,
	int  nEncodedFrms,
	int  nFrameRate,
	int  nBitRate)
{
	OMX_ERRORTYPE eError = OMX_ErrorUndefined;
	OMX_HANDLETYPE pHandle = NULL;
	OMX_VIDEO_PARAM_PROFILELEVELTYPE tProfileLevel;
	OMX_VIDEO_PARAM_ENCODER_PRESETTYPE tEncoderPreset;
	OMX_VIDEO_PARAM_BITRATETYPE tVidEncBitRate;
	OMX_VIDEO_PARAM_PORTFORMATTYPE tVideoParams;
	OMX_PARAM_PORTDEFINITIONTYPE tPortDef;

	DBG_LOG (DBGLVL_TRACE, ("Enter "));
	DBG_LOG (DBGLVL_SETUP, ("nWidth=%d nHeight=%d nInputBuffers=%d nOutBuffers=%d nFrameRate=%d nBitRate=%d",nWidth, nHeight, nInputBuffers, nOutBuffers, nFrameRate, nBitRate));
	pHandle = encILComp->handle;

	/* Number of frames to be encoded */
	encILComp->numFrames = nEncodedFrms;


	OMX_INIT_PARAM (&tPortDef);
	/* Get the Number of Ports */

	tPortDef.nPortIndex = OMX_VIDENC_INPUT_PORT;
	eError = OMX_GetParameter (pHandle, OMX_IndexParamPortDefinition, &tPortDef);
	/* set the actual number of buffers required */
	tPortDef.nBufferCountActual = nInputBuffers;
	/* set the video format settings */
	tPortDef.format.video.nFrameWidth = nWidth;
	tPortDef.format.video.nStride = nWidth;
	tPortDef.format.video.nFrameHeight = nHeight;
	tPortDef.format.video.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
	/* settings for OMX_IndexParamVideoPortFormat */
	tPortDef.nBufferSize = (nWidth * nHeight * 3) >> 1;
	eError = OMX_SetParameter (pHandle, OMX_IndexParamPortDefinition, &tPortDef);
	if (eError != OMX_ErrorNone) {
		ERROR ("failed to set Encode OMX_IndexParamPortDefinition for input \n");
	}

	OMX_INIT_PARAM (&tPortDef);

	tPortDef.nPortIndex = OMX_VIDENC_OUTPUT_PORT;
	eError = OMX_GetParameter (pHandle, OMX_IndexParamPortDefinition, &tPortDef);
	/* settings for OMX_IndexParamPortDefinition */
	/* set the actual number of buffers required */
	tPortDef.nBufferCountActual = nOutBuffers;
	tPortDef.format.video.nFrameWidth = nWidth;
	tPortDef.format.video.nFrameHeight = nHeight;
	tPortDef.format.video.eCompressionFormat = OMX_VIDEO_CodingAVC;
	tPortDef.format.video.xFramerate = (nFrameRate << 16);
	tVideoParams.xFramerate = (nFrameRate << 16);
	tPortDef.format.video.nBitrate = nBitRate;
	/* settings for OMX_IndexParamVideoPortFormat */

	eError = OMX_SetParameter (pHandle, OMX_IndexParamPortDefinition, &tPortDef);
	if (eError != OMX_ErrorNone) {
		ERROR ("failed to set Encode OMX_IndexParamPortDefinition for output \n");
	}


	/* Set the profile and level for H264 */
	OMX_INIT_PARAM (&tProfileLevel);
	tProfileLevel.nPortIndex = OMX_VIDENC_OUTPUT_PORT;

	eError = OMX_GetParameter (pHandle, OMX_IndexParamVideoProfileLevelCurrent, &tProfileLevel);

	/* set as baseline 4.2 level */

	tProfileLevel.eProfile = OMX_VIDEO_AVCProfileBaseline;
	tProfileLevel.eLevel = OMX_VIDEO_AVCLevel42;
	//tProfileLevel.eLevel = OMX_VIDEO_AVCLevel3;

	eError = OMX_SetParameter (pHandle, OMX_IndexParamVideoProfileLevelCurrent, &tProfileLevel);
	if (eError != OMX_ErrorNone)
		ERROR ("failed to set encoder pfofile \n");

	/* Encoder Preset settings */
	OMX_INIT_PARAM (&tEncoderPreset);
	tEncoderPreset.nPortIndex = OMX_VIDENC_OUTPUT_PORT;
	eError = OMX_GetParameter (pHandle, OMX_TI_IndexParamVideoEncoderPreset, &tEncoderPreset);

	tEncoderPreset.eEncodingModePreset = OMX_Video_Enc_Default;
	tEncoderPreset.eRateControlPreset = OMX_Video_RC_None;

	eError = OMX_SetParameter (pHandle, OMX_TI_IndexParamVideoEncoderPreset, &tEncoderPreset);
	if (eError != OMX_ErrorNone) {
		ERROR ("failed to Encoder Preset \n");
	}

  /* before creating use set_parameters, for run-time change use set_config
     all codec supported parameters can be set using this index       */
	{
		OMX_VIDEO_CONFIG_DYNAMICPARAMS tDynParams;
		OMX_INIT_PARAM (&tDynParams);
		tDynParams.nPortIndex = OMX_VIDENC_OUTPUT_PORT;
		eError = OMX_GetParameter (pHandle, OMX_TI_IndexParamVideoDynamicParams, &tDynParams);
		/* setting I frame interval */
		tDynParams.videoDynamicParams.h264EncDynamicParams.videnc2DynamicParams.intraFrameInterval = 15;//90;
		tDynParams.videoDynamicParams.h264EncDynamicParams.videnc2DynamicParams.refFrameRate = 60 * 1000;
		tDynParams.videoDynamicParams.h264EncDynamicParams.videnc2DynamicParams.targetFrameRate = nFrameRate * 1000;
		eError = OMX_SetParameter (pHandle, OMX_TI_IndexParamVideoDynamicParams, &tDynParams);
	}
	{
		OMX_VIDEO_PARAM_STATICPARAMS tStaticParam;

		OMX_INIT_PARAM (&tStaticParam);
		tStaticParam.nPortIndex = OMX_VIDENC_OUTPUT_PORT;
		eError = OMX_GetParameter (pHandle, OMX_TI_IndexParamVideoStaticParams,	&tStaticParam);
		tStaticParam.videoStaticParams.h264EncStaticParams.videnc2Params.encodingPreset = XDM_USER_DEFINED;

		tStaticParam.videoStaticParams.h264EncStaticParams.numTemporalLayer = IH264_TEMPORAL_LAYERS_1;

		/* for base profile */
		tStaticParam.videoStaticParams.h264EncStaticParams.transformBlockSize = IH264_TRANSFORM_4x4;
		tStaticParam.videoStaticParams.h264EncStaticParams.entropyCodingMode = IH264_ENTROPYCODING_CAVLC;
		/* for base profile end */

		 /* for the mask bits, please refer to codec user guide */
		 tStaticParam.videoStaticParams.h264EncStaticParams.nalUnitControlParams.naluControlPreset = IH264_NALU_CONTROL_USERDEFINED;
		 tStaticParam.videoStaticParams.h264EncStaticParams.nalUnitControlParams.naluPresentMaskStartOfSequence |= 0x2380;//0x2180;
		 tStaticParam.videoStaticParams.h264EncStaticParams.nalUnitControlParams.naluPresentMaskIDRPicture |= 0x23A0; //0x21A0; //0x2180;
		 tStaticParam.videoStaticParams.h264EncStaticParams.nalUnitControlParams.naluPresentMaskIntraPicture |= 0x0202;//0x0002;//0x21A0;//0x2180;
		 tStaticParam.videoStaticParams.h264EncStaticParams.nalUnitControlParams.naluPresentMaskNonIntraPicture |= 0x0202; //0x0002; //0x2180;
		 tStaticParam.videoStaticParams.h264EncStaticParams.nalUnitControlParams.naluPresentMaskEndOfSequence |= 0x2380; //0x2180;

		 tStaticParam.videoStaticParams.h264EncStaticParams.vuiCodingParams.vuiCodingPreset = IH264_VUICODING_USERDEFINED; 
		 tStaticParam.videoStaticParams.h264EncStaticParams.vuiCodingParams.aspectRatioInfoPresentFlag = 0;
		 tStaticParam.videoStaticParams.h264EncStaticParams.vuiCodingParams.aspectRatioIdc = 0;
		 tStaticParam.videoStaticParams.h264EncStaticParams.vuiCodingParams.videoSignalTypePresentFlag = 0;
		 tStaticParam.videoStaticParams.h264EncStaticParams.vuiCodingParams.videoFormat = IH264ENC_VIDEOFORMAT_NTSC;
		 tStaticParam.videoStaticParams.h264EncStaticParams.vuiCodingParams.videoFullRangeFlag = 0;
		 tStaticParam.videoStaticParams.h264EncStaticParams.vuiCodingParams.timingInfoPresentFlag = 0;
		 tStaticParam.videoStaticParams.h264EncStaticParams.vuiCodingParams.hrdParamsPresentFlag = 1;
		 tStaticParam.videoStaticParams.h264EncStaticParams.vuiCodingParams.numUnitsInTicks = 1000;

		 tStaticParam.videoStaticParams.h264EncStaticParams.IDRFrameInterval = 1;

		eError = OMX_SetParameter (pHandle, OMX_TI_IndexParamVideoStaticParams, &tStaticParam);
	}
	/* For changing bit rate following index can be used */
	OMX_INIT_PARAM (&tVidEncBitRate);

	tVidEncBitRate.nPortIndex = OMX_DirOutput;
	eError = OMX_GetParameter (pHandle, OMX_IndexParamVideoBitrate, &tVidEncBitRate);

	tVidEncBitRate.eControlRate = OMX_Video_ControlRateVariable;
	tVidEncBitRate.nTargetBitrate = nBitRate;
	eError = OMX_SetParameter (pHandle, OMX_IndexParamVideoBitrate, &tVidEncBitRate);

	if (eError != OMX_ErrorNone) {
		ERROR ("failed to set Encode bitrate \n");
	}

	return eError;
}


int decoderInit(
		void                      *pParent,
		IL_CLIENT_COMP_PRIVATE    *pComp,
		OMX_CALLBACKTYPE          *pCb
	)
{
	OMX_ERRORTYPE eError = OMX_ErrorUndefined;
	DBG_LOG (DBGLVL_TRACE, ("Enter "));
	strcpy(pComp->comp_name,"OMX.TI.DUCATI.VIDDEC");
	eError = OMX_GetHandle (&pComp->handle, (OMX_STRING) pComp->comp_name, pComp,  pCb);
	DBG_LOG (DBGLVL_TRACE, ("Create Dec:Success"));

	if ((eError != OMX_ErrorNone) || (pComp->handle == NULL)){
		DBG_LOG(DBGLVL_ERROR,("!!! Error in Get Handle function : %s !!!\n",	IL_ClientErrorToStr(eError)));
		goto EXIT;
	} else {
		DBG_LOG(DBGLVL_SETUP,("Create Dec : Success handle=%p", pComp->handle));
	}
	pComp->pParent = pParent;
EXIT:
	DBG_LOG (DBGLVL_TRACE, ("Leave"));
	return 0;
}

int encoderInit(
	void                      *pParent,
	IL_CLIENT_COMP_PRIVATE    *pComp,
	OMX_CALLBACKTYPE          *pCb,
	int  nWidth,
	int  nHeight,
	int  nInputBuffers,
	int  nOutBuffers,
	int  nEncodedFrms,
	int  nFrameRate,
	int  nBitRate
	)
{
	DBG_LOG (DBGLVL_TRACE, ("Enter"));
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_HANDLETYPE            handle;
	int res = 0;
  /* Create the H264 encoder Component, component handle would be returned
     component name is unique and fixed for a componnet, callback are passed
     to componnet in this function. component would be loaded state post this
     call */
	strcpy(pComp->comp_name,  "OMX.TI.DUCATI.VIDENC");
	eError = OMX_GetHandle (&handle, (OMX_STRING)pComp->comp_name,  pComp, pCb);

	DBG_MSG (" encoder component is created \n");

	if ((eError != OMX_ErrorNone) || (handle == NULL)) {
		printf ("Error in Get Handle function : %s \n", IL_ClientErrorToStr (eError));
		res = -1; goto EXIT;
	}

	pComp->handle = handle;
	pComp->pParent = pParent;
	/* Configute the encode componet, ports are default enabled for encode comp,
		so no need to enable from IL Client */
	/* calling OMX_Setparam in this function */
	IL_ClientSetEncodeParams (pComp,nWidth, nHeight, nInputBuffers, nOutBuffers, nEncodedFrms, nFrameRate, nBitRate);

EXIT:
	DBG_LOG (DBGLVL_TRACE, ("Leave"));
	return res;
}


int compSetInportAllocationType(IL_CLIENT_COMP_PRIVATE *pComp, int nInputPort, int fUsePeerBuffer)
{
	IL_CLIENT_INPORT_PARAMS  *inPortParamsPtr = pComp->inPortParams + nInputPort;
	inPortParamsPtr->fUsePeerBuffer = fUsePeerBuffer;
}

int compSetOutPortAllocationType(IL_CLIENT_COMP_PRIVATE *pComp, int nOutputPort, int fUsePeerBuffer)
{
	IL_CLIENT_OUTPORT_PARAMS  *outPortParamsPtr = pComp->outPortParams + nOutputPort;
	outPortParamsPtr->fUsePeerBuffer = fUsePeerBuffer;
}


int compInitResource(IL_CLIENT_COMP_PRIVATE *pComp, OMX_HANDLETYPE ctrlHandle)
{
	int i, j;

	DBG_LOG (DBGLVL_TRACE, ("Enter %s", pComp->comp_name));
	
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	IL_CLIENT_INPORT_PARAMS  *inPortParamsPtr;
	IL_CLIENT_OUTPORT_PARAMS *peerOutPortParamsPtr;
	IL_CLIENT_OUTPORT_PARAMS *outPortParamsPtr;
	IL_CLIENT_INPORT_PARAMS  *peerInPortParamsPtr;
	/* control component does not allocate any data buffers, It's interface is
     though as it is omx componenet */

	DBG_LOG(DBGLVL_TRACE, ("%s: numInport=%d numOutport=%d", pComp->comp_name, pComp->numInport, pComp->numOutport));

	if(ctrlHandle) {
		eError = OMX_SendCommand (ctrlHandle, OMX_CommandStateSet,   OMX_StateIdle, NULL);
		if (eError != OMX_ErrorNone) {
			DBG_LOG(DBGLVL_ERROR, ("Error in SendCommand()-OMX_StateIdle State set : %s", IL_ClientErrorToStr (eError)));
			return -1;
		}
	
		DBG_LOG(DBGLVL_ERROR, ("Ctrl:Idle state : Wait..."));
		if (semp_timedpend(pComp->done_sem, 1000) == -1) {
			DBG_LOG(DBGLVL_ERROR, ("Ctrl:Idle state : Failed!"));
			NotifyOmxFatalError(pComp->pParent, -1);
			return -1;
		}

		DBG_LOG(DBGLVL_SETUP, ("Ctrl:Idle state : Complete!"));
	}
	DBG_LOG(DBGLVL_SETUP, ("%s: Set idle state (%p)", pComp->comp_name, pComp->handle));
	eError = OMX_SendCommand (pComp->handle, OMX_CommandStateSet, OMX_StateIdle, NULL);
	if (eError != OMX_ErrorNone)  {
		DBG_LOG(DBGLVL_ERROR, ("Error in SendCommand()-OMX_StateIdle State set : %s", IL_ClientErrorToStr (eError)));
		return -1;
	}

	for (j=0; j < pComp->numInport; j++) {

		inPortParamsPtr = pComp->inPortParams + j;
		peerOutPortParamsPtr = compGetConnectedCompOutPortParams(pComp, j);
		if(peerOutPortParamsPtr) {
			for (i = 0; i < peerOutPortParamsPtr->nBufferCountActual; i++)	{
				DBG_LOG(DBGLVL_SETUP, ("UseBuffer: Inputport=%d buffer=%d size=%d bufferp=%p", j, i, peerOutPortParamsPtr->nBufferSize, peerOutPortParamsPtr->pOutBuff[i]->pBuffer));
				eError = OMX_UseBuffer (pComp->handle, &inPortParamsPtr->pInBuff[i], j, pComp,	peerOutPortParamsPtr->nBufferSize, peerOutPortParamsPtr->pOutBuff[i]->pBuffer);
				if (eError != OMX_ErrorNone){
					DBG_LOG(DBGLVL_ERROR, ("!!!!! Error OMX_UseBuffer %d- %s !!!!!", i, IL_ClientErrorToStr (eError)));
					goto EXIT;
				}
			}
		} else {
			for (i = 0; i < inPortParamsPtr->nBufferCountActual; i++)  {
				DBG_LOG(DBGLVL_SETUP, ("AllocateBuffer: Inputport=%d buffer=%d size=%d", j, i, inPortParamsPtr->nBufferSize));
				eError = OMX_AllocateBuffer (pComp->handle,  &inPortParamsPtr->pInBuff[i], j, pComp->pParent, inPortParamsPtr->nBufferSize);
				if (eError != OMX_ErrorNone) {
					DBG_LOG(DBGLVL_ERROR, ("Error in OMX_AllocateBuffer()-input Port State set : %s \n", IL_ClientErrorToStr (eError)));
					goto EXIT;
				}
			}
		}
	}
	for (j=0; j < pComp->numOutport; j++) {
		outPortParamsPtr = pComp->outPortParams + j;
		peerInPortParamsPtr = compGetConnectedCompInPortParams(pComp, pComp->startOutportIndex + j);
		if(peerInPortParamsPtr) {
			for (i = 0; i < peerInPortParamsPtr->nBufferCountActual; i++)	{
				DBG_LOG(DBGLVL_SETUP, ("UseBuffer: Outport=%d buffer=%d size=%d bufferp=%p", j, i, peerInPortParamsPtr->nBufferSize, peerInPortParamsPtr->pInBuff[i]->pBuffer));
				eError = OMX_UseBuffer (pComp->handle,&outPortParamsPtr->pOutBuff[i],  pComp->startOutportIndex + j, pComp,	peerInPortParamsPtr->nBufferSize, peerInPortParamsPtr->pInBuff[i]->pBuffer);
				if (eError != OMX_ErrorNone){
					DBG_LOG(DBGLVL_ERROR, ("!!!!! Error OMX_UseBuffer %d- %s !!!!!", i, IL_ClientErrorToStr (eError)));
					goto EXIT;
				}
			}
		} else {
			for (i = 0; i < outPortParamsPtr->nBufferCountActual; i++)  {
				DBG_LOG(DBGLVL_SETUP, ("AllocateBuffer: Outport=%d buffer=%d size=%d", j, i, outPortParamsPtr->nBufferSize));
				eError = OMX_AllocateBuffer (pComp->handle,  &outPortParamsPtr->pOutBuff[i], pComp->startOutportIndex + j, pComp->pParent, outPortParamsPtr->nBufferSize);
				if (eError != OMX_ErrorNone) {
					DBG_LOG(DBGLVL_ERROR, ("Error in OMX_AllocateBuffer()-Output Port State set : %s \n", IL_ClientErrorToStr (eError)));
					goto EXIT;
				}
			}
		}
	}

	DBG_LOG(DBGLVL_ERROR, (" %s: Idle state : Wait...", pComp->comp_name));
	if (semp_timedpend(pComp->done_sem, 1000) == -1) {
		DBG_LOG(DBGLVL_ERROR, ("%s:Idle state : Failed!", pComp->comp_name));
		NotifyOmxFatalError(pComp->pParent, -1);
		return -1;
	}
	DBG_LOG(DBGLVL_SETUP, (" %s Idle state : Complete!", pComp->comp_name));
EXIT:
	DBG_LOG (DBGLVL_TRACE, ("Leave"));
	return 0;
}

int compDeinitResource(IL_CLIENT_COMP_PRIVATE *pComp, OMX_HANDLETYPE ctrlHandle)
{
	int i,j;

	DBG_LOG (DBGLVL_TRACE, ("Enter"));

	OMX_ERRORTYPE eError = OMX_ErrorNone;
	IL_CLIENT_INPORT_PARAMS *inPortParamsPtr;
	IL_CLIENT_OUTPORT_PARAMS *outPortParamsPtr;

	DBG_LOG (DBGLVL_SETUP, (" %s:OMX_CommandStateSet OMX_StateLoaded,",pComp->comp_name));
	eError = OMX_SendCommand (pComp->handle, OMX_CommandStateSet,  OMX_StateLoaded, NULL);
	if (eError != OMX_ErrorNone) {
		DBG_LOG(DBGLVL_ERROR, ("Error from SendCommand-Idle State set :%s \n",	IL_ClientErrorToStr (eError)));
		return -1;
	}
	/* During idle-> loaded state transition buffers need to be freed up */
	for (j=0; j < pComp->numInport; j++) {
		inPortParamsPtr = pComp->inPortParams + j;
		for (i = 0; i < inPortParamsPtr->nBufferCountActual; i++)	{
			DBG_LOG (DBGLVL_TRACE, ("Free port=%d buffer%d (%p)",j, i, inPortParamsPtr->pInBuff[i]));
			eError =  OMX_FreeBuffer (pComp->handle, j,	inPortParamsPtr->pInBuff[i]);
			if (eError != OMX_ErrorNone) {
				DBG_LOG(DBGLVL_ERROR, ("Error in OMX_FreeBuffer : %s", IL_ClientErrorToStr (eError)));
				return -1;
			}
		}
	}
	for (j=0; j < pComp->numOutport; j++) {
		outPortParamsPtr = pComp->outPortParams + j;
		for (i = 0; i < outPortParamsPtr->nBufferCountActual; i++)   {
			DBG_LOG (DBGLVL_TRACE, ("Free port=%d buffer%d (%p)",pComp->startOutportIndex+j, i, outPortParamsPtr->pOutBuff[i]));
			eError =  OMX_FreeBuffer (pComp->handle, pComp->startOutportIndex + j, outPortParamsPtr->pOutBuff[i]);
			if (eError != OMX_ErrorNone) {
				DBG_LOG(DBGLVL_ERROR, ("Error in OMX_FreeBuffer : %s \n", IL_ClientErrorToStr (eError)));
				goto EXIT;
			}
		}
	}
	DBG_LOG(DBGLVL_SETUP, ("%s:[state loaded:wait",pComp->comp_name ));
	semp_pend (pComp->done_sem);
	DBG_LOG(DBGLVL_SETUP, ("%s:state loaded:done]", pComp->comp_name));

	/* control component does not alloc/free any data buffers, It's interface is
		though as it is omx componenet */
	if(ctrlHandle) {
		eError = OMX_SendCommand (ctrlHandle, OMX_CommandStateSet, OMX_StateLoaded, NULL);

		if (eError != OMX_ErrorNone) {
			DBG_LOG(DBGLVL_ERROR, ("Error in SendCommand()-OMX_StateLoaded State set : %s \n",	IL_ClientErrorToStr (eError)));
			return -1;
		}

		DBG_LOG(DBGLVL_TRACE, ("[ctrl state loaded:wait"));
		semp_pend (pComp->done_sem);

		DBG_LOG(DBGLVL_TRACE, (" free ctrl  handle"));
		eError = OMX_FreeHandle (ctrlHandle);
		if ((eError != OMX_ErrorNone))  {
			DBG_LOG(DBGLVL_ERROR, ("Error in Free Ctrl Handle function : %s", IL_ClientErrorToStr (eError)));
			return -1;
		}
		DBG_LOG(DBGLVL_TRACE, ("ctrl state loaded:done]"));
	}


	DBG_LOG(DBGLVL_TRACE, (" free handle"));
	eError = OMX_FreeHandle (pComp->handle);
	if ((eError != OMX_ErrorNone))  {
		DBG_LOG(DBGLVL_ERROR, ("Error in Free Handle function : %s",  IL_ClientErrorToStr (eError)));
		return -1;
	}
EXIT:
	DBG_LOG (DBGLVL_TRACE, ("Leave"));
	return 0;
} 

int semp_timedpend (semp_t *semp, int nTimeOutMsec)
{
	struct timespec timeout;
	struct timeval now;
	int    timeout_us;
	int status = -1;

	gettimeofday (&now, NULL);
	timeout_us = now.tv_usec + 1000 * nTimeOutMsec;
	timeout.tv_sec = now.tv_sec + timeout_us / 1000000;
	timeout.tv_nsec = (timeout_us % 1000000) * 1000;

	pthread_mutex_lock (&semp->mutex);
	while (semp->semcount == 0) 	{
        status = pthread_cond_timedwait (&semp->condition, &semp->mutex, &timeout);
		if (ETIMEDOUT == status){
			return -1;
		}
	}
	semp->semcount--;
	pthread_mutex_unlock (&semp->mutex);
	return 0;
}

int compStartStream(IL_CLIENT_COMP_PRIVATE *pComp, ILC_StartFcnPtr pfnTask)
{
	DBG_LOG(DBGLVL_TRACE,("%s: streaming thread created", pComp->comp_name));
	pthread_attr_init (&pComp->ThreadAttr);

	if (0 != pthread_create (&pComp->connDataStrmThrdId, &pComp->ThreadAttr, pfnTask, pComp))	{
		DBG_LOG(DBGLVL_ERROR, ("Create_Task failed !"));
		goto EXIT;
	}
EXIT:
	return 0;
}

int compStopStream(IL_CLIENT_COMP_PRIVATE *pComp)
{
	DBG_LOG (DBGLVL_TRACE, ("Enter"));
	if(pComp->connDataStrmThrdId) {
		IL_CLIENT_PIPE_MSG pipeMsg;
		void *ret_value;
		pipeMsg.cmd = IL_CLIENT_PIPE_CMD_EXIT;
		write (pComp->localPipe[1], &pipeMsg, sizeof (IL_CLIENT_PIPE_MSG));

		DBG_LOG(DBGLVL_SETUP, ("%s: Waiting for completion data stream thread:begin",pComp->comp_name));
		pthread_join (pComp->connDataStrmThrdId, (void **) &ret_value);
		DBG_LOG(DBGLVL_SETUP, ("%s:Wait for disp-stream-out complete:end", pComp->comp_name));
	}
	DBG_LOG (DBGLVL_TRACE, ("Leave"));
}

void close_pipe(int *pipedscrpt)
{
	close(pipedscrpt[0]);
	close(pipedscrpt[1]);
}