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

#ifndef __OMX_VID_ILCLIENT_UTILS_H__
#define __OMX_VID_ILCLIENT_UTILS_H__

/* SD display is through NF in this example, and NF has limitation of multiple 
   of 32 ; Netra PG1.1 SD width has to be 720 ; follwing is done for Scalar and NF*/

#define SD_DISPLAY_WIDTH    ((720 + 31) & 0xffffffe0)
#define SD_DISPLAY_HEIGHT   ((480 + 31) & 0xffffffe0)

// TODO: Change to get from user
#define MAX_FILE_NAME_SIZE      256
#define IL_CLIENT_ENC_HEIGHT      16

OMX_ERRORTYPE IL_DecClientSetDecodeParams (IL_Client *pAppData);
OMX_ERRORTYPE IL_DecClientSetScalarParams (IL_Client *pAppData);
OMX_ERRORTYPE IL_DecClientSetDisplayParams (IL_Client *pAppData);

IL_CLIENT_COMP_PRIVATE *IL_ClientCreateComponent(int nInPorts, int nOutPorts);
IL_CLIENT_COMP_HOST_T *IL_ClientCreateHostComponent(int nInPorts, int nOutPorts);
void IL_ClientDeleteComponent(IL_CLIENT_COMP_PRIVATE *pILComp);
int IL_ClientConfigInportParams(IL_CLIENT_COMP_PRIVATE *pILComp, int nInBuffers, int nInBufSize);

int IL_ClientConfigOutportParams(IL_CLIENT_COMP_PRIVATE *pILComp, int nOutBuffers, int nOutBufSize);

#endif //__OMX_AUD_ILCLIENT_UTILS_H__

/* Nothing beyond this point */
