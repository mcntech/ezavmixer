#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libavutil/frame.h>
#include <libavutil/mem.h>

#include <libavcodec/avcodec.h>
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
    int        fStreaming;
    int        nUiCmd;
    long long  llTotolRead;
    unsigned char *mPcmBuffer;
} decCtx;

static int  decode(decCtx *pCtx, AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *frame)
{
    int i, ch;
    int ret, data_size;
    int pcmLen =0;
    unsigned char *pOut = pCtx->mPcmBuffer;
    /* send the packet with the compressed data to the decoder */
    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0) {
        JDBG_LOG(CJdDbg::LVL_ERR, ("Error submitting the packet to the decoder\n"));
        return ret;
    }

    /* read all the output frames (in general there may be any number of them */
    while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return ret;
        else if (ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            exit(1);
        }
        data_size = av_get_bytes_per_sample(dec_ctx->sample_fmt);
        if (data_size < 0) {
            /* This should not occur, checking just for paranoia */
            JDBG_LOG(CJdDbg::LVL_ERR, ("Failed to calculate data size\n"));
            return ret;
        }
        pcmLen = 0;
        for (i = 0; i < frame->nb_samples; i++)
            for (ch = 0; ch < dec_ctx->channels; ch++) {
                memcpy(&pOut[pcmLen], frame->data[ch] + data_size*i, data_size);
                pcmLen += data_size;

            }
        pCtx->pConnOut->Write(pCtx->pConnOut, (char *)pOut, pcmLen,  0/*pkt->flags*/, pkt->pts * 1000 / 90);
    }
}

static void threadDecProcess(void *threadsArg)
{
    decCtx *pCtx =  (decCtx *)threadsArg;
    const AVCodec *codec;
    AVCodecContext *c= NULL;
    AVCodecParserContext *parser = NULL;
    int len, ret;
    uint8_t *inbuf = (uint8_t *)malloc(READ_MAX_SIZE);
    pCtx->mPcmBuffer = (unsigned char*)malloc(AUDIO_OUTBUF_SIZE);
    uint8_t *data;
    size_t   data_size;
    AVPacket *pkt = NULL;
    AVFrame *decoded_frame = NULL;
    unsigned long ulFlags;
    long long ullPts;

    /* register all the codecs */
    avcodec_register_all();

    pkt = av_packet_alloc();

    /* find the MPEG audio decoder */
    codec = avcodec_find_decoder(AV_CODEC_ID_MP2);
    if (!codec) {
        JDBG_LOG(CJdDbg::LVL_ERR,("Codec not found\n"));
        goto Exit;
    }

    parser = av_parser_init(codec->id);
    if (!parser) {
        JDBG_LOG(CJdDbg::LVL_ERR, ("Parser not found\n"));
        goto Exit;
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        JDBG_LOG(CJdDbg::LVL_ERR, ( "Could not allocate audio codec context\n"));
        goto Exit;
    }

    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        JDBG_LOG(CJdDbg::LVL_ERR, ("Could not open codec\n"));
        goto Exit;
    }


    /* decode until eof */
    data      = inbuf;
    //data_size = fread(inbuf, 1, AUDIO_INBUF_SIZE, f);

    data_size = pCtx->pConnIn->Read(pCtx->pConnIn, (char *)data, READ_MAX_SIZE, &ulFlags, &ullPts);

    while (data_size > 0) {
        if (!decoded_frame) {
            if (!(decoded_frame = av_frame_alloc())) {
                fprintf(stderr, "Could not allocate audio frame\n");
                exit(1);
            }
        }

        ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
                               data, data_size,
                               AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        if (ret < 0) {
            fprintf(stderr, "Error while parsing\n");
            exit(1);
        }
        data      += ret;
        data_size -= ret;

        if (pkt->size)
            decode(pCtx, c, pkt, decoded_frame);

        if (data_size < AUDIO_REFILL_THRESH) {
            memmove(inbuf, data, data_size);
            data = inbuf;
            //len = fread(data + data_size, 1, AUDIO_INBUF_SIZE - data_size, f);
            len = pCtx->pConnIn->Read(pCtx->pConnIn, (char *)data + data_size, READ_MAX_SIZE, &ulFlags, &ullPts);
            if (len > 0)
                data_size += len;
        }
    }

    /* flush the decoder */
    pkt->data = NULL;
    pkt->size = 0;
    decode(pCtx, c, pkt, decoded_frame);

Exit:
    if(c)
     avcodec_free_context(&c);

    if(parser)
        av_parser_close(parser);

    if(decoded_frame)
        av_frame_free(&decoded_frame);

    if(pkt)
        av_packet_free(&pkt);

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
    pCtx->fStreaming = 1;

    if(pCtx->pConnIn == NULL || pCtx->pConnOut == 0) {
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
    JdDbg(CJdDbg::DBGLVL_TRACE, ("Enter fStreaming=%d", pCtx->fStreaming));

    if(pCtx->fStreaming) {
        pCtx->nUiCmd = STRM_CMD_STOP;
        JdDbg(CJdDbg::DBGLVL_TRACE, ("fStreaming=%d", pCtx->fStreaming));
        while(pCtx->fStreaming && nTimeOut > 0) {
            nTimeOut -= 1000;
#ifdef WIN32
            Sleep(1);
#else
            usleep(1000);
#endif
        }
        // If thread did not exit, force close it
        JdDbg(CJdDbg::DBGLVL_TRACE, ("fStreaming=%d nTimeOut rem=%d", pCtx->fStreaming, nTimeOut));

    }

    if(pCtx->fStreaming) {
        // Close the file to force EoS
        JdDbg(CJdDbg::DBGLVL_TRACE, ("Force EoS by closing source"));

        nTimeOut = 1000000;
        while(pCtx->fStreaming && nTimeOut > 0) {
            nTimeOut -= 1000;
#ifdef WIN32
            Sleep(1);
#else
            usleep(1000);
#endif
        }
    }

    if(pCtx->fStreaming) {
        JdDbg(CJdDbg::DBGLVL_SETUP, ("[Thread did nit exit. Cancelling the thread..."));
#ifdef WIN32
#else
        //pthread_cancel(pCtx->thrdId);
        pthread_join (pCtx->thrdId, (void **) &ret_value);
#endif
        JdDbg(CJdDbg::DBGLVL_SETUP, ("Cancelling the thread: Done]"));
    }

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
decmp2Create()
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
