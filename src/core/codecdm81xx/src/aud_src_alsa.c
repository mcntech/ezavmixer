#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <alsa/asoundlib.h>
#include "dbglog.h"
#include "strmcomp.h"
#include "acap_omx_chain.h"
#include "dbglog.h"
#include "dec_clock.h"
#include "aud_src_alsa.h"
//#define DBG_DUMP

#define FRAMES_PER_XFR (1536)
#define FRAME_SIZE     8			// 2channels * 4bytes/sample
#define XFR_BUF_SIZE (FRAMES_PER_XFR * FRAME_SIZE)
#define AC3_DETECT_SIZE (FRAMES_PER_XFR * 8 * 2) // Check for two frames worth of data
typedef enum _AES_PARSE_STATE
{
	PARSE_STATE_INIT,
	PARSE_STATE_SYNC,
	PARSE_STATE_PAYLOAD
} AES_PARSE_STATE;

typedef enum _DETECTED_AUD_T
{
	DETECTED_AUD_UNKNOWN,
	DETECTED_AUD_PCM,
	DETECTED_AUD_AC3
} DETECTED_AUD_T;

typedef struct _AlsaCapCtx {
	char              device[MAX_AUD_DEV_NAME_SIZE];
	snd_pcm_t         *handle;
	snd_pcm_stream_t  stream; //SND_PCM_STREAM_PLAYBACK or SND_PCM_STREAM_CAPTURE

	int               rate;
	int               nFrameCount;
	unsigned long ulTotalBytes; 
	unsigned long long start_aud_pts;
	unsigned long long crnt_sample;

	int                fDetectFormat;
	int                nParseState;
	int                nAesPa;
	int                nAesPb;
	int                nAesPc;
	int                nAesPd;
	int                nSkippedBytes;
	int                nDetectedfFormat;
} AlsaCapCtx;


typedef struct _AcapSrcCtx
{
	ConnCtxT *pConn;

	int          fEoS;
	int                sync;
	int                latency;
	int                buffer_size;
	int                fStreaming;
#ifdef DBG_DUMP
	int     fDbgSave;
#endif
	int     nUiCmd;
	FILE *hFile;
	AlsaCapCtx captureCtx;
	pthread_t thrdIdCapture;
} AcapSrcCtx;


static void DumpHex(unsigned char *pData, int nSize)
{
	int i;
	for (i=0; i < nSize; i++)
		DBG_PRINT("%02x ", pData[i]);
	DBG_PRINT("\n");
}

static int ConfigureAudio(AlsaCapCtx *pCtx)
{
	int err;
	int exact_rate;
	int buffer_size;// = XFR_BUF_SIZE * 10; // period * periods. TODO: calclualte based on latency requirement
	int res = -1;
	/* the PCM stream. */
	snd_pcm_hw_params_t *hw_params;
	int rate = pCtx->rate;
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	DBG_LOG(DBGLVL_SETUP, ("device=%s stream=%d",pCtx->device, pCtx->stream));

	/* Open PCM. The last parameter of this function is the mode. */
	if ((err = snd_pcm_open (&pCtx->handle, pCtx->device, pCtx->stream, 0))< 0) {
		DBG_LOG(DBGLVL_ERROR, ("Could not open audio device %s. err=%s", pCtx->device, snd_strerror (err)));
		goto Exit;
	}


	/* Allocate the snd_pcm_hw_params_t structure on the stack. */
	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
		DBG_LOG(DBGLVL_ERROR, ("cannot allocate hardware parameters (%s)", snd_strerror (err)));
		goto Exit;
	}

	/* Init hwparams with full configuration space */
	if ((err = snd_pcm_hw_params_any (pCtx->handle, hw_params)) <0) {
		DBG_LOG(DBGLVL_ERROR,  ("cannot initialize hardware parameter structure (%s)\n", snd_strerror (err)));
		goto Exit;
	}

	/* Set access type. */
	if ((err = snd_pcm_hw_params_set_access (pCtx->handle, hw_params,SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		DBG_LOG(DBGLVL_ERROR,  ( "cannot set access type (%s)\n", snd_strerror(err)));
			goto Exit;
	}
	/* Set sample format */
	if ((err = snd_pcm_hw_params_set_format (pCtx->handle, hw_params,/*SND_PCM_FORMAT_SPECIAL*/SND_PCM_FORMAT_S32_LE)) < 0) {
		DBG_LOG(DBGLVL_ERROR,  ("cannot set sample format (%s)\n", snd_strerror	(err)));
			goto Exit;
	}

	buffer_size = XFR_BUF_SIZE * 2;//10;

	if ((err = snd_pcm_hw_params_set_buffer_size_near(pCtx->handle, hw_params,   &buffer_size)) < 0) {
		DBG_LOG(DBGLVL_ERROR,  ("cannot set buffer size (%s)\n", snd_strerror(err)));
			goto Exit;
	}

	/* Set sample rate. If the exact rate is not supported by the
	hardware, use nearest possible rate. */
	exact_rate = rate;
	if ((err = snd_pcm_hw_params_set_rate_near (pCtx->handle, hw_params, (unsigned int *) &rate, 0))  < 0) {
		DBG_LOG(DBGLVL_ERROR,  ("cannot set sample rate (%s)\n", snd_strerror(err)));
		goto Exit;
	}
	if (rate != exact_rate) {
		DBG_LOG(DBGLVL_ERROR, ("The rate %d Hz is not supported by the hardware.==> Using %d Hz instead.\n", rate, exact_rate));
	}

	/* Set number of channels */
	if ((err = snd_pcm_hw_params_set_channels (pCtx->handle,hw_params, 2)) < 0) {
		DBG_LOG(DBGLVL_ERROR,  ("cannot set channel count (%s)\n", snd_strerror(err)));
			goto Exit;
	}
	/* Apply HW parameter settings to PCM device and prepare device. */
	if ((err = snd_pcm_hw_params (pCtx->handle, hw_params)) < 0) {
		DBG_LOG(DBGLVL_ERROR, ("cannot set parameters (%s)\n", snd_strerror(err)));
			goto Exit;
	}

	snd_pcm_hw_params_free (hw_params);

	if ((err = snd_pcm_prepare (pCtx->handle)) < 0) {
		DBG_LOG(DBGLVL_ERROR,  ("cannot prepare audio interface for use (%s)", snd_strerror (err)));
		goto Exit;
	}

	res = 0;
Exit:
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
    return res;
}

static  int acapsrcSetOption(
	StrmCompIf *pComp, 	
	int         nCmd, 
	char       *pOptionData)
{
	AcapSrcCtx *pAppData = (AcapSrcCtx *)pComp->pCtx;
	
	DBG_LOG(DBGLVL_TRACE,("Enter"));

	AlsaCapCtx *pCpCtx = &pAppData->captureCtx;
	if(nCmd == AUD_SRC_ALSA_CMD_SET_PARAMS) {
		IL_AUD_SRC_ALSA_ARGS  *pArgs = (IL_AUD_SRC_ALSA_ARGS  *)pOptionData;
		DBG_LOG(DBGLVL_SETUP,("buffer=%d capture device=%s", pArgs->buffer_size, pArgs->device));
		pAppData->buffer_size= pArgs->buffer_size;
		strcpy(pCpCtx->device, pArgs->device);
		pCpCtx->fDetectFormat = pArgs->fDetectFormat;
	}

	DBG_LOG(DBGLVL_TRACE,("Leave"));

	return 0;
}

#define GET_LONG(p, i) (((p[i] << 24) & 0xFF000000)   | ((p[i+1] << 16) & 0x00FF0000) | ((p[i+2] << 8) &  0x0000FF00) | p[i+3] & 0xFF)
int ParseAesNonPcm(AlsaCapCtx *pCtx, char *pAesData, int nInSize, char *pAc3Data, int *pnAc3Bytes)
{
	int nUsedBytes = 0;
	int nIdx = 0;

	//DBG_LOG(DBGLVL_SETUP, (":Begin nInSize=%d", nInSize));
	//DumpHex((char *)pAesData, 32);
	while(nIdx < nInSize) {
		switch(pCtx->nParseState)
		{
			case PARSE_STATE_INIT:
				if(pAesData[nIdx] == 0x00 && pAesData[nIdx+1] == 0x00 && pAesData[nIdx+2] == 0x72 && pAesData[nIdx+3] == 0xF8/*|| pAesData[nIdx] == 0x6F872 || pAesData[nIdx] == 0x0096F872*/) {
					//DBG_LOG(DBGLVL_SETUP, (":****** Sync1 Found *******"));
					pCtx->nAesPa = GET_LONG(pAesData,nIdx);
					nIdx += 4;
					if(nIdx < nInSize - 4 && (pAesData[nIdx] == 0x00 && pAesData[nIdx+1] == 0x00 && pAesData[nIdx+2] == 0x1F && pAesData[nIdx+3] == 0x4E/* || pAesData[nIdx] == 0x00054E1F || pAesData[nIdx] == 0x00A54E1F*/)) {
						//DBG_LOG(DBGLVL_SETUP, (":****** Sync2 Found *******"));
						pCtx->nAesPb = GET_LONG(pAesData,nIdx);
						nIdx += 4;
						if(nIdx < nInSize - 4) {
							pCtx->nAesPc = GET_LONG(pAesData,nIdx);
							nIdx += 4;
						}
						if(nIdx < nInSize - 4) {
							pCtx->nAesPd = (pAesData[nIdx+2] & 0x00FF) | ((pAesData[nIdx+3] << 8) & 0xFF00);
							pCtx->nAesPd = pCtx->nAesPd - 16;
							nIdx += 4;
							pCtx->nParseState = PARSE_STATE_SYNC;
							pCtx->nSkippedBytes = 0; // Reset
							pCtx->nDetectedfFormat = DETECTED_AUD_AC3;
							DBG_LOG(DBGLVL_FRAME, (":****** Payload Bytes=%d*******", pCtx->nAesPd));
						}
					}
				} else {
					// Skip
					nIdx ++;
					pCtx->nSkippedBytes++;
					if(pCtx->nSkippedBytes > AC3_DETECT_SIZE){
						pCtx->nDetectedfFormat = DETECTED_AUD_PCM;
						DBG_LOG(DBGLVL_FRAME, ("***** Not AC3 data ****", pCtx->nSkippedBytes));
					}
				}
				break;
			case PARSE_STATE_SYNC:
				pCtx->nParseState = PARSE_STATE_PAYLOAD;
				// Todo: parse AC3 Header.
				break;
			case PARSE_STATE_PAYLOAD:
				//if(pCtx->nAesPa == 0xF8720000) 
				{
					*pAc3Data++ = pAesData[nIdx+2];
					*pAc3Data++ = pAesData[nIdx+3];
					nUsedBytes += 2;
					nIdx += 4;
					pCtx->nAesPd -= 4;
				}
				// TODO: Add support for 20bit and 24bit
				if(pCtx->nAesPd <= 0) {
					pCtx->nParseState = PARSE_STATE_INIT;
					pCtx->nAesPd = 0;
				}
				break;
		}
	}
	*pnAc3Bytes = nUsedBytes;
	return pCtx->nDetectedfFormat;
}

static  int threadCapture(AcapSrcCtx *pAppData)
{
	int err;
	char *pAc3Data = NULL;
	char *pData = NULL;
	ConnCtxT *pConn = pAppData->pConn;
	DBG_LOG(DBGLVL_TRACE,("Enter"));
	

	AlsaCapCtx *pCtx = &pAppData->captureCtx;
	
	pAc3Data = (char *)malloc(XFR_BUF_SIZE);
	pData = (char *)malloc(XFR_BUF_SIZE);

	if(pData == NULL) {
		DBG_LOG(DBGLVL_ERROR,("malloc failed"));
		goto Exit;
	}

	while (pAppData->nUiCmd != STRM_CMD_STOP) {
		// Wait for buffer empty
		DBG_LOG(DBGLVL_FRAME, ("Wait for Empty Frame..."));
		while(pConn->IsFull(pConn) && pAppData->nUiCmd != STRM_CMD_STOP) {
			//fprintf(stderr,"<cpw %d>", pConn->BufferFullness(pConn));
			usleep(1000);
		}

		if(!pConn->IsFull(pConn)) {
			int nFramesRead = 0;

			DBG_LOG(DBGLVL_FRAME, ("Capture Frame..."));
			do {
				nFramesRead = snd_pcm_readi (pCtx->handle, pData, FRAMES_PER_XFR); 

				DBG_LOG(DBGLVL_FRAME, ("Capturered %d", nFramesRead));
				if(nFramesRead <= 0){
					//snd_pcm_prepare(pCtx->handle);
					snd_pcm_recover(pCtx->handle, nFramesRead, 0);
					DBG_LOG(DBGLVL_ERROR, ("<<<<<<<<<<<<<<< Buffer overrun. called snd_pcm_prepare err=%d(%s)>>>>>>>>>>>>>>>", nFramesRead, snd_strerror(nFramesRead)));
				} else {
					// Successful
				}
			} while(nFramesRead <= 0 && pAppData->nUiCmd != STRM_CMD_STOP);
			if (pCtx->fDetectFormat) {
				pConn->Write(pConn, pData, nFramesRead * FRAME_SIZE, 0, 0);
			} else {
				int nAc3Bytes = 0;
				int nDetectedFormat = ParseAesNonPcm(pCtx, pData, nFramesRead * FRAME_SIZE, pAc3Data, &nAc3Bytes);
				if(nDetectedFormat == DETECTED_AUD_AC3 && nAc3Bytes > 0) {
					DBG_LOG(DBGLVL_FRAME, ("AC3 Data %d",nAc3Bytes));
					//DumpHex(pAc3Data, 16);
					pConn->Write(pConn, pAc3Data, nAc3Bytes, OMX_AUD_DYNAMIC_FMT_AC3, 0);
				} else if (nDetectedFormat == DETECTED_AUD_PCM) {
					// Ignore for now
					//pConn->Write(pConn, pData, nFramesRead * FRAME_SIZE, OMX_AUD_DYNAMIC_FMT_PCM, 0);
				} else {
					// Ignore data. We may loose a frame in the transition
				}
			}
			pCtx->ulTotalBytes += nFramesRead * FRAME_SIZE;
			pCtx->crnt_sample += nFramesRead;
		}
		pCtx->nFrameCount++;
		
		if(gDbgLevel >= DBGLVL_STAT)
		{
				char szClck[256];
				char szPts[256];
				static CLOCK_T prev_clk = 0;
				static int prev_sample_count = 0;
				CLOCK_T clk = ClockGetInternalTime(NULL);
				if(pCtx->start_aud_pts == 0) {
					pCtx->start_aud_pts = clk;
				}
				
				if(clk - prev_clk >= TIME_SECOND) {
					double avg_frame_rate = 0.0;
					double crnt_frame_rate = 0.0;
					double  strm_time =  1.0 * (clk - pCtx->start_aud_pts) / TIME_SECOND;;
					if(strm_time > 0.0) {
						avg_frame_rate = 1.0 * (pCtx->crnt_sample / 1024) / strm_time;
					}
					crnt_frame_rate = (1.0 * (pCtx->crnt_sample - prev_sample_count) / 1024) / (1.0 * (clk - prev_clk) / TIME_SECOND);
					Clock2HMSF(clk, szClck, 255);
					DBG_PRINT("<AudCp:@%s: TotalFrames=%d TotalBytes=%d crnt_rate=%0.2f avg_rate=%0.2f\n", szClck,pCtx->nFrameCount,  pCtx->ulTotalBytes, crnt_frame_rate, avg_frame_rate);
					prev_sample_count = pCtx->crnt_sample;
					prev_clk = clk;
				}
		}
	}

Exit:
	if (pCtx->handle){
		int err;

		DBG_LOG(DBGLVL_TRACE, ("snd_pcm_drain:Begin"));
		if ((err = snd_pcm_drain (pCtx->handle))< 0)	{
			DBG_LOG(DBGLVL_ERROR, ("Could not drain audio device"));
		}
		DBG_LOG(DBGLVL_TRACE, ("snd_pcm_drain:End"));
		if(pCtx->handle){
			snd_pcm_close (pCtx->handle);
			pCtx->handle = NULL;
		}
	}
	if(pData) {
		free(pData);
	}
	if(pAc3Data){
		free(pAc3Data);
	}
	DBG_LOG(DBGLVL_TRACE,("Leave"));
	
	return (0);
} /* OMX_Audio_Decode_Test */

static  int acapsrcOpen(StrmCompIf *pComp, const char *pszResource)
{
	int err = 0;
	void *ret_value;
	AcapSrcCtx *pAppData = (AcapSrcCtx *)pComp->pCtx;
	AlsaCapCtx *pCpCtx = &pAppData->captureCtx;

	DBG_LOG(DBGLVL_TRACE, ("Enter"));

	if(pszResource != NULL){
		strcpy(pCpCtx->device, pszResource);
	}

	if(ConfigureAudio(pCpCtx) < 0) {
		DBG_LOG(DBGLVL_ERROR, ("Capture configuration failed"));
		err = -1;
		goto Exit;
	}
Exit:
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	return err;
}

static  int acapsrcClose(StrmCompIf *pComp)
{
	int err;
	void *ret_value;
	AcapSrcCtx *pAppData = (AcapSrcCtx *)pComp->pCtx;
	AlsaCapCtx *pCpCtx = &pAppData->captureCtx;

	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	if(pCpCtx->handle) {
		snd_pcm_close (pCpCtx->handle);
		pCpCtx->handle = NULL;
	}

Exit:
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
}

static  int acapsrcStart(StrmCompIf *pComp)
{
	int err;
	void *ret_value;
	AcapSrcCtx *pAppData = (AcapSrcCtx *)pComp->pCtx;
	AlsaCapCtx *pCpCtx = &pAppData->captureCtx;

	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	pAppData->fStreaming = 1;
#ifdef FILE_CWRITE
	pCpCtx->hFile = fopen(FILE_CWRITE, "w+");
#endif

	pthread_create(&pAppData->thrdIdCapture, NULL, threadCapture, pAppData);
Exit:
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
}

static  int acapsrcSetOutputConn(StrmCompIf *pComp, int nCOnnNum, ConnCtxT *pConn)
{
	AcapSrcCtx *pCtx = pComp->pCtx;
	pCtx->pConn = pConn;
	return 0;
}

static  int acapsrcStop(StrmCompIf *pComp)
{
	void *ret_value;
	AcapSrcCtx *pAppData = (AcapSrcCtx *)pComp->pCtx;
	pAppData->nUiCmd = STRM_CMD_STOP;
	int nTimeout = 300000;
	DBG_LOG(DBGLVL_TRACE, ("Waiting for stream stop beign"));
	while(pAppData->fStreaming && nTimeout > 0) {
		usleep(1000);
		nTimeout -= 1000;
	}
	DBG_LOG(DBGLVL_TRACE, ("Waiting for stream stop end timeoutreamin=%dms", nTimeout/1000));

	if(pAppData->thrdIdCapture) {
		DBG_LOG(DBGLVL_TRACE, ("Wait for exiting for capture thread"));
		pthread_join (pAppData->thrdIdCapture, (void **) &ret_value);
	}

#ifdef FILE_CWRITE
	close(pCpCtx->hFile);
#endif

Exit:
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	pAppData->fStreaming = 0;

	return 0;
}

static  int acapsrcDelete(StrmCompIf *pComp)
{
	AcapSrcCtx *pAppData = (AcapSrcCtx *)pComp->pCtx;
	free (pAppData);
	return 0;
}
static void InitDefaults(AcapSrcCtx *pAppData)
{
	AlsaCapCtx *pCpCtx = &pAppData->captureCtx;

	pCpCtx->stream = SND_PCM_STREAM_CAPTURE;
	pCpCtx->nDetectedfFormat = DETECTED_AUD_UNKNOWN;
	pCpCtx->nParseState = PARSE_STATE_INIT;

	pCpCtx->fDetectFormat = 0;
	strcpy(pCpCtx->device, "sdi_audio");
	pCpCtx->rate = 48000;
}

StrmCompIf *
audsrcalsaCreate()
{
	StrmCompIf *pComp = (StrmCompIf *)malloc(sizeof(StrmCompIf));
	memset (pComp, 0x0, sizeof (StrmCompIf));
	AcapSrcCtx *pAppData = (AcapSrcCtx *) malloc (sizeof (AcapSrcCtx));
	memset (pAppData, 0x0, sizeof (AcapSrcCtx));
	InitDefaults(pAppData);
	pComp->pCtx = pAppData;

	pComp->Open= acapsrcOpen;
	pComp->SetOption = acapsrcSetOption;
	pComp->SetInputConn= NULL;
	pComp->SetOutputConn= acapsrcSetOutputConn;
	pComp->SetClkSrc = NULL;
	pComp->GetClkSrc = NULL;
	pComp->Start = acapsrcStart;
	pComp->Stop = acapsrcStop;
	pComp->Close = acapsrcClose;
	pComp->Delete = acapsrcDelete;
	return pComp;
}
