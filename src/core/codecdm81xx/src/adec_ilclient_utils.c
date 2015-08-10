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
 *  @file  omx_codec_tunnel_test.c
 *  @brief This file contains h264 codec specific initialization utils
 *
 *  @rev 1.0
 *******************************************************************************
 */
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <xdc/std.h>
#include <OMX_Video.h>
#include "OMX_TI_Audio.h"
#include "OMX_TI_Index.h"
#include "OMX_TI_Common.h"
#include "omx_adec.h"
#include "OMX_Component.h"
#ifdef CODEC_MP3DEC
#include <ti/sdo/codecs/mp3dec/imp3dec.h>
#endif
#include <ti/sdo/codecs/aaclcdec/iaacdec.h>
#include "adec_ilclient_utils.h"
#include <getopt.h>
#include "dbglog.h"

#define OMX_TEST_INIT_STRUCT_PTR(_s_, _name_)       \
 memset((_s_), 0x0, sizeof(_name_)); \
    (_s_)->nSize = sizeof(_name_);              \
    (_s_)->nVersion.s.nVersionMajor = 0x1;      \
    (_s_)->nVersion.s.nVersionMinor = 0x1;      \
    (_s_)->nVersion.s.nRevision  = 0x0;       \
    (_s_)->nVersion.s.nStep   = 0x0;

//void usage (IL_AUD_ARGS *argsp)
//{
//  printf (" ./adec_snt_a8host_debug.xv5T  -i <input filename> -o <output file> -c codec \n"
//          "-i | --input           input filename \n"
//          "-o | --output          output filename. \t(optional. If this field is not given, the audio is played back via serial port - HP out) \n"
//          "-c | --codec           aaclc, mp3 \n"
//          "-r | --rawAac          1 (Raw AAC format), 0 (ADTS/ADIF)\t(required only for aaclc) \n"
//          "-s | --sampleRate      Source SampleRate for RAW AAC format\t(required only for aaclc with raw format)\n");
//  printf(" example : ./adec_snt_a8host_debug.xv5T -i in.aac -o out.pcm -c aaclc -r 1 -s 48000\n ");
//  exit (1);
//}

/* ========================================================================== */
/**
* ADEC_SetParamPortDefinition() : Function to fill the port definition
* structures and call the Set_Parameter function on to the MP3 Decoder
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
OMX_ERRORTYPE ADEC_SetParamPortDefinition(OMX_HANDLETYPE handle, Int32 decType,
                Int aacRawFormat, Int audRawSampleRate,
  				int nInputBufferSize,
				int nOutputBufferSize
)
{
    OMX_ERRORTYPE eError = OMX_ErrorUndefined;
    OMX_HANDLETYPE pHandle = handle;
    OMX_PORT_PARAM_TYPE portInit;
    OMX_PARAM_PORTDEFINITIONTYPE pInPortDef, pOutPortDef;
	OMX_AUDIO_PARAM_PORTFORMATTYPE InPortFormat;
	OMX_AUDIO_PARAM_AACPROFILETYPE aacParams;
    if (!pHandle) {
        eError = OMX_ErrorBadParameter;
        goto EXIT;
    }

    OMX_TEST_INIT_STRUCT_PTR(&portInit, OMX_PORT_PARAM_TYPE);

    portInit.nPorts = 2;
    portInit.nStartPortNumber = 0;
    eError = OMX_SetParameter(pHandle, OMX_IndexParamAudioInit, &portInit);
    if (eError != OMX_ErrorNone) {
        goto EXIT;
    }
    /* Set the component's OMX_PARAM_PORTDEFINITIONTYPE structure (input) */

    OMX_TEST_INIT_STRUCT_PTR(&pInPortDef, OMX_PARAM_PORTDEFINITIONTYPE);

    pInPortDef.nPortIndex = OMX_AUDDEC_INPUT_PORT;
    pInPortDef.eDir = OMX_DirInput;
    pInPortDef.nBufferCountActual = 1;
    pInPortDef.nBufferCountMin = 1;
    pInPortDef.nBufferSize = nInputBufferSize;

    pInPortDef.bEnabled = OMX_TRUE;
    pInPortDef.bPopulated = OMX_FALSE;
    pInPortDef.eDomain = OMX_PortDomainAudio;
    pInPortDef.bBuffersContiguous = OMX_FALSE;
    pInPortDef.nBufferAlignment = 32;

	InPortFormat.nPortIndex = OMX_AUDDEC_INPUT_PORT;

    /* OMX_VIDEO_PORTDEFINITION values for input port */
    pInPortDef.format.audio.cMIMEType = "ADEC";
    pInPortDef.format.audio.pNativeRender = NULL;
#ifdef EN_DDPDEC
	if(decType == 0){
		pInPortDef.format.audio.eEncoding = OMX_AUDIO_CodingMP3;
		InPortFormat.eEncoding = OMX_AUDIO_CodingMP3;
	} else if (decType == 2) {
		pInPortDef.format.audio.eEncoding = OMX_AUDIO_CodingDDP;
		InPortFormat.eEncoding = OMX_AUDIO_CodingDDP;
	} else {
		pInPortDef.format.audio.eEncoding = OMX_AUDIO_CodingAAC;
		InPortFormat.eEncoding = OMX_AUDIO_CodingAAC;
	}
#else
	pInPortDef.format.audio.eEncoding = (decType)?OMX_AUDIO_CodingAAC:OMX_AUDIO_CodingMP3;
#endif
    pInPortDef.format.audio.bFlagErrorConcealment = OMX_FALSE;

    eError =
        OMX_SetParameter(pHandle, OMX_IndexParamPortDefinition, &pInPortDef);
    if (eError != OMX_ErrorNone) {
        goto EXIT;
    }

	DBG_LOG(DBGLVL_SETUP,  ("OMX_IndexParamAudioPortFormat 0x%x", InPortFormat.eEncoding));
    eError =
    OMX_SetParameter(pHandle, OMX_IndexParamAudioPortFormat, &InPortFormat);
    if (eError != OMX_ErrorNone) {
        goto EXIT;
    }

    /* Set the component's OMX_PARAM_PORTDEFINITIONTYPE structure (output) */
    OMX_TEST_INIT_STRUCT_PTR(&pOutPortDef, OMX_PARAM_PORTDEFINITIONTYPE);

    pOutPortDef.nPortIndex = OMX_AUDDEC_OUTPUT_PORT;
    pOutPortDef.eDir = OMX_DirOutput;
    pOutPortDef.nBufferCountActual = 1;
    pOutPortDef.nBufferCountMin = 1;

    /* for referance purpose, in PT mode, stride and height would be used by FQ
       It is padded inside components */
    pOutPortDef.nBufferSize = nOutputBufferSize;
    pOutPortDef.bEnabled = OMX_TRUE;
    pOutPortDef.bPopulated = OMX_FALSE;
    pOutPortDef.eDomain = OMX_PortDomainAudio;
    pOutPortDef.bBuffersContiguous = OMX_FALSE;
    pOutPortDef.nBufferAlignment = 32;

    /* OMX_VIDEO_PORTDEFINITION values for output port */
    pOutPortDef.format.audio.cMIMEType = "PCM";
    pOutPortDef.format.audio.pNativeRender = NULL;
    pOutPortDef.format.audio.bFlagErrorConcealment = OMX_FALSE;
    pOutPortDef.format.audio.eEncoding = OMX_AUDIO_CodingUnused;
        eError =
        OMX_SetParameter(pHandle, OMX_IndexParamPortDefinition, &pOutPortDef);
    if (eError != OMX_ErrorNone) {
        eError = OMX_ErrorBadParameter;
        goto EXIT;
    }

	/* Set AAC specific parameters */
	if(decType == 1) { /* AAC specific */
		DBG_LOG(DBGLVL_SETUP,  ("Setting profile OMX_AUDIO_AACObjectLC"));
		/* Only LC is supported; Hence set this below */
		aacParams.eAACProfile = OMX_AUDIO_AACObjectLC;
		/* Refer OMX_AUDIO_AACSTREAMFORMATTYPE for below: Default is ADTS *
		 * This setting can be set for ADIF streams also.                 *
		 * For RAW, set below to OMX_AUDIO_AACStreamFormatRAW             */
        aacParams.eAACStreamFormat = (aacRawFormat)?OMX_AUDIO_AACStreamFormatRAW:
                                        OMX_AUDIO_AACStreamFormatMP2ADTS;
		/* Sample Rate is mandatory for RAW data format only */
		aacParams.nSampleRate = audRawSampleRate;
		eError = OMX_SetParameter(pHandle, OMX_IndexParamAudioAac, &aacParams);
		if (eError != OMX_ErrorNone) {
			goto EXIT;
		}
	}

  EXIT:
    return eError;
}
/***********************************************************************************/








