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
 *  @file  main.c
 *  @brief This file contains platform (A8) specific initializations and 
 *         the main () of the test application.
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
#include <stdlib.h>
#include <stdio.h>
#include <xdc/std.h>
#include "vdec_ilclient.h"
#include "dbglog.h"
#include "dec_platform_utils.h"

#include <OMX_TI_Index.h>
#include <omx_vfdc.h>
#include <omx_ctrl.h>
#include "minini.h"

#define DISP_SECTION   "display"
extern char             *gIniFile;

void disp_IL_ClientSetSecondaryDisplayParams(
	OMX_HANDLETYPE pDisHandle,
	OMX_HANDLETYPE pctrlHandle)
{
  OMX_PARAM_DC_CUSTOM_MODE_INFO customModeInfo;
  OMX_ERRORTYPE eError = OMX_ErrorNone;
  

    /* For LCD display configuration, custom mode parameters need to be 
       configured */
    OMX_INIT_PARAM (&customModeInfo);
    
	DBG_LOG(DBGLVL_SETUP, ("Initializing defaults from %s", gIniFile));
    customModeInfo.width       =  ini_getl(DISP_SECTION, "width",   DISPLAY_WIDTH, gIniFile);
    customModeInfo.height      =  ini_getl(DISP_SECTION, "height",  DISPLAY_HEIGHT, gIniFile);
    customModeInfo.pixelClock  =  ini_getl(DISP_SECTION, "clock",   LCD_PIXEL_CLOCK, gIniFile);
    customModeInfo.hFrontPorch =  ini_getl(DISP_SECTION, "hfp",     LCD_H_FRONT_PORCH, gIniFile);
    customModeInfo.hBackPorch  =  ini_getl(DISP_SECTION, "hbp",     LCD_H_BACK_PORCH, gIniFile);
    customModeInfo.hSyncLen    =  ini_getl(DISP_SECTION, "hsync",   LCD_H_SYNC_LENGTH, gIniFile);
    customModeInfo.vFrontPorch =  ini_getl(DISP_SECTION, "vfp",     LCD_V_FRONT_PORCH, gIniFile);
    customModeInfo.vBackPorch  =  ini_getl(DISP_SECTION, "vbp",     LCD_V_BACK_PORCH, gIniFile);
    customModeInfo.vSyncLen    =  ini_getl(DISP_SECTION, "vsync",   LCD_V_SYNC_LENGTH, gIniFile);
    customModeInfo.scanFormat  = OMX_SF_PROGRESSIVE;
	
	DBG_MSG("Display : w=%d h=%d clk=%d hfp=%d hbp=%d hsync=%d vfp=%d vbp=%d vsync=%d\n",
					customModeInfo.width,      
					customModeInfo.height,     
					customModeInfo.pixelClock,
					customModeInfo.hFrontPorch,
					customModeInfo.hBackPorch,
					customModeInfo.hSyncLen,
					customModeInfo.vFrontPorch,
					customModeInfo.vBackPorch, 
					customModeInfo.vSyncLen,   
					customModeInfo.scanFormat );

	/*Configure Display component and Display controller with these parameters*/
    eError = OMX_SetParameter (pDisHandle, (OMX_INDEXTYPE)OMX_TI_IndexParamVFDCCustomModeInfo, &customModeInfo);    
    if (eError != OMX_ErrorNone)
      ERROR ("failed to set custom mode setting for Display component\n");

    eError = OMX_SetParameter (pctrlHandle, (OMX_INDEXTYPE)OMX_TI_IndexParamVFDCCustomModeInfo, &customModeInfo);    
    if (eError != OMX_ErrorNone)
      ERROR ("failed to set custom mode setting for Display Controller \
             component\n"); 
}
