#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <alsa/asoundlib.h>
#include <sched.h> 

#include "dbglog.h"
#include "strmcomp.h"
#include "acap_omx_chain.h"
#include "dbglog.h"
#include "dec_clock.h"

//#define FILE_CWRITE "aud_cap_dump.dat"

#define FRAMES_PER_XFR 1024 
#define FRAME_SIZE     8			// 2channels * 4bytes/sample
#define XFR_BUF_SIZE (FRAMES_PER_XFR * FRAME_SIZE)


typedef struct _AlsaAudCtx {
	char              device[MAX_AUD_DEV_NAME_SIZE];
	snd_pcm_t         *handle;
	snd_pcm_stream_t  stream; //SND_PCM_STREAM_PLAYBACK or SND_PCM_STREAM_CAPTURE

	snd_ctl_t *ctl;
	snd_ctl_elem_value_t *ctl_notify;
	snd_ctl_elem_value_t *ctl_rate_shift;
	snd_ctl_elem_value_t *ctl_active;
	snd_ctl_elem_value_t *ctl_format;
	snd_ctl_elem_value_t *ctl_rate;
	snd_ctl_elem_value_t *ctl_channels;


	int               rate;
	int               nFrameCount;
	unsigned long ulTotalBytes; 
	unsigned long long crnt_aud_pts;
	unsigned long long start_aud_pts;
	unsigned long long crnt_sample;
	FILE *hFile;
} AlsaAudCtx;


typedef struct ALOOP_Client
{
	ConnCtxT *pConn;

	int          fEoS;

	int                sync;
	int                latency;
	int                buffer_size;
	int                fStreaming;
	int     nUiCmd;

	AlsaAudCtx playbackCtx;
	AlsaAudCtx captureCtx;
	pthread_t thrdIdPlayback;
	pthread_t thrdIdCapture;
} ALOOP_Client;

void set_realtime_priority() 
{    
	int ret;
	pthread_t this_thread = pthread_self();
	struct sched_param params;
	params.sched_priority = sched_get_priority_max(SCHED_FIFO);
	ret = pthread_setschedparam(this_thread, SCHED_RR/*SCHED_FIFO*/, &params);
	if (ret != 0) {    
		DBG_LOG(DBGLVL_ERROR, ("Can not set thread priority %d", ret));    
		return;
	}
}

static void openctl_elem(AlsaAudCtx *pCtx,
			 int device, int subdevice,
			 const char *name,
			 snd_ctl_elem_value_t **elem)
{
	int err;

	if (snd_ctl_elem_value_malloc(elem) < 0) {
		*elem = NULL;
	} else {
		snd_ctl_elem_value_set_interface(*elem,
						 SND_CTL_ELEM_IFACE_PCM);
		snd_ctl_elem_value_set_device(*elem, device);
		snd_ctl_elem_value_set_subdevice(*elem, subdevice);
		snd_ctl_elem_value_set_name(*elem, name);
		err = snd_ctl_elem_read(pCtx->ctl, *elem);
		if (err < 0) {
			snd_ctl_elem_value_free(*elem);
			*elem = NULL;
		}
	}
}

static int set_rate_shift(AlsaAudCtx *pCtx, double pitch)
{
	int err;

	if (pCtx->ctl_rate_shift == NULL)
		return 0;
	snd_ctl_elem_value_set_integer(pCtx->ctl_rate_shift, 0, pitch * 100000);
	err = snd_ctl_elem_write(pCtx->ctl, pCtx->ctl_rate_shift);
	if (err < 0) {
		DBG_LOG(DBGLVL_ERROR, ("Cannot set PCM Rate Shift element  %s\n",  snd_strerror(err)));
		return err;
	}
	return 0;
}

//Ram: Do it only for capture
static int openctl(AlsaAudCtx *pCtx, int device, int subdevice)
{
	int err;

	pCtx->ctl_rate_shift = NULL;
	openctl_elem(pCtx, device, subdevice, "PCM Notify",
			&pCtx->ctl_notify);
	openctl_elem(pCtx, device, subdevice, "PCM Rate Shift 100000",
			&pCtx->ctl_rate_shift);
	set_rate_shift(pCtx, 1);
	openctl_elem(pCtx, device, subdevice, "PCM Slave Active",
			&pCtx->ctl_active);
	openctl_elem(pCtx, device, subdevice, "PCM Slave Format",
			&pCtx->ctl_format);
	openctl_elem(pCtx, device, subdevice, "PCM Slave Rate",
			&pCtx->ctl_rate);
	openctl_elem(pCtx, device, subdevice, "PCM Slave Channels",
			&pCtx->ctl_channels);
#if 0
	if ((pCtx->ctl_active &&
	     pCtx->ctl_format &&
	     pCtx->ctl_rate &&
	     pCtx->ctl_channels) ||
	    pCtx->loopback->controls) {
	      __events:
		if ((err = snd_ctl_poll_descriptors_count(pCtx->ctl)) < 0)
			pCtx->ctl_pollfd_count = 0;
		else
			pCtx->ctl_pollfd_count = err;
		if (snd_ctl_subscribe_events(pCtx->ctl, 1) < 0)
			pCtx->ctl_pollfd_count = 0;
	}
#endif
	return 0;
}

int ConfigureAudioCtrl(AlsaAudCtx *pCtx)
{
	int err;
	err = snd_ctl_open(&pCtx->ctl, "hw:0"/*card ?*/, SND_CTL_NONBLOCK);
	//openctl(pCtx, device, subdevice);
	openctl(pCtx, 0, 0);
}

int ConfigureAudio(AlsaAudCtx *pCtx)
{
	int err;
	int exact_rate;
	int buffer_size_num_samples;// = XFR_BUF_SIZE * 10; // period * periods. TODO: calclualte based on latency requirement
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
	if ((err = snd_pcm_hw_params_set_format (pCtx->handle, hw_params,SND_PCM_FORMAT_S32_LE)) < 0) {
		DBG_LOG(DBGLVL_ERROR,  ("cannot set sample format (%s)\n", snd_strerror	(err)));
			goto Exit;
	}

	/* Set buffer size */
	/* Set buffer size (in frames). The resulting latency is given by */
	/* latency = periodsize * periods / (rate * bytes_per_frame)     */
	if(pCtx->stream == SND_PCM_STREAM_PLAYBACK) {
		buffer_size_num_samples = 3 * FRAMES_PER_XFR;//XFR_BUF_SIZE * 2;//10;
	} else {
		buffer_size_num_samples = 3 * FRAMES_PER_XFR;//XFR_BUF_SIZE * 2;//10;
	}
	if ((err = snd_pcm_hw_params_set_buffer_size_near(pCtx->handle, hw_params,   &buffer_size_num_samples)) < 0) {
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

	if(pCtx->stream == SND_PCM_STREAM_PLAYBACK) {
		snd_pcm_sw_params_t *swparams;
		snd_pcm_sw_params_alloca(&swparams);
		snd_pcm_sw_params_current(pCtx->handle, swparams);
		err = snd_pcm_sw_params_set_start_threshold(pCtx->handle, swparams, 3 * 1024);
		snd_pcm_sw_params(pCtx->handle, swparams);
	}

	if ((err = snd_pcm_prepare (pCtx->handle)) < 0) {
		DBG_LOG(DBGLVL_ERROR,  ("cannot prepare audio interface for use (%s)", snd_strerror (err)));
		goto Exit;
	}

	res = 0;
Exit:
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
    return res;
}

int acapchainSetOption(
	StrmCompIf *pComp, 	
	int         nCmd, 
	char       *pOptionData)
{
	ALOOP_Client *pAppData = (ALOOP_Client *)pComp->pCtx;
	
	DBG_LOG(DBGLVL_TRACE,("Enter"));

	AlsaAudCtx *pPbCtx = &pAppData->playbackCtx;
	AlsaAudCtx *pCpCtx = &pAppData->captureCtx;
	if(nCmd == AUD_LOOP_CMD_SET_PARAMS) {

		IL_ALOOP_ARGS  *pArgs = (IL_ALOOP_ARGS  *)pOptionData;
		DBG_LOG(DBGLVL_SETUP,("buffer=%d playback device=%s capture device=%s", pArgs->buffer_size, pArgs->output_device, pArgs->input_device));
		pAppData->buffer_size= pArgs->buffer_size;
		strcpy(pPbCtx->device, pArgs->output_device);
		strcpy(pCpCtx->device, pArgs->input_device);
	}

	DBG_LOG(DBGLVL_TRACE,("Leave"));

	return 0;
}

int threadPlayback(ALOOP_Client *pAppData)
{
	int err;
	AlsaAudCtx *pCtx = &pAppData->playbackCtx;
	ConnCtxT *pConn = pAppData->pConn;

	DBG_LOG(DBGLVL_TRACE,("Enter"));
	
	char *pData = (char *)malloc(XFR_BUF_SIZE);
	if(pData == NULL) {
		DBG_LOG(DBGLVL_ERROR,("malloc failed"));
		goto Exit;
	}

	set_realtime_priority();

	while (pAppData->nUiCmd != STRM_CMD_STOP) {
		// Wait for buffer available
		DBG_LOG(DBGLVL_FRAME, ("Wait for Filled Frame..."));
		while(pConn->IsEmpty(pConn) && pAppData->nUiCmd != STRM_CMD_STOP) {
			//fprintf(stderr,"<pbw %d>", pConn->BufferFullness(pConn));
			//usleep(1000);
			usleep(5000);
		}
		if(!pConn->IsEmpty(pConn)) {
			unsigned long ulFlags;
			unsigned long long ullPts;
			int nRet;
			DBG_LOG(DBGLVL_FRAME, ("Play Frame..."));
			int nBytesRead = pConn->Read(pConn, pData, XFR_BUF_SIZE, &ulFlags, &ullPts);
			DBG_LOG(DBGLVL_FRAME, ("Play %d frames", nBytesRead / FRAME_SIZE));
			nRet = snd_pcm_writei (pCtx->handle, pData, nBytesRead / FRAME_SIZE);
#ifdef FILE_PWRITE
			if (pCtx->hFile)
				fwrite(pData, nBytesRead, 1, pCtx->hFile);
#endif

			if ( nRet < 0) {
				//snd_pcm_prepare(pCtx->handle);
				snd_pcm_recover(pCtx->handle, nRet, 0);
				DBG_LOG(DBGLVL_ERROR, ("<<<<<<<<<<<<<<< Buffer Underrun. called snd_pcm_prepare >>>>>>>>>>>>>>>"));
			} else {
				//fprintf (stdout, "snd_pcm_writei successful\n");
			}
			pCtx->nFrameCount++;
			pCtx->crnt_sample += nBytesRead / FRAME_SIZE;
			pCtx->ulTotalBytes += nBytesRead;
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
					DBG_PRINT("<AudPb:@%s: TotalFrames=%d TotalBytes=%d crnt_rate=%0.2f avg_rate=%0.2f\n", szClck,pCtx->nFrameCount,  pCtx->ulTotalBytes, crnt_frame_rate, avg_frame_rate);
					prev_sample_count = pCtx->crnt_sample;
					prev_clk = clk;
				}
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
		snd_pcm_close (pCtx->handle);
	}

	if(pData) {
		free(pData);
	}
	DBG_LOG(DBGLVL_TRACE,("Leave"));
	return (0);
} /* OMX_Audio_Decode_Test */

int threadCapture(ALOOP_Client *pAppData)
{
	int err;
	ConnCtxT *pConn = pAppData->pConn;
	DBG_LOG(DBGLVL_TRACE,("Enter"));

	AlsaAudCtx *pCtx = &pAppData->captureCtx;

	char *pData = (char *)malloc(XFR_BUF_SIZE);
	if(pData == NULL) {
		DBG_LOG(DBGLVL_ERROR,("malloc failed"));
		goto Exit;
	}
	
	set_realtime_priority();

	while (pAppData->nUiCmd != STRM_CMD_STOP) {
		// Wait for buffer empty
		DBG_LOG(DBGLVL_FRAME, ("Wait for Empty Frame..."));
		while(pConn->IsFull(pConn) && pAppData->nUiCmd != STRM_CMD_STOP) {
			//fprintf(stderr,"<cpw %d>", pConn->BufferFullness(pConn));
			//usleep(1000);
			usleep(5000);
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
			pConn->Write(pConn, pData, nFramesRead * FRAME_SIZE, 0, 0);
#ifdef FILE_CWRITE
			if (pCtx->hFile)
				fwrite(pData, nFramesRead * FRAME_SIZE, 1, pCtx->hFile);
#endif

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
		snd_pcm_close (pCtx->handle);
	}
	if(pData) {
		free(pData);
	}
	DBG_LOG(DBGLVL_TRACE,("Leave"));
	
	return (0);
} /* OMX_Audio_Decode_Test */

int acapchainStart(StrmCompIf *pComp)
{
	int err;
	void *ret_value;
	ALOOP_Client *pAppData = (ALOOP_Client *)pComp->pCtx;
	AlsaAudCtx *pPbCtx = &pAppData->playbackCtx;
	AlsaAudCtx *pCpCtx = &pAppData->captureCtx;
	DBG_LOG(DBGLVL_TRACE, ("Enter"));
	pAppData->fStreaming = 1;
	
	pAppData->pConn = CreateStrmConn(XFR_BUF_SIZE, 3);

	if(ConfigureAudio(pCpCtx) < 0) {
		DBG_LOG(DBGLVL_ERROR, ("Capture configuration failed"));
		goto Exit;
	}
	snd_pcm_close (pCpCtx->handle);

	if(ConfigureAudio(pPbCtx) < 0) {
		DBG_LOG(DBGLVL_ERROR, ("Playback configuration failed"));
		goto Exit;
	}

	if(ConfigureAudio(pCpCtx) < 0) {
		DBG_LOG(DBGLVL_ERROR, ("Capture configuration failed"));
		goto Exit;
	}
	//ConfigureAudioCtrl(pCpCtx);

#ifdef FILE_CWRITE
	pCpCtx->hFile = fopen(FILE_CWRITE, "w+");
#endif
#ifdef FILE_PWRITE
	pPbCtx->hFile = fopen(FILE_PWRITE, "w+");
#endif

	pthread_create(&pAppData->thrdIdPlayback, NULL, threadPlayback, pAppData);
	pthread_create(&pAppData->thrdIdCapture, NULL, threadCapture, pAppData);

	if(pAppData->thrdIdCapture) {
		DBG_LOG(DBGLVL_TRACE, ("Wait for exiting for capture thread"));
		pthread_join (pAppData->thrdIdCapture, (void **) &ret_value);
	}
	if(pAppData->thrdIdPlayback) {
		DBG_LOG(DBGLVL_TRACE, ("Wait for exiting for playback thread"));
		pthread_join (pAppData->thrdIdPlayback, (void **) &ret_value);
	}

#ifdef FILE_CWRITE
	close(pCpCtx->hFile);
#endif
#ifdef FILE_PWRITE
	close(pPbCtx->hFile);
#endif

	DeleteStrmConn(pAppData->pConn);

Exit:
	DBG_LOG(DBGLVL_TRACE, ("Leave"));
	pAppData->fStreaming = 0;
}

int acapchainStop(StrmCompIf *pComp)
{
	ALOOP_Client *pAppData = (ALOOP_Client *)pComp->pCtx;
	pAppData->nUiCmd = STRM_CMD_STOP;
	int nTimeout = 300000;
	DBG_LOG(DBGLVL_TRACE, ("Waiting for stream stop beign"));
	while(pAppData->fStreaming && nTimeout > 0) {
		usleep(1000);
		nTimeout -= 1000;
	}
	DBG_LOG(DBGLVL_TRACE, ("Waiting for stream stop end timeoutreamin=%dms", nTimeout/1000));
	return 0;
}

int acapchainDelete(StrmCompIf *pComp)
{
	ALOOP_Client *pAppData = (ALOOP_Client *)pComp->pCtx;
	free (pAppData);
	return 0;
}
static void InitDefaults(ALOOP_Client *pAppData)
{
	AlsaAudCtx *pPbCtx = &pAppData->playbackCtx;
	AlsaAudCtx *pCpCtx = &pAppData->captureCtx;

	pPbCtx->stream = SND_PCM_STREAM_PLAYBACK;
	pPbCtx->rate = 48000;
	strcpy(pPbCtx->device, "default");

	pCpCtx->stream = SND_PCM_STREAM_CAPTURE;
	strcpy(pCpCtx->device, "sdi_audio");
	pCpCtx->rate = 48000;
}

StrmCompIf *
acapchainCreate()
{
	StrmCompIf *pComp = (StrmCompIf *)malloc(sizeof(StrmCompIf));
	memset (pComp, 0x0, sizeof (StrmCompIf));
	ALOOP_Client *pAppData = (ALOOP_Client *) malloc (sizeof (ALOOP_Client));
	memset (pAppData, 0x0, sizeof (ALOOP_Client));
	InitDefaults(pAppData);
	pComp->pCtx = pAppData;

	pComp->Open= NULL;
	pComp->SetOption = acapchainSetOption;
	pComp->SetInputConn= NULL;
	pComp->SetOutputConn= NULL;
	pComp->SetClkSrc = NULL;
	pComp->GetClkSrc = NULL;
	pComp->Start = acapchainStart;
	pComp->Stop =acapchainStop;
	pComp->Close = NULL;
	pComp->Delete = acapchainDelete;
	return pComp;
}
