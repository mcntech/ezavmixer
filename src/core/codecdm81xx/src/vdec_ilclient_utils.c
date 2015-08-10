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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <memory.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/fcntl.h>

#include "dbglog.h"
/*-------------------------program files -------------------------------------*/
#define CODEC_H264DEC      // Ram: This definition is required when compiling this component as library

#include <xdc/std.h>
#include "ti/omx/interfaces/openMaxv11/OMX_Core.h"
#include "ti/omx/interfaces/openMaxv11/OMX_Component.h"
#include "OMX_TI_Common.h"
#include "dec_platform_utils.h"

#include <omx_vdec.h>
#include <omx_vfpc.h>
#include <omx_vfdc.h>
#include <omx_ctrl.h>
#include <OMX_TI_Index.h>
#include "OMX_TI_Video.h"

#include "vdec_ilclient.h"
#include "vdec_ilclient_utils.h"
#include "vdec_es_parser.h"
#include "dec_xport.h"
#include "ilclient_common.h"
#include "video_mixer.h"

#define HD_WIDTH       (1920)
#define HD_HEIGHT      (1080)

/* DM8168 PG1.1 SD display takes only 720 as width */

#define SD_WIDTH       (720)

unsigned int CROPPED_WIDTH = 1920;
unsigned int CROPPED_HEIGHT = 1080;


#define CROP_START_X    0
#define CROP_START_Y    0

IL_CLIENT_COMP_PRIVATE *IL_ClientCreateComponent(int nInPorts, int nOutPorts)
{
	IL_CLIENT_COMP_PRIVATE *pILComp;
	/* alloacte data structure for each component used in this IL Cleint */
	pILComp = (IL_CLIENT_COMP_PRIVATE *) malloc (sizeof (IL_CLIENT_COMP_PRIVATE));
	memset (pILComp, 0x0, sizeof (IL_CLIENT_COMP_PRIVATE));

	/* number of ports for each component, which this IL cleint will handle, this 
		will be equal to number of ports supported by component or less */
	pILComp->numInport = nInPorts;
	pILComp->numOutport = nOutPorts;
	pILComp->startOutportIndex = nOutPorts ? nInPorts + 1 : 0;

	if(nInPorts)  {
		pILComp->inPortParams = malloc (sizeof (IL_CLIENT_INPORT_PARAMS) *	pILComp->numInport);
		memset (pILComp->inPortParams, 0x0, sizeof (IL_CLIENT_INPORT_PARAMS));
	}
	if(nOutPorts) {
		pILComp->outPortParams = malloc (sizeof (IL_CLIENT_OUTPORT_PARAMS) * pILComp->numOutport);
		memset (pILComp->outPortParams, 0x0, sizeof (IL_CLIENT_OUTPORT_PARAMS));
	}
	return pILComp;
}

IL_CLIENT_COMP_HOST_T *IL_ClientCreateHostComponent(int nInPorts, int nOutPorts)
{
	IL_CLIENT_COMP_HOST_T *pILComp;
	/* alloacte data structure for each component used in this IL Cleint */
	pILComp = (IL_CLIENT_COMP_HOST_T *) malloc (sizeof (IL_CLIENT_COMP_HOST_T));
	memset (pILComp, 0x0, sizeof (IL_CLIENT_COMP_HOST_T));

	return pILComp;
}

void IL_ClientDeleteComponent(IL_CLIENT_COMP_PRIVATE *pILComp)
{
	if(pILComp->numInport)  {
		free(pILComp->inPortParams);;
	}
	if(pILComp->numOutport) {
		free(pILComp->outPortParams);
	}
	free(pILComp);
}

int IL_ClientConfigInportParams(IL_CLIENT_COMP_PRIVATE *pILComp, int nInBuffers, int nInBufSize)
{
	IL_CLIENT_INPORT_PARAMS *inPortParamsPtr;
	int i;
	for (i = 0; i < pILComp->numInport; i++) {
		inPortParamsPtr = pILComp->inPortParams + i;
		inPortParamsPtr->nBufferCountActual = nInBuffers;
		/* input buffers size for bitstream buffers, It can be smaller than this
			value , setting it for approx value */
		inPortParamsPtr->nBufferSize = nInBufSize;
		/* this pipe is used for taking buffers from read thread */
		pipe ((int *) inPortParamsPtr->ipBufPipe);
	}
	
	/* each componet will have local pipe to take bufffes from other component or 
		its own consumed buffer, so that it can be passed to other conected
		components */
	pipe ((int *) pILComp->localPipe);
}


int IL_ClientConfigOutportParams(IL_CLIENT_COMP_PRIVATE *pILComp, int nOutBuffers, int nOutBufSize)
{
	IL_CLIENT_OUTPORT_PARAMS *outPortParamsPtr;
	int i;
	for (i = 0; i < pILComp->numOutport; i++)	{
		outPortParamsPtr = pILComp->outPortParams + i;
		outPortParamsPtr->nBufferCountActual = nOutBuffers;

		outPortParamsPtr->nBufferSize = nOutBufSize;

		/* This pipe is used if output is directed to write thread */
		pipe ((int *) outPortParamsPtr->opBufPipe);
	}
}

#ifndef EXCLUDE_OMX


/* ========================================================================== */
/**
* IL_ClientDeInit() : This function is to deinitialize the application
*                   data structure.
*
* @param pAppData          : appliaction / client data Handle 
*  @return      
*
*
*/
/* ========================================================================== */

void IL_ClientDeInit (IL_Client *pAppData)
{
	int i;
	IL_CLIENT_INPORT_PARAMS *inPortParamsPtr;
	IL_CLIENT_OUTPORT_PARAMS *outPortParamsPtr;

	if(pAppData->nDestType != DEST_TYPE_NULL) {
		close_pipe ((int) pAppData->scILComp->localPipe);
		for (i = 0; i < pAppData->scILComp->numInport; i++)	{
		inPortParamsPtr = pAppData->scILComp->inPortParams + i;
		/* this pipe is not used in this application, as scalar does not read /
			write into file */
		close_pipe ((int) inPortParamsPtr->ipBufPipe);
		}
		for (i = 0; i < pAppData->scILComp->numOutport; i++) {
			outPortParamsPtr = pAppData->scILComp->outPortParams + i;
			/* this pipe is not used in this application, as scalar does not read /
			write into file */
			close_pipe ((int) outPortParamsPtr->opBufPipe);
		}
	}
	close_pipe ((int) pAppData->decILComp->localPipe);

	for (i = 0; i < pAppData->decILComp->numInport; i++)	{
		inPortParamsPtr = pAppData->decILComp->inPortParams + i;
		close_pipe ((int) inPortParamsPtr->ipBufPipe);
	}
	for (i = 0; i < pAppData->decILComp->numOutport; i++)	{
		outPortParamsPtr = pAppData->decILComp->outPortParams + i;
		/* This pipe is used if output is directed to file write thread, in this
			example, file write is not used */
		close_pipe ((int) outPortParamsPtr->opBufPipe);
	}

	if(pAppData->nDestType != DEST_TYPE_NULL) {
		pthread_mutex_destroy(&pAppData->scILComp->ebd_mutex);
		pthread_mutex_destroy(&pAppData->scILComp->fbd_mutex);
		free (pAppData->scILComp->inPortParams);
		free (pAppData->scILComp->outPortParams);
	}
	pthread_mutex_destroy(&pAppData->decILComp->ebd_mutex);
	pthread_mutex_destroy(&pAppData->decILComp->fbd_mutex);
	free (pAppData->decILComp->inPortParams);
	free (pAppData->decILComp->outPortParams);

	/* these semaphores are used for tracking the callbacks received from
		component */
	if(pAppData->nDestType != DEST_TYPE_NULL) {
		semp_deinit (pAppData->scILComp->eos);
		free (pAppData->scILComp->eos);

		semp_deinit (pAppData->scILComp->done_sem);
		free (pAppData->scILComp->done_sem);

		semp_deinit (pAppData->scILComp->port_sem);
		free (pAppData->scILComp->port_sem);
	}
	semp_deinit (pAppData->decILComp->eos);
	free (pAppData->decILComp->eos);

	semp_deinit (pAppData->decILComp->done_sem);
	free (pAppData->decILComp->done_sem);

	semp_deinit (pAppData->decILComp->port_sem);
	free (pAppData->decILComp->port_sem);

	free (pAppData->decILComp);
	if(pAppData->nDestType != DEST_TYPE_NULL) {
		free (pAppData->scILComp);
	}

	if (pAppData->eCompressionFormat == OMX_VIDEO_CodingAVC)
	{
		if(pAppData->H264Parser.readBuf)
		{
			free(pAppData->H264Parser.readBuf);
		}
	}
	else if (pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG2)
	{
		if(pAppData->pcmpeg2.working_frame)
		{
			free(pAppData->pcmpeg2.working_frame);
		}
		if(pAppData->pcmpeg2.savedbuff)
		{
			free(pAppData->pcmpeg2.savedbuff);
		}
	}

	else if ( pAppData->eCompressionFormat == OMX_VIDEO_CodingMPEG4   )
	{
		if(pAppData->pcmpeg4.working_frame)
		{
			free(pAppData->pcmpeg4.working_frame);
		}
		if(pAppData->pcmpeg4.savedbuff)
		{
			free(pAppData->pcmpeg4.savedbuff);
		}
	}
	if(pAppData->useDemux) {
		IL_ClientDeleteComponent(pAppData->vparseILComp);
	}
}





/* ========================================================================== */
/**
* IL_DecClientSetDecodeParams() : Function to fill the port definition 
* structures and call the Set_Parameter function on to the H264 Decoder
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

OMX_ERRORTYPE IL_DecClientSetDecodeParams (IL_Client *pAppData)
{
	extern OMX_U8 PADX;
	extern OMX_U8 PADY;

	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_HANDLETYPE pHandle = pAppData->decILComp->handle;
	OMX_PORT_PARAM_TYPE portInit;
	OMX_PARAM_PORTDEFINITIONTYPE pInPortDef, pOutPortDef;
	OMX_VIDEO_PARAM_STATICPARAMS tStaticParam;
	OMX_PARAM_COMPPORT_NOTIFYTYPE pNotifyType;
  
	if (!pHandle)	{
		eError = OMX_ErrorBadParameter;
		goto EXIT;
	}

	DBG_LOG (DBGLVL_SETUP, ("PADX: %d PADY: %d\n",PADX, PADY));
	OMX_INIT_PARAM (&portInit);

	portInit.nPorts = 2;
	portInit.nStartPortNumber = 0;
	eError = OMX_SetParameter (pHandle, OMX_IndexParamVideoInit, &portInit);
	if (eError != OMX_ErrorNone)
	{
		goto EXIT;
	}

	/* Set the component's OMX_PARAM_PORTDEFINITIONTYPE structure (input) */

	OMX_INIT_PARAM (&pInPortDef);

	/* populate the input port definataion structure, It is Standard OpenMax
		structure */
	/* set the port index */
	pInPortDef.nPortIndex = OMX_VIDDEC_INPUT_PORT;
	/* It is input port so direction is set as Input, Empty buffers call would be 
		accepted based on this */
	pInPortDef.eDir = OMX_DirInput;
	/* number of buffers are set here */
	pInPortDef.nBufferCountActual = pAppData->nDecInputBufferCount;
	pInPortDef.nBufferCountMin = 1;
	/* buffer size by deafult is assumed as width * height for input bitstream
		which would suffice most of the cases */
	pInPortDef.nBufferSize = pAppData->nDecWidth * pAppData->nDecHeight;


	pInPortDef.bEnabled = OMX_TRUE;
	pInPortDef.bPopulated = OMX_FALSE;

	/* OMX_VIDEO_PORTDEFINITION values for input port */
	/* set the width and height, used for buffer size calculation */
	pInPortDef.format.video.nFrameWidth = pAppData->nDecWidth;
	pInPortDef.format.video.nFrameHeight = pAppData->nDecHeight;
	/* for bitstream buffer stride is not a valid parameter */
	pInPortDef.format.video.nStride = -1;
	/* component supports only frame based processing */

	/* bitrate does not matter for decoder */
	pInPortDef.format.video.nBitrate = 104857600;

	/* as per openmax frame rate is in Q16 format */
	pInPortDef.format.video.xFramerate = (pAppData->nDecFrameRate) << 16;
	/* input port would receive encoded stream */
	pInPortDef.format.video.eCompressionFormat = pAppData->codingType;
	/* this is codec setting, OMX component does not support it */
	/* color format is irrelavant */
	pInPortDef.format.video.eColorFormat = OMX_COLOR_FormatYUV420Planar;

	eError = OMX_SetParameter (pHandle, OMX_IndexParamPortDefinition, &pInPortDef);
	if (eError != OMX_ErrorNone) {
		goto EXIT;
	}

	/* Set the component's OMX_PARAM_PORTDEFINITIONTYPE structure (output) */
	OMX_INIT_PARAM (&pOutPortDef);

	/* setting the port index for output port, properties are set based on this
		index */
	pOutPortDef.nPortIndex = OMX_VIDDEC_OUTPUT_PORT;
	pOutPortDef.eDir = OMX_DirOutput;
	/* componet would expect these numbers of buffers to be allocated */
	pOutPortDef.nBufferCountActual = pAppData->nDecOutputBufferCount;
	pOutPortDef.nBufferCountMin = 1;
	/* codec requires padded height and width and width needs to be aligned at 	128 byte boundary */
	// TODO: Verify both the following calculations are same
	if(pAppData->codingType == OMX_VIDEO_CodingAVC) {
		pOutPortDef.nBufferSize = ((((pAppData->nDecWidth + (2 * PADX) + 127) & 0xFFFFFF80) * ((((pAppData->nDecHeight + 15) & 0xfffffff0) + (4 * PADY))) * 3) >> 1);
	} else {
		pOutPortDef.nBufferSize = (UTIL_ALIGN ((pAppData->nDecWidth + (2 * PADX)), 128) * ((pAppData->nDecHeight + (4 * PADY))) * 3) >> 1;
	}

	pOutPortDef.bEnabled = OMX_TRUE;
	pOutPortDef.bPopulated = OMX_FALSE;
	pInPortDef.eDomain = OMX_PortDomainVideo;
	/* currently component alloactes contigous buffers with 128 alignment, these
		values are do't care */

	/* OMX_VIDEO_PORTDEFINITION values for output port */
	pOutPortDef.format.video.nFrameWidth = pAppData->nDecWidth;
	pOutPortDef.format.video.nFrameHeight = ((pAppData->nDecHeight + 15) & 0xfffffff0);
	/* stride is set as buffer width */
	pOutPortDef.format.video.nStride = UTIL_ALIGN ((pAppData->nDecWidth + (2 * PADX)), 128);
	pOutPortDef.format.video.nSliceHeight = 0;
	pOutPortDef.format.video.bFlagErrorConcealment = OMX_FALSE;

	/* bitrate does not matter for decoder */
	pOutPortDef.format.video.nBitrate = 25000000;
	/* as per openmax frame rate is in Q16 format */
	pOutPortDef.format.video.xFramerate = (pAppData->nDecFrameRate) << 16;
	/* output is raw YUV 420 SP format, It support only this */
	pOutPortDef.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
	pOutPortDef.format.video.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;

	DBG_LOG(DBGLVL_TRACE, ("Output Port Def (width=%d, height=%d, buffsize=%d)", pOutPortDef.format.video.nFrameWidth, pOutPortDef.format.video.nFrameHeight, pOutPortDef.nBufferSize));
	eError = OMX_SetParameter (pHandle, OMX_IndexParamPortDefinition, &pOutPortDef);
	if (eError != OMX_ErrorNone) {
		eError = OMX_ErrorBadParameter;
		goto EXIT;
	}

	eError = OMX_GetParameter (pHandle, OMX_IndexParamPortDefinition, &pOutPortDef);
	if (eError != OMX_ErrorNone) {
		eError = OMX_ErrorBadParameter;
		goto EXIT;
	}
	pAppData->nDecStride = pOutPortDef.format.video.nStride;

	/* Make VDEC execute periodically based on fps */
	OMX_INIT_PARAM(&pNotifyType);
	pNotifyType.eNotifyType = OMX_NOTIFY_TYPE_NONE;
	pNotifyType.nPortIndex =  OMX_VIDDEC_INPUT_PORT;
	eError = OMX_SetParameter (pHandle, OMX_TI_IndexParamCompPortNotifyType, &pNotifyType);
	if (eError != OMX_ErrorNone)	{
		DBG_MSG("input port OMX_SetParameter failed\n");
		eError = OMX_ErrorBadParameter;
		goto EXIT;
	}
	pNotifyType.eNotifyType = OMX_NOTIFY_TYPE_NONE;
	pNotifyType.nPortIndex =  OMX_VIDDEC_OUTPUT_PORT;
	eError = 
		OMX_SetParameter (pHandle, OMX_TI_IndexParamCompPortNotifyType,
						&pNotifyType);
	if (eError != OMX_ErrorNone)	{
		DBG_MSG("output port OMX_SetParameter failed\n");
		eError = OMX_ErrorBadParameter;
		goto EXIT;
	}

	/* Set the codec's static parameters, set it at the end, so above parameter settings
		is not disturbed */
	if (pAppData->codingType == OMX_VIDEO_CodingAVC) {
      
		OMX_INIT_PARAM (&tStaticParam);
   
		tStaticParam.nPortIndex = OMX_VIDDEC_OUTPUT_PORT;
   
		eError = OMX_GetParameter (pHandle, OMX_TI_IndexParamVideoStaticParams,	&tStaticParam);
		/* setting I frame interval */
		DBG_MSG( " level set is %d \n", (int) tStaticParam.videoStaticParams.h264DecStaticParams.presetLevelIdc);
   
		tStaticParam.videoStaticParams.h264DecStaticParams.presetLevelIdc = IH264VDEC_LEVEL42;
                         
		eError = OMX_SetParameter (pHandle, OMX_TI_IndexParamVideoStaticParams,	&tStaticParam);
	}
EXIT:
  return eError;
}
#endif

