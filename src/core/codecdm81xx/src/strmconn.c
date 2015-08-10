#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>

#ifdef WIN32
#else
#include <sys/ioctl.h>
#include <linux/fcntl.h>
#endif

#include "strmconn.h"
#include "dbglog.h"

/* Circular buffer object */
typedef struct 
{
	int         size;   /* maximum number of elements           */
	int         start;  /* index of oldest element              */
	int         end;    /* index at which to write new element  */

	OMX_BUFFERHEADERTYPE_M   *elems;  /* vector of elements                   */
} CIRCULAR_BUFF_T;

static OMX_BUFFERHEADERTYPE_M *AllocateElemBuffers(int nBufferSize, int nBufferCount)
{
	int i;
	char *pBuffer;
	OMX_BUFFERHEADERTYPE_M *pElemBuff = (OMX_BUFFERHEADERTYPE_M *)malloc(sizeof(OMX_BUFFERHEADERTYPE_M) * nBufferCount);
	if(pElemBuff == NULL){
		ERROR("Failure allocating buffer");
	}
	memset(pElemBuff, 0x00, sizeof(OMX_BUFFERHEADERTYPE_M) * nBufferCount);
	pBuffer = (char *)malloc(nBufferSize * nBufferCount);
	if(pBuffer == NULL){
		ERROR("Failure allocating buffe");
	}

	for (i = 0; i < nBufferCount; i++) {
		OMX_BUFFERHEADERTYPE_M *pTmpElem = (char *)pElemBuff + i * sizeof(OMX_BUFFERHEADERTYPE_M);
		pTmpElem->pBuffer = pBuffer + i * nBufferSize;
		pTmpElem->nAllocLen = nBufferSize;
		pTmpElem->nFilledLen = 0;
	}
	return pElemBuff;
}

void FreeElemBuffers(OMX_BUFFERHEADERTYPE_M *pInBuff)
{
	free(pInBuff->pBuffer);
	free(pInBuff);
}



void cbInit(CIRCULAR_BUFF_T *dp, int nBufSize, int nCounBuf) 
{
	dp->elems = AllocateElemBuffers(nBufSize, nCounBuf + 1);
    dp->size  = nCounBuf + 1; /* include empty elem */
    dp->start = 0;
    dp->end   = 0;
}
 
void cbFree(CIRCULAR_BUFF_T *dp) 
{
	FreeElemBuffers(dp->elems);
}
 
int cbIsFull(CIRCULAR_BUFF_T *dp) 
{
    return (dp->end + 1) % dp->size == dp->start; 
}
 
int cbIsEmpty(CIRCULAR_BUFF_T *dp) 
{
	return dp->end == dp->start; 
}
 
/* Write an element, overwriting oldest element if buffer is full. App can
   choose to avoid the overwrite by checking dpIsFull(). */
void cbAdvanceWritePtr(CIRCULAR_BUFF_T *dp) 
{
    dp->end = (dp->end + 1) % dp->size;
}

OMX_BUFFERHEADERTYPE_M *cbGetCrntWriteSlot(CIRCULAR_BUFF_T *dp) 
{
    return &dp->elems[dp->end];
}

/* Read oldest element. App must ensure !dpIsEmpty() first. */
void cbAdvanceReadPtr(CIRCULAR_BUFF_T *dp) 
{
	dp->start = (dp->start + 1) % dp->size;
}

OMX_BUFFERHEADERTYPE_M *cbGetCrntReadSlot(CIRCULAR_BUFF_T *dp) 
{
	return &dp->elems[dp->start];
}

//=====================================================================================================================
//              circular buffer based implementation
//=====================================================================================================================

int dpRead (ConnCtxT *pConn, char *pData, int lenData, unsigned long *pulFlags, long long *pllPts)
{
	int ret = 0;
	CIRCULAR_BUFF_T *dp = (CIRCULAR_BUFF_T *)pConn->pPvtCtx;
	OMX_BUFFERHEADERTYPE_M *pBuffer = cbGetCrntReadSlot(dp);
	
	*pllPts = pBuffer->nTimeStamp;

	if(lenData >= pBuffer->nFilledLen && pBuffer->nFilledLen > 0) {
		memcpy(pData, pBuffer->pBuffer,pBuffer->nFilledLen);
		ret = pBuffer->nFilledLen;
	} else {
		DBG_LOG(DBGLVL_ERROR, ("Error: Insufficient buffer fill=%d buf=%d!!",pBuffer->nFilledLen, lenData));
	}
	*pulFlags = pBuffer->nFlags;
	cbAdvanceReadPtr(dp);
	return ret;
}

int dpWrite(ConnCtxT *pConn, char *pData, int dataLen, unsigned long ulFlags, long long llPts)
{
	CIRCULAR_BUFF_T *dp = (CIRCULAR_BUFF_T *)pConn->pPvtCtx;
	OMX_BUFFERHEADERTYPE_M *pBuffer = cbGetCrntWriteSlot(dp);

	if(pData && dataLen > 0) {
		memcpy(pBuffer->pBuffer, pData, dataLen);
	}
	pBuffer->nFilledLen = dataLen;
	pBuffer->nFlags = ulFlags;
	pBuffer->nTimeStamp = llPts;
	cbAdvanceWritePtr(dp);
}

int dpIsEmpty(struct _ConnCtxT *pConn)
{
	CIRCULAR_BUFF_T *dp = (CIRCULAR_BUFF_T *)pConn->pPvtCtx;
	return cbIsEmpty(dp);
}

int dpIsFull(struct _ConnCtxT *pConn)
{
	CIRCULAR_BUFF_T *dp = (CIRCULAR_BUFF_T *)pConn->pPvtCtx;
	return cbIsFull(dp);
}

int dpFlush(struct _ConnCtxT *pConn)
{
	return 0;
}
int dpBufferFullness (struct _ConnCtxT *pConn)
{
	int percentFull = 0;
	CIRCULAR_BUFF_T *dp = (CIRCULAR_BUFF_T *)pConn->pPvtCtx;
	percentFull = ((dp->end + dp->size - dp->start) % dp->size) * 100 / dp->size;
	return percentFull;
}

ConnCtxT *CreateStrmConn(int nBufferSize, int nBufferCount)
{
	ConnCtxT *pCtx = (ConnCtxT *)malloc(sizeof(ConnCtxT));
	memset(pCtx, sizeof(ConnCtxT), 0x00);
	pCtx->pPvtCtx = (CIRCULAR_BUFF_T *)malloc(sizeof(CIRCULAR_BUFF_T));

	cbInit((CIRCULAR_BUFF_T *)pCtx->pPvtCtx, nBufferSize,nBufferCount);
	pCtx->Read = dpRead;
	pCtx->Write = dpWrite;
	pCtx->IsEmpty = dpIsEmpty;
	pCtx->IsFull = dpIsFull;
	pCtx->Flush = dpFlush;
	pCtx->BufferFullness  = dpBufferFullness;

	return pCtx;
}

void DeleteStrmConn(ConnCtxT *pCtx)
{
	if(pCtx->pPvtCtx){
		cbFree((CIRCULAR_BUFF_T *)pCtx->pPvtCtx);
	}
	free(pCtx);
}
