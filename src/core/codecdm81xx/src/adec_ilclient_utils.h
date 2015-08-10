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
#ifndef __OMX_ADEC_H__
#define __OMX_ADEC_H__

/******************************************************************************\
 *      Includes
\******************************************************************************/

/******************************************************************************/

#ifdef __cplusplus              /* required for headers that might */
extern "C"
{                               /* be compiled by a C++ compiler */
#endif

/* 
 * OMX_AUDIO_CodingDDP is defined in $(EZSDK)/component-sources/omx_05_02_00_48/src/ti/omx/comp/adec
 * as part of Dolby Digital Plus Support.
 * It is redunntly defined here to avoid compilation issues, when building on a system that does not contain
 * the OMX patch for Dolby Digital Plus Support.
 */

#ifndef OMX_AUDIO_CodingDDP
#define OMX_AUDIO_CodingDDP    0x80000003
#endif

#define AUD_DEFAULT_INPUT_PKT_SIZE     (188)
#define AUD_DEFAULT_INPUT_BUF_SIZE     (4 * 1024)
#define AUD_DEFAULT_OUTPUT_BUF_SIZE    (16 * 1024) //(8192)
#define ALSA_DEFAULT_PERIOD_BUFF_SIZE  (8 * 1536)
#endif
