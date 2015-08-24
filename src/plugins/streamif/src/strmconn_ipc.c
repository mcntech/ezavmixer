#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/fcntl.h>
#include <sys/un.h>
#include <errno.h>
#endif


#include "strmconn_ipc.h"
#include "dbglog.h"
#define MAX_IPC_SRV_NAME 128

typedef struct _IPC_CONN_PVT_CTX_T
{
	char               *m_pBuffer;
	int                m_nBuffLen;
	int                m_hIpcSock;
	char               m_pszPeerName[MAX_IPC_SRV_NAME];
} IPC_CONN_PVT_CTX_T;

char *msgGetBuffer(IPC_CONN_PVT_CTX_T *pCtx)
{
	return pCtx->m_pBuffer;
}

OMX_BUFFERHEADERTYPE_M *msgGetHdr(char *pBuffer)
{
	return (OMX_BUFFERHEADERTYPE_M *)pBuffer;
}

char *msgGetDataPtr(char *pBuffer)
{
	return (pBuffer + sizeof(OMX_BUFFERHEADERTYPE_M ));
}

static int OpenIpcSocket(const char *szSockName)
{
	int len;
	int sock = -1;
#ifdef WIN32
	struct sockaddr_in  ipc_sock_addr;

	// Open IPC Socket
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	unlink(szSockName);
	ipc_sock_addr.sin_family = AF_INET;
	ipc_sock_addr.sin_port =  htons((unsigned short)atoi(szSockName));
	ipc_sock_addr.sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);

	len =  sizeof(struct sockaddr);
	if (bind(sock, (struct sockaddr *)&ipc_sock_addr, len) < 0) {
		DBG_LOG(DBGLVL_ERROR,("Failed to bind IPC socket %s", szSockName));
	}
#else
	struct sockaddr_un ipc_sock_addr;

	// Open IPC Socket
	sock = socket(AF_UNIX, SOCK_DGRAM, 0);
	unlink(szSockName);
	ipc_sock_addr.sun_family = AF_UNIX;
	strcpy(ipc_sock_addr.sun_path, szSockName);

	len = sizeof(ipc_sock_addr.sun_family) + strlen(ipc_sock_addr.sun_path);
	if (bind(sock, &ipc_sock_addr, len) < 0) {
		DBG_LOG(DBGLVL_ERROR,("Failed to bind IPC socket %s", szSockName));
	}
#endif
	// Socket Options
	{
		int windowSize = 1024*1024;
		int windowSizeLen = sizeof(int);

		if ( setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&windowSize, windowSizeLen) == -1 )  {
			fprintf(stderr, "setsockopt Failed");
			return -1;
		}
		if ( setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&windowSize, windowSizeLen) == -1 )  {
			fprintf(stderr, "setsockopt Failed");
			return -1;
		}
	}

	return sock;
}


static int Read (ConnCtxT *pConn, char *pData, int lenData, unsigned long *pulFlags, long long *pllPts)
{
	int nDataLen = 0;
	int ret = 0;
	IPC_CONN_PVT_CTX_T *pCtx = (IPC_CONN_PVT_CTX_T *)pConn->pPvtCtx;
	char *pBuffer = msgGetBuffer(pCtx);
	OMX_BUFFERHEADERTYPE_M *pHdr = msgGetHdr(pBuffer);
	char *pDataPtr = msgGetDataPtr(pBuffer);

	ret = recv(pCtx->m_hIpcSock, pBuffer, pCtx->m_nBuffLen, 0);
	if(ret > 0) {
		*pllPts = pHdr->nTimeStamp;
		*pulFlags = pHdr->nFlags;
		nDataLen = pHdr->nFilledLen;
		if(lenData >= nDataLen ) {
			memcpy(pData, pDataPtr, pHdr->nFilledLen);
		} else {
			DBG_LOG(DBGLVL_ERROR, ("Error: Insufficient buffer fill=%d buf=%d!!",pHdr->nFilledLen, lenData));
		}
	} else {
		DBG_LOG(DBGLVL_ERROR, ("Socket Error: !!"));
	}

	return nDataLen;
}
#ifdef WI32
int dpWrite(ConnCtxT *pConn, char *pData, int dataLen, unsigned long ulFlags, long long llPts)
{
	// TODO
	return -1;
}
#else
static int Write(ConnCtxT *pConn, char *pData, int dataLen, unsigned long ulFlags, long long llPts)
{
	int res=0;
	int addr_len = 0;
#ifdef WIN32
	struct sockaddr_in  ipc_sock_addr;
	IPC_CONN_PVT_CTX_T *pPvtCtx = (IPC_CONN_PVT_CTX_T *)pConn->pPvtCtx;
	char  *pBuffer = msgGetBuffer(pPvtCtx);
	OMX_BUFFERHEADERTYPE_M *pBufferHdr = msgGetHdr(pBuffer);
	char  *pDataPtr = msgGetDataPtr(pBuffer);


	pBufferHdr->nFilledLen = dataLen;
	pBufferHdr->nFlags = ulFlags;
	pBufferHdr->nTimeStamp = llPts;

	if(pData && dataLen > 0) {
		memcpy(pDataPtr, pData, dataLen);
	}

	ipc_sock_addr.sin_family = AF_INET;
	ipc_sock_addr.sin_addr.S_un.S_addr =  htonl(INADDR_LOOPBACK);
	ipc_sock_addr.sin_port =  htons((unsigned short)atoi( pPvtCtx->m_pszPeerName));
	addr_len =  sizeof(struct sockaddr);

	res = sendto(pPvtCtx->m_hIpcSock, pBuffer, dataLen + sizeof(OMX_BUFFERHEADERTYPE_M), 0, (struct sockaddr *)&ipc_sock_addr, addr_len);
	if(res <= 0){
		fprintf(stderr,"%s:%s:Failed to %s send %d. err=0x%d\n", __FILE__, __FUNCTION__, pPvtCtx->m_pszPeerName, dataLen, errno);
	}

#else
	struct sockaddr_un ipc_sock_addr;
	IPC_CONN_PVT_CTX_T *pPvtCtx = (IPC_CONN_PVT_CTX_T *)pConn->pPvtCtx;
	char  *pBuffer = msgGetBuffer(pPvtCtx);
	OMX_BUFFERHEADERTYPE_M *pBufferHdr = msgGetHdr(pBuffer);
	char  *pDataPtr = msgGetDataPtr(pBuffer);


	pBufferHdr->nFilledLen = dataLen;
	pBufferHdr->nFlags = ulFlags;
	pBufferHdr->nTimeStamp = llPts;

	if(pData && dataLen > 0) {
		memcpy(pDataPtr, pData, dataLen);
	}

	ipc_sock_addr.sun_family = AF_UNIX;
	strcpy(ipc_sock_addr.sun_path, pPvtCtx->m_pszPeerName);
	addr_len = sizeof(ipc_sock_addr.sun_family) + strlen(ipc_sock_addr.sun_path);

	res = sendto(pPvtCtx->m_hIpcSock, pBuffer, dataLen + sizeof(OMX_BUFFERHEADERTYPE_M), 0, &ipc_sock_addr, addr_len);
	if(res <= 0){
		fprintf(stderr,"%s:%s:Failed to %s send %d. err=0x%d\n", __FILE__, __FUNCTION__, pPvtCtx->m_pszPeerName, dataLen, errno);
	}
#endif
	return dataLen;
}
#endif

static int IsEmpty (struct _ConnCtxT *pConn)
{
	IPC_CONN_PVT_CTX_T *pCtx = (IPC_CONN_PVT_CTX_T *)pConn->pPvtCtx;
	fd_set         rfds;
	struct timeval tv;
	int	selectRet;
	FD_ZERO(&rfds);
	FD_SET(pCtx->m_hIpcSock, &rfds);
	tv.tv_sec = 0; 
	tv.tv_usec = 100000;
	selectRet = select(pCtx->m_hIpcSock + 1, &rfds, NULL, NULL, &tv);
	return (selectRet == 0);
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


int InitPvtCtx(IPC_CONN_PVT_CTX_T *pCtx, const char *pszLocalName, const char *pszPeerName, int nMaxBufferSize)
{
	pCtx->m_pBuffer = (char *)malloc(sizeof(OMX_BUFFERHEADERTYPE_M) + nMaxBufferSize);
	pCtx->m_nBuffLen = sizeof(OMX_BUFFERHEADERTYPE_M) + nMaxBufferSize;
	pCtx->m_hIpcSock = OpenIpcSocket(pszLocalName);
	strncpy(pCtx->m_pszPeerName, pszPeerName, 128);
}

int DeinitPvtCtx(IPC_CONN_PVT_CTX_T *pCtx)
{
	if(pCtx->m_hIpcSock != -1){
#ifdef WIN32
		closesocket(pCtx->m_hIpcSock);
#else
		close(pCtx->m_hIpcSock);
#endif
	}
	free(pCtx->m_pBuffer);
}

static ConnCtxT *CreateIpcStrmConnBase(int nMaxBufferSize)
{
	ConnCtxT *pCtx = (ConnCtxT *)malloc(sizeof(ConnCtxT));
	memset(pCtx, sizeof(ConnCtxT), 0x00);
	pCtx->pPvtCtx = malloc(sizeof(IPC_CONN_PVT_CTX_T));

	pCtx->Read = Read;
	pCtx->Write = Write;
	pCtx->IsEmpty = IsEmpty;
	pCtx->IsFull = IsFull;
	pCtx->Flush = Flush;
	pCtx->BufferFullness  = BufferFullness;
	return pCtx;
}

ConnCtxT *CreateIpcStrmConn(const char *pszLocalName, const char *pszPeerName, int nMaxBufferSize)
{
	ConnCtxT *pCtx = CreateIpcStrmConnBase(nMaxBufferSize);
	InitPvtCtx((IPC_CONN_PVT_CTX_T *)pCtx->pPvtCtx, pszLocalName, pszPeerName, nMaxBufferSize);
	return pCtx;
}


void DeleteIpcStrmConn(ConnCtxT *pCtx)
{

	if(pCtx->pPvtCtx){
		DeinitPvtCtx((IPC_CONN_PVT_CTX_T *)pCtx->pPvtCtx);
	}
	free(pCtx);
}
