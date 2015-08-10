/*--------------------- system and platform files ----------------------------*/
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "strmcomp.h"
#include "dec_clock.h"
#include "dbglog.h"
#ifdef WIN32
#include <winsock2.h>
#endif
#include "oal.h"

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

#define NUM_OF_IN_BUFFERS 1






//=========================================================
//           G711
//=========================================================





typedef struct AENCFake_Client {

    FILE *fOut;
        FILE *fIn;
	unsigned long nChannels;
	unsigned long bitrate;
	unsigned long samplerate;
	int outputformat;
    OMX_BUFFERHEADERTYPE_M *pInBuff;
    long long InFileSize;
    unsigned long nEncodedFrms;
	unsigned long InputDataReadSize;
	ConnCtxT *pConnSrc;
	ConnCtxT *pConnDest;

	unsigned long fStreaming;
	int     fClkSrc;
	void    *pClk;
	char       device[256];
	int   codec_name[32];
	int   audRawSampleRate;
	int   aacRawFormat;

	int        max_input_pkt_size;	// Max input size. 188 for TS Demux, (8*1536) for SDI AC3
	int        dec_input_buffer_size;
	int        dec_output_buffer_size;
	int        alsa_output_buffer_size;
	int        fEoS;
	unsigned long long crnt_input_pts;
	unsigned long long crnt_out_pts;
	void     *thrdHandle;
} AENCFake_Client;

static int GetInputData(AENCFake_Client *pAppData, unsigned char *pData, int lenData)
{
	int nBytesCopied = 0;
	int nMaxPktSize = pAppData->max_input_pkt_size;


	int ret = 0;
	unsigned long long ullPts = 0;
	unsigned long ulFlags;
	DBG_LOG(DBGLVL_FRAME,  (" Waitfor buffer..."));
	while(pAppData->pConnSrc->IsEmpty(pAppData->pConnSrc) && pAppData->fStreaming){
		OAL_TASK_SLEEP(1)
	}
	DBG_LOG(DBGLVL_FRAME,  (" Waitfor buffer:Done"));
	if(!pAppData->fStreaming) {
		ulFlags = OMX_BUFFERFLAG_EOS;
	} else {
		ret = pAppData->pConnSrc->Read(pAppData->pConnSrc, pData + nBytesCopied, nMaxPktSize, &ulFlags, &ullPts);
		DBG_LOG(DBGLVL_FRAME,  (" Read bytes=%d ulFlags=0x%0x ullPts=%lld", ret, ulFlags, ullPts));
		if(nBytesCopied == 0/*first buffer */)
			pAppData->crnt_input_pts = ullPts;
	}
	if(ulFlags & OMX_BUFFERFLAG_EOS) {
		DBG_LOG(DBGLVL_TRACE, ("AudParserReadData: End of file"));
		pAppData->fEoS = 1;
	}
	nBytesCopied += ret; 


	return nBytesCopied;
}

static int SendOutputData(AENCFake_Client *pAppData, unsigned char *pData, int lenData)
{

	while(pAppData->pConnDest->IsFull(pAppData->pConnDest) && pAppData->fStreaming){
		DBG_LOG(DBGLVL_WAITLOOP,("Waiting for free buffer"))
		OAL_TASK_SLEEP(1)
	}
	pAppData->pConnDest->Write(pAppData->pConnDest, pData, lenData,  0, pAppData->crnt_out_pts * 1000 / 90);
	return 0;
}

static unsigned long
AAC_Read_InputData(AENCFake_Client * pAppData, OMX_BUFFERHEADERTYPE_M * pBufHdr)
{
	unsigned long nRead = 0;

	if (pAppData->pConnSrc != NULL){
		while(pAppData->pConnSrc->IsEmpty(pAppData->pConnSrc) && pAppData->fStreaming)
			OAL_TASK_SLEEP(1)
		if(!pAppData->fStreaming)
			goto Exit;
		nRead = GetInputData(pAppData, pBufHdr->pBuffer, pBufHdr->nAllocLen);
    }
    pBufHdr->nFilledLen = nRead;

Exit:
    return nRead;

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
static unsigned long
AENC_FillData(AENCFake_Client * pAppData, OMX_BUFFERHEADERTYPE_M * pBuf)
{
    unsigned long nRead = 0;
    nRead = AAC_Read_InputData(pAppData, pBuf);
    return nRead;
}


/******************************************************************************/
/* Main entrypoint into the Test */
/******************************************************************************/
static void *thrdProcess(void *thrdArg)
{
	AENCFake_Client *pAppData = (AENCFake_Client *)thrdArg;
	OMX_BUFFERHEADERTYPE_M *pInBuff = pAppData->pInBuff;
	pInBuff->pBuffer = (char *)malloc(1024 * 8);
	pInBuff->nAllocLen = 1024 * 8;
	while (pAppData->fStreaming) {
		AENC_FillData(pAppData, pInBuff);
#ifdef WIN32
		myWaveWrite(pInBuff->pBuffer, pInBuff->nFilledLen);
#endif
	}
	return (0);
}                               /* OMX_Audio_Encode_Test */

int aencchainStart(StrmCompIf *pComp)
{
	AENCFake_Client *pCtx = (AENCFake_Client *)pComp->pCtx;
	pCtx->fStreaming = 1;
	oalThreadCreate(&pCtx->thrdHandle, thrdProcess,pCtx);
	return 0;
}
/* ilclient.c - EOF */

int aencchainSetOption(
	StrmCompIf *pComp, 	
	int         nCmd, 
	char       *pOptionData)
{
	int ret;
	AENCFake_Client *pAppData = (AENCFake_Client *)pComp->pCtx;
#if 0
	if(nCmd == AUD_DEC_CMD_SET_PARAMS) {
		IL_AUD_ARGS  *pArgs = (IL_AUD_ARGS  *)pOptionData;

		strncpy(pAppData->codec_name, pArgs->codec_name, 31);
		pAppData->aacRawFormat = pArgs->nRrawFormat; 
		if(pArgs->nSampleRate) {
			pAppData->audRawSampleRate = pArgs->nSampleRate;
		}
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
	AENCFake_Client *pAppData = (AENCFake_Client *)pComp->pCtx;
	if(pszResource != NULL){
		strcpy(pAppData->device, pszResource);
	}

	DBG_LOG(DBGLVL_TRACE, ("Using user defined sample Rate: %d", (int) pAppData->audRawSampleRate));

	return 0;
}


void aencchainSetClkSrc(StrmCompIf *pComp, void *pClk)
{
	AENCFake_Client *pAppData = (AENCFake_Client *)pComp->pCtx;
	pAppData->pClk = pClk;
}

void *aencchainGetClkSrc(StrmCompIf *pComp)
{
	AENCFake_Client *pAppData = (AENCFake_Client *)pComp->pCtx;
	pAppData->fClkSrc= 1;
	pAppData->pClk = clkCreate(1);
	return pAppData->pClk;
}

void aencchainSetInputConn(StrmCompIf *pComp, int nConnNum, ConnCtxT *pConn)
{
	AENCFake_Client *pAppData = (AENCFake_Client *)pComp->pCtx;
	pAppData->pConnSrc = pConn;
}

void aencchainSetOutputConn(StrmCompIf *pComp, int nConnNum, ConnCtxT *pConn)
{
	AENCFake_Client *pAppData = (AENCFake_Client *)pComp->pCtx;
	pAppData->pConnDest = pConn;
}

int aencchainStop(StrmCompIf *pComp)
{
	int nTimeout = 300000;
	AENCFake_Client *pAppData = (AENCFake_Client *)pComp->pCtx;
	pAppData->fStreaming = 0;
	DBG_LOG(DBGLVL_SETUP, ("Waiting for stream stop beign"));

	oalThreadJoin(pAppData->thrdHandle, 1000);
	DBG_LOG(DBGLVL_TRACE, ("Waiting for stream stop end timeoutreamin=%dms", nTimeout/1000));
	return 0;
}

int aencchainDelete(StrmCompIf *pComp)
{
	if(pComp && pComp->pCtx) {
		AENCFake_Client *pAppData = (AENCFake_Client *)pComp->pCtx;
		free (pAppData);
	}
	if(pComp)
		free(pComp);
	return 0;
}

#define AUD_DEFAULT_INPUT_PKT_SIZE (1024 * 2 * 16) // 16 bit 2 channel 1024 sample frame
StrmCompIf *
aencchainCreate()
{

	StrmCompIf *pComp = (StrmCompIf *)malloc(sizeof(StrmCompIf));
	AENCFake_Client *pAppData = (AENCFake_Client *) malloc (sizeof (AENCFake_Client));
	
	DBG_LOG(DBGLVL_TRACE, ("Begin"));

	memset (pComp, 0x0, sizeof (StrmCompIf));
	memset (pAppData, 0x0, sizeof (AENCFake_Client));
	pComp->pCtx = pAppData;
	pAppData->pInBuff = (OMX_BUFFERHEADERTYPE_M *)malloc(sizeof(OMX_BUFFERHEADERTYPE_M));


#if 0
	pAppData->nJitterLatency = ini_getl(DECODER_SECTION, "JITTER_LATENCY", 0, gIniFile);
	pAppData->nSyncMaxWaitRunning = ini_getl(DECODER_SECTION, "ASYNC_MAX_WAIT_RUNNING", 30000, gIniFile);
	pAppData->nSyncMaxWaitStartup = ini_getl(DECODER_SECTION, "ASYNC_MAX_WAIT_STARTUP", 500000, gIniFile);
	DBG_LOG(DBGLVL_SETUP, ("nSyncMaxWaitStartup=%d",pAppData->nSyncMaxWaitStartup));

	strcpy(pAppData->device,"default"); /* playback device */
	pAppData->audRawSampleRate = 48000;
	pAppData->fPcmPassthru = 0;
	pAppData->fEnableFormatChange = ini_getl(DECODER_SECTION, "ENABLE_AUD_DYNAMIC_FORMAT_CHANGE", 1, gIniFile);
	pAppData->nDecodedFrms = 0;
	pAppData->audchainPrimed = 0;
	pAppData->enc_input_buffer_size = AUD_DEFAULT_INPUT_BUF_SIZE;
	pAppData->enc_output_buffer_size = AUD_DEFAULT_OUTPUT_BUF_SIZE;
	pAppData->alsa_output_buffer_size = ALSA_DEFAULT_PERIOD_BUFF_SIZE * 3; 
#endif
	pAppData->max_input_pkt_size = AUD_DEFAULT_INPUT_PKT_SIZE;

	pAppData->nChannels = 2; //default
	pAppData->bitrate = 128000;
    pAppData->samplerate = 48000;
	//pAppData->eCompressionFormat = OMX_AUDIO_CodingAAC;
	//pAppData->outputformat = OMX_AUDIO_AACStreamFormatMP4ADTS;
    

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
