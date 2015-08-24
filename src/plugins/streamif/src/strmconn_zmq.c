#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>

#ifdef WIN32
#include <zmq.h>
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/fcntl.h>
#include <sys/un.h>
#include <errno.h>
#endif


#include "strmconn_zmq.h"
#include "dbglog.h"
#define MAX_IPC_SRV_NAME 128

void               *g_pZmqContext = NULL;

typedef struct _ZMQ_CONN_PVT_CTX_T
{
	char               *m_pBuffer;
	int                m_nBuffLen;
	void               *m_pZmqSocket;
} ZMQ_CONN_PVT_CTX_T;

static  char *msgGetBuffer(ZMQ_CONN_PVT_CTX_T *pCtx)
{
	return pCtx->m_pBuffer;
}

static  OMX_BUFFERHEADERTYPE_M *msgGetHdr(char *pBuffer)
{
	return (OMX_BUFFERHEADERTYPE_M *)pBuffer;
}

static char *msgGetDataPtr(char *pBuffer)
{
	return (pBuffer + sizeof(OMX_BUFFERHEADERTYPE_M ));
}

static int Read (ConnCtxT *pConn, char *pData, int lenData, unsigned long *pulFlags, long long *pllPts)
{
	int nDataLen = 0;
	int ret = 0;
	ZMQ_CONN_PVT_CTX_T *pCtx = (ZMQ_CONN_PVT_CTX_T *)pConn->pPvtCtx;
	char *pBuffer = msgGetBuffer(pCtx);
	OMX_BUFFERHEADERTYPE_M *pHdr = msgGetHdr(pBuffer);
	char *pDataPtr = msgGetDataPtr(pBuffer);

	ret = zmq_recv (pCtx->m_pZmqSocket, pBuffer, pCtx->m_nBuffLen, 0);

	if(ret > 0) {
		*pllPts = pHdr->nTimeStamp;
		*pulFlags = pHdr->nFlags;
		nDataLen = pHdr->nFilledLen;
		if(lenData >= nDataLen ) {
			memcpy(pData, pDataPtr, pHdr->nFilledLen);
		} else {
			DBG_LOG(DBGLVL_ERROR, ("ZMQ: Insufficient buffer fill=%d buf=%d!!",pHdr->nFilledLen, lenData));
		}
	} else {
		DBG_LOG(DBGLVL_ERROR, ("ZMQ Error: !!"));
	}

	return nDataLen;
}
static int Write(ConnCtxT *pConn, char *pData, int dataLen, unsigned long ulFlags, long long llPts)
{
	int res=0;
	ZMQ_CONN_PVT_CTX_T *pPvtCtx = (ZMQ_CONN_PVT_CTX_T *)pConn->pPvtCtx;
	char  *pBuffer = msgGetBuffer(pPvtCtx);
	OMX_BUFFERHEADERTYPE_M *pBufferHdr = msgGetHdr(pBuffer);
	char  *pDataPtr = msgGetDataPtr(pBuffer);


	pBufferHdr->nFilledLen = dataLen;
	pBufferHdr->nFlags = ulFlags;
	pBufferHdr->nTimeStamp = llPts;

	if(pData && dataLen > 0) {
		memcpy(pDataPtr, pData, dataLen);
	}

	zmq_send (pPvtCtx->m_pZmqSocket, pBuffer, dataLen + sizeof(OMX_BUFFERHEADERTYPE_M), 0);
	return dataLen;
}


static int IsEmpty (struct _ConnCtxT *pConn)
{
#if 1
	int rc;
	ZMQ_CONN_PVT_CTX_T *pPvtCtx = (ZMQ_CONN_PVT_CTX_T *)pConn->pPvtCtx;
	zmq_pollitem_t items [1];
	/* First item refers to ØMQ socket 'socket' */
	items[0].socket = pPvtCtx->m_pZmqSocket;
	items[0].events = ZMQ_POLLIN;
	/* Second item refers to standard socket 'fd' */
	//items[1].socket = NULL;
	//items[1].fd = fd;
	//items[1].events = ZMQ_POLLIN;
	/* Poll for events indefinitely */
	rc = zmq_poll (items, 1, 1 * 1000);
	return (rc == 0);
#else
	return 0;
#endif
}

static int IsFull (struct _ConnCtxT *pConn)
{
	return 0;
}

static int Flush (struct _ConnCtxT *pConn)
{
	return 0;
}

static int BufferFullness (struct _ConnCtxT *pConn)
{
	return 0;
}


static int InitPvtCtx(ZMQ_CONN_PVT_CTX_T *pCtx, int fServer, const char *pszSockName, int nMaxBufferSize)
{
	int rc;
	pCtx->m_pBuffer = (char *)malloc(sizeof(OMX_BUFFERHEADERTYPE_M) + nMaxBufferSize);
	pCtx->m_nBuffLen = sizeof(OMX_BUFFERHEADERTYPE_M) + nMaxBufferSize;

	if(g_pZmqContext == NULL) {
		g_pZmqContext = zmq_ctx_new ();
	}
	
	if(fServer) {
		pCtx->m_pZmqSocket = zmq_socket (g_pZmqContext, ZMQ_PUSH);
		rc = zmq_bind (pCtx->m_pZmqSocket, pszSockName); //"tcp://*:5555"
	} else {
		pCtx->m_pZmqSocket = zmq_socket (g_pZmqContext, ZMQ_PULL);
		rc = zmq_connect (pCtx->m_pZmqSocket, pszSockName);
	}
}

static int DeinitPvtCtx(ZMQ_CONN_PVT_CTX_T *pCtx)
{
	if(pCtx->m_pZmqSocket)
		zmq_close(pCtx->m_pZmqSocket);

	free(pCtx->m_pBuffer);
}

static ConnCtxT *CreateIpcStrmConnBase(int nMaxBufferSize)
{
	ConnCtxT *pCtx = (ConnCtxT *)malloc(sizeof(ConnCtxT));
	memset(pCtx, sizeof(ConnCtxT), 0x00);
	pCtx->pPvtCtx = malloc(sizeof(ZMQ_CONN_PVT_CTX_T));

	pCtx->Read = Read;
	pCtx->Write = Write;
	pCtx->IsEmpty = IsEmpty;
	pCtx->IsFull = IsFull;
	pCtx->Flush = Flush;
	pCtx->BufferFullness  = BufferFullness;
	return pCtx;
}

ConnCtxT *CreateZmqStrmConn(int fServer, const char *pszSockName, int nMaxBufferSize)
{
	ConnCtxT *pCtx = CreateIpcStrmConnBase(nMaxBufferSize);
	InitPvtCtx((ZMQ_CONN_PVT_CTX_T *)pCtx->pPvtCtx, fServer, pszSockName, nMaxBufferSize);
	return pCtx;
}


void DeleteZmqStrmConn(ConnCtxT *pCtx)
{
	if(pCtx->pPvtCtx){
		DeinitPvtCtx((ZMQ_CONN_PVT_CTX_T *)pCtx->pPvtCtx);
	}
	free(pCtx);
}
