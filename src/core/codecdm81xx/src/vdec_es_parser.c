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
*  @file   h264_parser.c
*  @brief  This file contains utiltity functions for parsing H.264 streams
*
*          This file contains functions for parsing H.264 streams. To understand
*          and/or modify these functions, one needs to have an understanding of
*          H.264 stream formats.
*
*  @rev 1.0
*******************************************************************************
*/
#include <stdio.h>
#include <malloc.h>
#include <sys/stat.h>

#ifdef WIN32
#include <windows.h>
#endif

#include "vdec_es_parser.h"
#include "dbglog.h"
#include "dec_omx_chains_common.h"

 
static int ParserReadData(ParserBaseIf *pComp, ConnCtxT *pConn, unsigned char *pData, int lenData, 
	unsigned long long *pPts, int *pfEos, int *pfDisCont
)
{
	int ret = 0;
	unsigned long ulFlags = 0;
	// TODO : Add termination condition
	*pfDisCont = 0;
	while(pConn->IsEmpty(pConn) && pComp->nUiCmd !=  STRM_CMD_STOP){
		DBG_LOG(DBGLVL_WAITLOOP,(":Waiting for data"))
#ifdef WIN32
		Sleep(1);
#else
		usleep(1000);
#endif
	}

	if(pComp->nUiCmd ==  STRM_CMD_STOP) {
		ulFlags = OMX_BUFFERFLAG_EOS;
	} else {
		ret = pConn->Read(pConn, pData, lenData, &ulFlags, pPts);
	}

	if(ulFlags & OMX_BUFFERFLAG_EOS) {
		*pfEos = 1;
	} else {
		*pfEos = 0;
	}
	if(ulFlags & OMX_EXT_BUFFERFLAG_DISCONT) {
		*pfDisCont = 1;
	}
	
	return ret;
}

/******************************************************************************\
*      Decode_ChunkingCtxInit Function Declaration
\******************************************************************************/
/**
*
*  @brief    H.264 Chunking Context initialization
*
*  @param in:
*            c: Pointer to H264_ChunkingCtx structure
*
*  @param Out:
*            None
*
*  @return   void
*
*/
void Decode_ChunkingCtxInit (H264_ChunkingCtx *c)
{
  c->workingWord = H264_WORKING_WORD_INIT;
  c->fifthByte = 0xFF;
  c->state = H264_ST_LOOKING_FOR_SPS;
}

/******************************************************************************\
*      Decode_H264ParserInit Function Declaration
\******************************************************************************/
/**
*
*  @brief    H.264  parser initialization
*
*  @param in:
*            pc: Pointer to H264_ChunkingCtx structure
*            fin: input elementary h264 file pointer
*
*  @param Out:
*            None
*
*  @return   void
*
*/

void Decode_H264ParserInit1 (H264_ParsingCtx *pc, void *fin)
{
	/* Initialize H.264 Parsing Context */
	pc->readBuf = (unsigned char *) malloc (READSIZE);
	if (!pc->readBuf)	{
		printf ("h264 Parser read buf memory allocation failed\n");
	}

	Decode_ChunkingCtxInit (&pc->ctx);

	pc->frameNo = 0;
	pc->frameSize = 0;
	pc->bytes = 0;
	pc->tmp = 0;
	pc->firstParse = 1;
	pc->chunkCnt = 1;
	pc->outBuf.ptr = NULL;
	pc->outBuf.bufsize = 0;
	pc->outBuf.bufused = 0;
	pc->fUseDemux = 0;

	pc->fEoS = 0;
	pc->fp = fin;
	pc->ParserBase.nUiCmd = STRM_CMD_NONE;
	pc->ParserBase.fStreaming = 1;
}

void Decode_H264ParserInit2 (H264_ParsingCtx *pc, ConnCtxT *pConn)
{
	/* Initialize H.264 Parsing Context */
	pc->readBuf = (unsigned char *) malloc (READSIZE);
	if (!pc->readBuf)	{
		printf ("h264 Parser read buf memory allocation failed\n");
	}

	Decode_ChunkingCtxInit (&pc->ctx);

	pc->frameNo = 0;
	pc->frameSize = 0;
	pc->bytes = 0;
	pc->tmp = 0;
	pc->firstParse = 1;
	pc->chunkCnt = 1;
	pc->outBuf.ptr = NULL;
	pc->outBuf.bufsize = 0;
	pc->outBuf.bufused = 0;
	pc->fUseDemux = 1;

	pc->fEoS = 0;
	pc->ParserBase.nUiCmd = STRM_CMD_NONE;
	pc->ParserBase.fStreaming = 1;
	pc->pConn = pConn;
}

void Decode_H264ParserFlush(H264_ParsingCtx *pc)
{
	Decode_ChunkingCtxInit (&pc->ctx);

	pc->frameNo = 0;
	pc->frameSize = 0;
	pc->bytes = 0;
	pc->tmp = 0;
	pc->firstParse = 1;
	pc->chunkCnt = 1;
	//pc->outBuf.ptr = NULL;
	//pc->outBuf.bufsize = 0;
	pc->outBuf.bufused = 0;
	//pc->fUseDemux = 1;

	//pc->fEoS = 0;
	//pc->ParserBase.nUiCmd = STRM_CMD_NONE;
	//pc->ParserBase.fStreaming = 1;
}

void DumpH264FrameStat(AVChunk_Buf *pBuf)
{
	printf ("len=%d ",pBuf->bufused);
	if((pBuf->ptr[3] & 0x1F) == 7)
		DumpHex(pBuf->ptr, 128);
	else
		DumpHex(pBuf->ptr, 8);
}
/******************************************************************************\
*      Decode_DoChunking Function Declaration
\******************************************************************************/
/**
*
*  @brief    Does H.264 Frame Chunking.
*
*  @param in:
*            c: Pointer to H264_ChunkingCtx structure
*            opBufs: Pointer to AVChunk_Buf Structure
*            numOutBufs: Number of output buffers
*            inBuf: Pointer to AVChunk_Buf Structure
*            attrs: Additional attributes
*
*  @param Out:
*            None
*
*  @return   AVC_Err - AVC Error Code
*
*/

AVC_Err Decode_DoChunking (H264_ChunkingCtx *c, AVChunk_Buf *opBufs,
                           unsigned int numOutBufs, AVChunk_Buf *inBuf,
                           void *attrs)
{
	int i = 0, j, frame_end, sc_found, tmp, bytes;
	unsigned int w, z;
	unsigned char *inp;

	inp = &inBuf->ptr[inBuf->bufused];
	bytes = inBuf->bufsize - inBuf->bufused;
	//printf("%s @ %d size=%d\n",__FUNCTION__, __LINE__, bytes);
	// DumpHex(inp, 16);
BACK:
	if (H264_ST_INSIDE_PICTURE == c->state)	{
		tmp = i;
		sc_found = frame_end = 0;
		while (i < bytes) {
			z = c->workingWord << 8;
			w = c->fifthByte;
			c->workingWord = z | w;
			c->fifthByte = inp[i++];

			if (z == 0x100)	{
				w &= 0x1f;
				if (w == H264_NAL_ACCESS_UNIT_CODEDSLICE_CODE_FOR_NONIDR ||
					w == H264_NAL_ACCESS_UNIT_CODEDSLICE_CODE_FOR_IDR)		{
					/* check for MB number in slice header */
					if (c->fifthByte & 0x80){
						sc_found = frame_end = 1;
						break;
					}
				}                       /* if (w) */
				if (w == H264_PPS_START_CODE || w == H264_SPS_START_CODE)	{
					sc_found = frame_end = 1;
					break;
				}                       /* if (w) */
			}                         /* if (z) */
		}                           /* while (i) */
		j = i - tmp;

		/* minus 5 to remove next header that is already copied */
		if (frame_end){
			j -= 5;
		}

		if (j > (int32_t) (opBufs->bufsize - opBufs->bufused))	{
			/* output buffer is full, end the frame right here */
			AVCLOG (AVC_MOD_MAIN, AVC_LVL_TRACE_ALL, ("memcpy(%p,%d,%p,%d,%d)",
												opBufs->ptr,
												opBufs->bufused, inp, tmp,
												opBufs->bufsize -
												opBufs->bufused));
			memcpy (&opBufs->ptr[opBufs->bufused], &inp[tmp],
				opBufs->bufsize - opBufs->bufused);
			opBufs->bufused = opBufs->bufsize;
			c->state = H264_ST_LOOKING_FOR_ZERO_SLICE;
			frame_end = 1;
		} else if (j > 0)	{
			AVCLOG (AVC_MOD_MAIN, AVC_LVL_TRACE_ALL, ("memcpy(%p,%d,%p,%d,%d)",
													opBufs->ptr,
													opBufs->bufused, inp, tmp, j));
			memcpy (&opBufs->ptr[opBufs->bufused], &inp[tmp], j);
			opBufs->bufused += j;
		}   else   {
			opBufs->bufused += j;
		}

		if (frame_end)  {
			if (sc_found) {
				c->state = H264_ST_HOLDING_SC;
				// printf("FrameSize = %d\n", i);
			}
			inBuf->bufused += i;
			return AVC_SUCCESS;
		}
	}

	if (H264_ST_LOOKING_FOR_ZERO_SLICE == c->state)	{
		tmp = i;
		sc_found = 0;
		while (i < bytes){
			z = c->workingWord << 8;
			w = c->fifthByte;
			c->workingWord = z | w;
			c->fifthByte = inp[i++];

			if (z == 0x100)	{
				w &= 0x1f;
				if (w == H264_NAL_ACCESS_UNIT_CODEDSLICE_CODE_FOR_NONIDR ||
						w == H264_NAL_ACCESS_UNIT_CODEDSLICE_CODE_FOR_IDR)	{
					/* check for MB number in slice header */
					if (c->fifthByte & 0x80)	{
						sc_found = 1;
						break;
					} /* if (c) */
				} /* if (w) */
			} /* if (z) */
		} /* while (i) */
		j = i - tmp;

		if (j > (opBufs->bufsize - opBufs->bufused))	{
			/* output buffer is full, discard this data, go back to looking for seq
				start code */
			opBufs->bufused = 0;
			c->state = H264_ST_LOOKING_FOR_SPS;
		}  else if (j > 0)  {
			AVCLOG (AVC_MOD_MAIN, AVC_LVL_TRACE_ALL, ("memcpy(%p,%d,%p,%d,%d)",
												opBufs->ptr,
												opBufs->bufused, inp, tmp, j));
			memcpy (&opBufs->ptr[opBufs->bufused], &inp[tmp], j);
			opBufs->bufused += j;
		}

		if (sc_found)  {
			/* Set the attribute at rioWptr */
			c->state = H264_ST_INSIDE_PICTURE;
		}
	} /* if (c->state...) */

	if (H264_ST_STREAM_ERR == c->state)	{
		while (i < bytes)	{
			z = c->workingWord << 8;
			w = c->fifthByte;
			c->workingWord = z | w;
			c->fifthByte = inp[i++];

			if (z == 0x100)	{
				w &= 0x1f;
				if (w == H264_NAL_ACCESS_UNIT_CODEDSLICE_CODE_FOR_NONIDR ||
					w == H264_NAL_ACCESS_UNIT_CODEDSLICE_CODE_FOR_IDR)	{
					/* chueck for MB number in slice header */
					if (c->fifthByte & 0x80){
						c->state = H264_ST_HOLDING_SC;
						break;
					}
				}
				if (w == H264_PPS_START_CODE || w == H264_SPS_START_CODE)	{
					c->state = H264_ST_HOLDING_SC;
					break;
				}
			} /* if (z) */
		} /* while (i) */
	}

	if (H264_ST_LOOKING_FOR_SPS == c->state) {
		while (i < bytes)  {
			z = c->workingWord << 8;
			w = c->fifthByte;
			c->workingWord = z | w;
			c->fifthByte = inp[i++];

			if (z == 0x100)	{
				// printf("%d: %08x @ %d\n",chunkCnt, c->workingWord,
				// inBuf->bufused+i-6);

				w &= 0x1f;
				if (w == H264_SPS_START_CODE)	{
					DBG_LOG(DBGLVL_SETUP, ("SPS Found"));
					c->state = H264_ST_HOLDING_SC;
					break;
				} /* if (w) */
			} /* if (z) */
		} /* while (i) */
	}

	if (H264_ST_HOLDING_SC == c->state)	{
		w = c->workingWord;
		opBufs->bufused = 0;
		if (opBufs->bufsize < 5)
		{
			/* Means output buffer does not have space for 4 bytes, bad error */
			AVCLOG (AVC_MOD_MAIN, AVC_LVL_CRITICAL_ERROR, ("Bad error"));
		}
		/* Copy these 5 bytes into output */
		for (j = 0; j < 4; j++, w <<= 8)   {
			opBufs->ptr[opBufs->bufused + j] = ((w >> 24) & 0xFF);
		}
		opBufs->ptr[opBufs->bufused + j] = c->fifthByte;
		opBufs->bufused += 5;
		/* Copying frame start code done, now proceed to next state */
		w = c->workingWord & 0x1f;
		if (w == H264_PPS_START_CODE || w == H264_SPS_START_CODE)   {
			c->state = H264_ST_LOOKING_FOR_ZERO_SLICE;
		}  else   {
			c->state = H264_ST_INSIDE_PICTURE;
		}

		c->workingWord = H264_WORKING_WORD_INIT;
		c->fifthByte = 0xFF;
	}

	if (i < bytes)	{
		goto BACK;
	}

	inBuf->bufused += i;
	return AVC_ERR_INSUFFICIENT_INPUT;
}


/******************************************************************************\
*      Decode_GetNextH264FrameSize Function Declaration
\******************************************************************************/
/**
*
*  @brief    Gets the size of the next frame, It is doing chunking of h264 
*            elementary bitstream and providing frames to OMX component.
*
*  @param in:
*            pc: Pointer to H264_ParsingCtx structure
*
*  @param Out:
*            None
*
*  @return   uint32_t - Frame Size
*
*/

unsigned int Decode_GetNextH264FrameSize (H264_ParsingCtx *pc)
{
	FILE *fp = pc->fp;
	unsigned char *readBuf = pc->readBuf;
	H264_ChunkingCtx *ctx = &pc->ctx;
	AVChunk_Buf *inBuf = &pc->inBuf;
	AVChunk_Buf *outBuf = &pc->outBuf;
	int fEoF = 0;
	unsigned char termCond = 1;
	int fDisCont = 0;
	pc->fDisCont = 0;
	if (pc->firstParse == 1)
		termCond = 0;
	
	outBuf->nTimeStamp = pc->crnt_vid_pts;

	// Check for End of stream
	if(pc->fUseDemux) {
		if(pc->fEoS) {
			fEoF = 1;
		}
	} else {
		if(feof (fp)) {
			fEoF = 1;
		}
	}
	while ((!fEoF && pc->ParserBase.nUiCmd != STRM_CMD_STOP) ||
		((((pc->firstParse == 0) && (pc->bytes != 0))
			&& (pc->bytes <= READSIZE) && (pc->tmp <= pc->bytes))))
	{
		if (pc->firstParse == 1) {
			if(pc->fUseDemux) {
				pc->bytes = ParserReadData(&pc->ParserBase, pc->pConn, readBuf, READSIZE,  &pc->crnt_vid_pts, &pc->fEoS, &fDisCont);
			} else {
				pc->bytes = fread (readBuf, 1, READSIZE, fp);
				printf("H264 Parser read @ %d size=%d\n",__LINE__, pc->bytes);
			}
			if (!pc->bytes || pc->fEoS) {
				return 0;
			}
			inBuf->ptr = readBuf;
			pc->tmp = 0;
			pc->firstParse = 0;
		}   else {
			if (pc->bytes <= pc->tmp) {
				if(pc->fUseDemux) {
					pc->bytes = ParserReadData(&pc->ParserBase, pc->pConn, readBuf, READSIZE,  &pc->crnt_vid_pts, &pc->fEoS, &fDisCont);
				} else {
					pc->bytes = fread (readBuf, 1, READSIZE, fp);
					printf("H264 Parser read @ %d size=%d\n",__LINE__, pc->bytes);
				}
				if (!pc->bytes || pc->fEoS)	{
					return 0;
				}
				inBuf->ptr = readBuf;
				pc->tmp = 0;
			}
		}
		if(fDisCont){
			DBG_LOG(DBGLVL_SETUP, ("Discontinuity Flushing!!!"));
			pc->fDisCont = fDisCont;
			Decode_H264ParserFlush(pc);
		}
		while (pc->bytes > pc->tmp) {
			inBuf->bufsize = ((pc->bytes - pc->tmp) > 184) ? 184 : (pc->bytes - pc->tmp);
			inBuf->bufused = 0;

			while (inBuf->bufsize != inBuf->bufused) {
				if (AVC_SUCCESS == Decode_DoChunking (ctx, outBuf, 1, inBuf, NULL)) {
					pc->frameNo = pc->chunkCnt++;
					pc->frameSize = outBuf->bufused;
					pc->tmp += inBuf->bufused;
					inBuf->ptr += inBuf->bufused;
					if(fEoF && pc->bytes <= pc->tmp) {
						printf("H264 Parser EoF(1)\n");
						outBuf->nFlags |= OMX_BUFFERFLAG_EOS;
					}
					//printf("framesize=%d\n", pc->frameSize);
					//DumpH264FrameStat(outBuf);
					return pc->frameSize;
				}
			} /* while (inBuf->bufSize */
			pc->tmp += inBuf->bufused;
			inBuf->ptr += inBuf->bufused;
		} /* while (pc->bytes) */
	} /* while ((!feof (fp)...)) */
NoData:
	printf("H264 Parser EoF(2)\n");
	outBuf->nFlags |= OMX_BUFFERFLAG_EOS;
	return 0;
}

unsigned int Decode_AbortGetNextH264Frame(H264_ParsingCtx *pc)
{
	pc->ParserBase.nUiCmd = STRM_CMD_STOP;
}

void Decode_Mpeg4ParserInit (MPEG4_ParsingCtx *ctx, void *fin) 
{
  /* Initialize MPEG4 Parsing Context */
  ctx->working_frame = (unsigned char *) 
                         malloc (READSIZE);
  if (!ctx->working_frame)
  {
    printf ("mpeg4 Parser read buf memory allocation failed\n");
  }

  ctx->savedbuff = (unsigned char *)
                     malloc (READSIZE);
  if (!ctx->savedbuff)
  {
    printf ("mpeg4 Parser saved buf memory allocation failed\n");
  }
 
  fread (ctx->savedbuff, 1, 4, fin);
  ctx->idx = 4;
  ctx->flagSEQ = 0;
  ctx->fp = fin;
  ctx->count = 0;
}

void Decode_Mpeg2ParserInit1 (MPEG2_ParsingCtx *ctx, void *fin) 
{
	/* Initialize MPEG2 Parsing Context */
	ctx->working_frame = (unsigned char *)malloc (MPEG2_FRAME_MAX_SIZE);
	if (!ctx->working_frame) {
		printf ("mpeg2 Parser read buf memory allocation failed\n");
	}

	ctx->savedbuff = (unsigned char *) malloc ((MPEG2_FRAME_MAX_SIZE/64));
	if (!ctx->savedbuff)  {
		printf ("mpeg2 Parser saved buf memory allocation failed\n");
	}
  
	fread (ctx->savedbuff, 1, 4, fin);
	ctx->idx = 4;
	ctx->flagSEQ = 1;
	ctx->fp = fin;
	ctx->ParserBase.nUiCmd = STRM_CMD_NONE;
	ctx->nCrntFrameTimeStamp = 0;
	ctx->nCrntFrameFlags = 0;
}

void Decode_Mpeg2ParserInit2 (MPEG2_ParsingCtx *ctx, ConnCtxT *pConn)
{
	/* Initialize H.264 Parsing Context */
	ctx->working_frame = (unsigned char *)malloc (MPEG2_FRAME_MAX_SIZE);
	if (!ctx->working_frame) {
		printf ("mpeg2 Parser read buf memory allocation failed\n");
	}

	ctx->savedbuff = (unsigned char *) malloc ((MPEG2_FRAME_MAX_SIZE/64));
	if (!ctx->savedbuff)  {
		printf ("mpeg2 Parser saved buf memory allocation failed\n");
	}

#if 0
	Decode_ChunkingCtxInit (&pc->ctx);

	pc->frameNo = 0;
	pc->frameSize = 0;
	pc->bytes = 0;
	pc->tmp = 0;
	pc->firstParse = 1;
	pc->chunkCnt = 1;
	pc->outBuf.ptr = NULL;
	pc->outBuf.bufsize = 0;
	pc->outBuf.bufused = 0;
#endif
	ctx->fUseDemux = 1;
	ctx->fEoS = 0;

	ctx->idx = 4;
	ctx->flagSEQ = 1;

	ctx->pConn = pConn;
	ctx->ParserBase.nUiCmd = STRM_CMD_NONE;
	ctx->nCrntFrameTimeStamp = 0;
	ctx->nCrntFrameFlags = 0;

}


unsigned int Decode_GetNextMpeg2FrameSize (MPEG2_ParsingCtx *ctx)
{
	FILE *fin = ctx->fp;
	int i, start_i, end_i, frame_len;
	unsigned char *buff;
	unsigned char *buff_in = ctx->buff_in;
	int slice_cnt = 0;
	int frame_len_total = 0;
	int fEoF = 0;
	int nPktLen = 0;
	int ret = 0;
	ctx->fDisCont = 0;
	DBG_LOG(DBGLVL_FRAME, ("Enter"));
	// Check for End of stream
	if(ctx->fEoS || ctx->ParserBase.nUiCmd == STRM_CMD_STOP) {
		goto Exit;
	}

	// Save the time stamp of the frame about to be processed.
	// ctx->crnt_vid_pts will be updated with next frame
	ctx->nCrntFrameTimeStamp = ctx->crnt_vid_pts;

	for (slice_cnt = 0; ; )	{
		buff = ctx->working_frame;
		if(fEoF)
			goto Exit;
		if (ctx->idx) {
			memcpy (buff, ctx->savedbuff, ctx->idx);
		}

		start_i = 4;      // When we come here, the first 4 bytes should be start code
		frame_len = 0;

		for (;;) {

			nPktLen = ParserReadData(&ctx->ParserBase, ctx->pConn, &buff[ctx->idx], 188/*CHUNK_TO_READ*/, &ctx->crnt_vid_pts, &ctx->fEoS, &ctx->fDisCont);
			end_i   = ctx->idx + nPktLen;
			if(ctx->fEoS || ctx->ParserBase.nUiCmd == STRM_CMD_STOP) {
				ctx->nCrntFrameFlags |= OMX_BUFFERFLAG_EOS;
				goto Exit;
			}
			
			// Detect next frame Start
			for (i = start_i; i < end_i-3; i++) {
				if (ctx->flagSEQ == 0)	{
					if ((buff[i] == 0x00) && (buff[i + 1] == 0x00) && (buff[i + 2] == 0x01) && (buff[i + 3] == 0xb3)) {
						frame_len = i;
						ctx->flagSEQ = 1;
						break;
					} else if ((buff[i] == 0x00) && (buff[i + 1] == 0x00) && (buff[i + 2] == 0x01) && (buff[i + 3] == 0x00)) {
						frame_len = i;
						ctx->flagSEQ = 0;
						break;
					}
				} /* if (ctx->...) */ 	else {
					if ((buff[i] == 0x00) && (buff[i + 1] == 0x00) && (buff[i + 2] == 0x01) && (buff[i + 3] == 0x00))	{ 
						/* First frame header will be skipped */
						ctx->flagSEQ = 0;
					}
				} /* if (ctx->...) else ...*/
			} /* for (i) */

			if (frame_len)	{
				ctx->idx = end_i - i; // reamining data leghth after currently identified frame
				memcpy (&ctx->savedbuff[0], &buff[i], ctx->idx);
				memset (&buff[i], 0, ctx->idx);
				memcpy (&buff_in[frame_len_total], ctx->working_frame, frame_len);
				frame_len_total += frame_len;
				slice_cnt ++;
				if (slice_cnt >= 1)	{
					//fflush (stdout); 
					ret = frame_len_total;
					goto Exit;
				} /* if (slice_cnt) */
			} /* if (frame_len) */	else {
				start_i = end_i-3;
				ctx->idx += nPktLen;
			} /* if (frame_len) else... */
		} /* for (;;) */
	} /* end of for slice_cnt */

Exit:
	DBG_LOG(DBGLVL_FRAME, ("Leave ret=%d", ret));
	return ret;
}

unsigned int Decode_AbortGetNextMpeg2Frame(MPEG2_ParsingCtx *pc)
{
	pc->ParserBase.nUiCmd = STRM_CMD_STOP;
}

unsigned int Decode_GetNextMpeg4FrameSize (MPEG4_ParsingCtx *ctx)
{
  FILE *fin = ctx->fp;
  int i, start_i, end_i, frame_len;
  unsigned char *buff;
  unsigned char *buff_in = ctx->buff_in;
  int slice_cnt = 0;
  int frame_len_total = 0;

  for (slice_cnt = 0; ; )
  {
    buff = ctx->working_frame;
    if (fin == NULL)
    {
       return 0;
    }

    if (ctx->idx)
    {
      memcpy (buff, ctx->savedbuff, ctx->idx);
    }

    start_i = 4;
    end_i   = ctx->idx + CHUNK_TO_READ;
    frame_len = 0;

    for (;;)
    {
      if (fread (&buff[ctx->idx], 1, CHUNK_TO_READ, fin) != CHUNK_TO_READ)
      {
        printf ("\nNo more to read\n");
        fflush (stdout);
            return 0;
      }

      for (i = start_i; i < end_i; i ++)
      {


        if ((buff[i] == 0x00) && (buff[i+1] == 0x00) && ((buff[i + 2]) == 0x01 ) && ((buff[i + 3]) == 0xb6 ))
        {
            if(ctx->count++ == 0)
                continue;
            frame_len = i;
            break;
             
        }

      } /* for (i) */

      if (frame_len)
      {
        ctx->idx = end_i - i;
        memcpy (&ctx->savedbuff[0], &buff[i], ctx->idx);
  
        memset (&buff[i], ctx->idx, 0);
        memcpy (&buff_in[frame_len_total], ctx->working_frame, frame_len);
        frame_len_total += frame_len;
        slice_cnt ++;
        if (slice_cnt >= 1)
        {
        
          return frame_len_total;
        } /* if (slice_cnt) */
      } /* if (frame_len) */
      else
      {
        start_i = end_i;
        end_i  += CHUNK_TO_READ;
        ctx->idx    += CHUNK_TO_READ;
      } /* if (frame_len) else... */
    } /* for (;;) */
  } /* end of for slice_cnt */

  return 0;

}
/******************************************************************************\
*      Decode_VDEC_Reset_Parser Function Declaration
\******************************************************************************/
/**
*
*  @brief    resets the parser to restart from begining of file.
*
*  @param in:
*            pc: Pointer to H264_ParsingCtx structure
*
*  @param Out:
*            None
*
*  @return   uint32_t - Frame Size
*
*/

void Decode_VDEC_Reset_Parser (void *parserPtr)
{
  H264_ParsingCtx *pc = (H264_ParsingCtx *) parserPtr;
  Decode_ChunkingCtxInit (&(pc->ctx));
  fseek (pc->fp, 0, SEEK_SET);
  pc->frameNo = 0;
  pc->frameSize = 0;
  pc->bytes = 0;
  pc->tmp = 0;
  pc->firstParse = 1;
  pc->chunkCnt = 1;
  pc->outBuf.ptr = NULL;
  pc->outBuf.bufsize = 0;
  pc->outBuf.bufused = 0;
}

void DumpParserStat(H264_ParsingCtx *pc)
{
	char szTmp[128] = {0};
	printf("H264 Parser Statistics:\n");
	
	Clock2HMSF(pc->crnt_vid_pts, szTmp,127);
	printf("Frame Count=%d, Current Video PTS=%s\n", pc->frameNo, szTmp);
	printf("pipeInEmptyBuff:\n");
}
