#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/avfft.h>
}

#include "decmp2.h"
#include "JdDbg.h"
#include "JdOsal.h"
#include "unistd.h"

#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096

#define READ_MAX_SIZE (AUDIO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE)
#define AUDIO_OUTBUF_SIZE (32*2048)

typedef void *(*thrdStartFcnPtr) (void *);
static int modDbgLevel = CJdDbg::LVL_TRACE;


typedef struct
{
    int        fpIn;
    ConnCtxT   *pConnIn;
    ConnCtxT   *pConnOut;
    int        nBlockSize;
#ifdef WIN32
    HANDLE     thrdId;
#else
    pthread_t  thrdId;
#endif
    int        nUiCmd;
    long long  llTotolRead;
    unsigned char *mPcmBuffer;
    int         mfRun;
} decCtx;



static void threadDecProcess(void *threadsArg)
{
    decCtx *pCtx =  (decCtx *)threadsArg;
    FFTContext *c= NULL;
    float      *sample_buffer;
    int len, ret;
    uint8_t *inbuf = (uint8_t *)malloc(READ_MAX_SIZE);
    pCtx->mPcmBuffer = (unsigned char*)malloc(AUDIO_OUTBUF_SIZE);
    uint8_t *data;
    size_t   data_size;
    unsigned long ulFlags;
    long long ullPts;

    c = av_fft_init(8, 0);
    if (!c) {
        JDBG_LOG(CJdDbg::LVL_ERR, ( "av_fft_init : failed\n"));
        goto Exit;
    }
    sample_buffer = (float*)av_mallocz(sizeof(float)*1024);
    if (!sample_buffer) {
        JDBG_LOG(CJdDbg::LVL_ERR, ( "av_mallocz : failed\n"));
        goto Exit;
    }
    /* decode until eof */
    data      = inbuf;
    //data_size = fread(inbuf, 1, AUDIO_INBUF_SIZE, f);
    pCtx->mfRun = 1;
    pCtx->nUiCmd = STRM_CMD_NONE;

    while(pCtx->mfRun) {

        if(pCtx->nUiCmd == STRM_CMD_STOP) {
            pCtx->mfRun =  false;
            break;
        } else {
            // TODO: implement other commands
        }

        data_size = pCtx->pConnIn->Read(pCtx->pConnIn, (char *) data, READ_MAX_SIZE, &ulFlags,
                                        &ullPts);

       if (data_size > 0) {
           for (int i=0 ; i<data_size ; i++)
               sample_buffer[i] = 0;//get_vlc2(&q->gb, q->vlc_table.table, vlc_table.bits, 3);  //read about the arguments in bitstream.h

           av_fft_permute(c, (FFTComplex *)sample_buffer);
            av_fft_calc(c, (FFTComplex *)sample_buffer);

            data += ret;
            data_size -= ret;


        }
    }
    av_fft_end(c);
    /* flush the decoder */

Exit:

    if(inbuf)
        free(inbuf);
    if(pCtx->mPcmBuffer)
        free(pCtx->mPcmBuffer);
}

static int DecInit(decCtx *pCtx)
{
    pCtx->nBlockSize = (21 * 188);
    return 0;
}

static int decOpen(StrmCompIf *pComp, const char *szFile)
{
    return 0;
}

static int decSetOption(StrmCompIf *pComp, int nCmd, char *pOptionData)
{
    decCtx *pCtx = (decCtx *)pComp->pCtx;
    return 0;
}

static int decSetInputConn(struct _StrmCompIf *pComp, int nConnNum, ConnCtxT *pConn)
{
    decCtx *pCtx = (decCtx *)pComp->pCtx;
    pCtx->pConnIn = pConn;
    return 0;
}

static int decSetOutputConn(StrmCompIf *pComp, int nConnNum, ConnCtxT *pConn)
{
    decCtx *pCtx = (decCtx *)pComp->pCtx;

    pCtx->pConnOut = pConn;

    return 0;
}

static void decClose(StrmCompIf *pComp)
{
    decCtx *pCtx = (decCtx *)pComp->pCtx;
    return;
}

static int decStart(StrmCompIf *pComp)
{
    int res = 0;
    decCtx *pCtx = (decCtx *)pComp->pCtx;

    if(pCtx->pConnIn == NULL ) {
        JdDbg(CJdDbg::DBGLVL_ERROR, ("Error starting file src !"));
        res = -1;
        goto Exit;
    }
#ifdef WIN32
    {
		DWORD dwThreadId;
		pCtx->thrdId = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadDecProcess, pCtx, 0, &dwThreadId);
	}
#else
    if (pthread_create (&pCtx->thrdId, NULL, (thrdStartFcnPtr) threadDecProcess, pCtx) != 0)	{
        JdDbg(CJdDbg::DBGLVL_ERROR, ("Create_Task failed !"));
    }
#endif
    Exit:
    return res;
}

static int decStop(StrmCompIf *pComp)
{
    decCtx *pCtx = (decCtx *)pComp->pCtx;
    void *ret_value;
    int nTimeOut = 1000000; // 1000 milli sec
    // Attempt closing the tthread
    JdDbg(CJdDbg::DBGLVL_TRACE, ("Enter fStreaming=%d", pCtx->mfRun));

    if(pCtx->mfRun) {
        pCtx->nUiCmd = STRM_CMD_STOP;
        JdDbg(CJdDbg::DBGLVL_TRACE, ("fStreaming=%d", pCtx->mfRun));
        while(pCtx->mfRun && nTimeOut > 0) {
            nTimeOut -= 1000;
#ifdef WIN32
            Sleep(1);
#else
            usleep(1000);
#endif
        }
        // If thread did not exit, force close it
        JdDbg(CJdDbg::DBGLVL_TRACE, ("mfRun=%d nTimeOut rem=%d", pCtx->mfRun, nTimeOut));

    }

 /*
    if(pCtx->mfRun) {
        JdDbg(CJdDbg::DBGLVL_SETUP, ("[Thread did nit exit. Cancelling the thread..."));
#ifdef WIN32
#else
        //pthread_cancel(pCtx->thrdId);
        pthread_join (pCtx->thrdId, (void **) &ret_value);
#endif
        JdDbg(CJdDbg::DBGLVL_SETUP, ("Cancelling the thread: Done]"));
    }
*/

    JdDbg(CJdDbg::DBGLVL_TRACE, ("Leave"));
    return 0;
}

static void decDelete(struct _StrmCompIf *pComp)
{
    if(pComp->pCtx) {
        free(pComp->pCtx);
    }
    free(pComp);
}

StrmCompIf *
audfftCreate()
{
    StrmCompIf *pComp = (StrmCompIf *)malloc(sizeof(StrmCompIf));
    pComp->pCtx = malloc(sizeof(decCtx));
    memset(pComp->pCtx, 0x00, sizeof(decCtx));

    DecInit((decCtx *)pComp->pCtx);

    pComp->Open= decOpen;
    pComp->SetOption = decSetOption;
    pComp->SetInputConn= decSetInputConn;
    pComp->SetOutputConn= decSetOutputConn;
    pComp->SetClkSrc = NULL;
    pComp->GetClkSrc = NULL;
    pComp->Start = decStart;
    pComp->Stop = decStop;
    pComp->Close = decClose;
    pComp->Delete = decDelete;
    return pComp;
}
