#ifdef WIN32
#include <winsock2.h>
#include <fcntl.h>
#include <io.h>
#include <Ws2ipdef.h>
#define close	closesocket
#define snprintf	_snprintf
#define write	_write
#else
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <strings.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/un.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#include <assert.h>

#include "dbglog.h"

#include "udprx.h"
#include "oal.h"
typedef struct _UdpRxCtxT
{
	int                fMulticast;
	int                SockAddr;
	unsigned short     usRxPort;
	struct in_addr     McastAddr;
	int                mhSock;
	int                fStreaming;
	void               *pUpstreamConn;
	ConnCtxT		   *pConn;
	int					nUiCmd;
#ifdef WIN32
	HANDLE hThread;
#else
	pthread_t hThread;
#endif
	int                  fUseIpc;
	int                 nTotoalBytes;
	int                 nPrevTotoalBytes;
	int                 nPrevStatTimeMs;
	int                 nNoInDataTimeMs;
	int                 nNoOutDataTimeMs;
} UdpRxCtxT;


#define UDP_PROTOCOL "udp://"

int ParseUdpSrc(const char *pszUdpSrc, struct in_addr *addr, unsigned short *pusPort, int *fMcast)
{
	int addrFirstByte = 0;
	memset(addr, sizeof(struct in_addr), 0x00);
	*pusPort = 0;
	*fMcast = 0;


	if(strncmp(pszUdpSrc,UDP_PROTOCOL,strlen(UDP_PROTOCOL)) == 0){
		const char *pTmp = pszUdpSrc + strlen(UDP_PROTOCOL);
		const char *pIndx = strchr(pTmp,':');
		if(pIndx) {
			char szAddr[32] = {0};
			if(pIndx == pTmp) {
				// Unicast address supplied as udp://:<port>
				// Do nothing
				DBG_LOG(DBGLVL_SETUP, ("Address not specified."));
			} else {
				strncpy(szAddr, pTmp, pIndx- pTmp);
#ifdef WIN32
				{
					addr->S_un.S_addr = inet_addr(szAddr);
					if (addr->S_un.S_addr == -1){
						fprintf(stderr, "inet_addr failed\n");
						return -1;
					}
				}
#else
				{
					if(inet_aton(szAddr, addr) == 0){
						fprintf(stderr, "inet_aton failed\n");
						return -1;
					}
				}
#endif
			}
			*pusPort = atoi(pIndx+1);
		}
	}
#ifdef WIN32
	addrFirstByte = addr->S_un.S_un_b.s_b1;
#else
	addrFirstByte = addr->s_addr & 0x000000FF;
#endif
	DBG_LOG(DBGLVL_SETUP, ("addrFirstByte=%d addr=0x%x", addrFirstByte, addr->s_addr));
	if(addrFirstByte >= 224 && addrFirstByte <= 239){
		*fMcast = 1;
	} else {
		*fMcast = 0;
	}
	return 0;
}

int OpenIpcSocket(char *szSockName)
{
#ifdef WIN32
	return -1;
#else
	struct sockaddr_un ipc_sock_addr;
	int len;
	int res = 0;
	int sock;
	// Open IPC Socket
	DBG_LOG(DBGLVL_SETUP, ("Opening IPC Socket %s", szSockName));
	sock = socket(AF_UNIX, SOCK_DGRAM, 0);
	//sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0) {
		fprintf(stderr, "Failed to create IPC socket err=%d\n", sock);
		exit(1);
	}
	unlink(szSockName);
	ipc_sock_addr.sun_family = AF_UNIX;
	strcpy(ipc_sock_addr.sun_path, szSockName);

	len = sizeof(ipc_sock_addr.sun_family) + strlen(ipc_sock_addr.sun_path);
	if (bind(sock, &ipc_sock_addr, len) < 0) {
		fprintf(stderr, "Failed to bind\n");
		exit(1);
	}
	return sock;
#endif
}

int udprxOpen(StrmCompIf *pSrc, const char *szUdpSrc)
{
	int sock;										/* Socket descriptor */
	struct sockaddr_in local_sa;
	int res = 0;

	int windowSize = 1024*1024;
    int windowSizeLen = sizeof(int);
	int on = 1;
	UdpRxCtxT *pCtx = (UdpRxCtxT *)pSrc->pCtx;

	if(strcmp(szUdpSrc, TS_IPC_SOCKET) == 0) {
		sock = OpenIpcSocket(TS_IPC_SOCKET);
		pCtx->fUseIpc = 1;
	} else {

		res = ParseUdpSrc(szUdpSrc, &pCtx->McastAddr, &pCtx->usRxPort, &pCtx->fMulticast);
		if(res < 0) {
			DBG_LOG(DBGLVL_ERROR, ("Failed to parse %s", szUdpSrc));
			return -1;
		}
	#ifdef WIN32
		{
			WORD wVersionRequested;
			WSADATA wsaData;
			int err;
			wVersionRequested = MAKEWORD(2, 2);
			err = WSAStartup(wVersionRequested, &wsaData);
			if (err != 0) {
				fprintf(stderr, "WSAStartup failed with error: %d\n", err);
				return 1;
			}
		}
	#endif

		sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(sock == -1) {  
			DBG_LOG(DBGLVL_ERROR, ("Failed to create socket"));
			return -1;
		}
	

		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on)) < 0) {
			fprintf(stderr,"setsockopt(SO_REUSEADDR) failed");
			return -1;
		}


		if ( setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&windowSize, windowSizeLen) == -1 )  {
			DBG_LOG(DBGLVL_ERROR, ("setsockopt Failed"));
			return -1;
		}

		memset(&local_sa, 0, sizeof(local_sa));
		local_sa.sin_family = AF_INET;
		local_sa.sin_port = htons(pCtx->usRxPort);
		local_sa.sin_addr.s_addr = htonl(INADDR_ANY);

		if ( bind(sock, (struct sockaddr*)&local_sa, sizeof(struct sockaddr)) != 0 ) {
			fprintf(stderr, "bind Failed \n");
			return -1;
		}

		if(pCtx->fMulticast) {
			struct ip_mreq mreq;
			mreq.imr_multiaddr.s_addr = pCtx->McastAddr.s_addr;
			mreq.imr_interface.s_addr = htonl(INADDR_ANY);
			DBG_LOG(DBGLVL_SETUP, ("Joining Multicast group"));
			if ( setsockopt( sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&mreq, sizeof(mreq)) < 0){
				fprintf(stderr, "IP_ADD_MEMBERSHIP Failed \n");
				DBG_LOG(DBGLVL_ERROR, ("IP_ADD_MEMBERSHIP Failed"));
				return -1;
			}
		} else {
			DBG_LOG(DBGLVL_SETUP, ("Listening on unicast port %d", pCtx->usRxPort));
		}
	}
	pCtx->mhSock = sock;
	printf("sock =%d\n", sock);
	return 0;
}

void udprxClose(StrmCompIf *pSrc)
{
	UdpRxCtxT *pCtx = (UdpRxCtxT *)pSrc->pCtx;
	if(pCtx) {
		close(pCtx->mhSock);
	}
#ifdef WIN32
	 WSACleanup();
#endif

}

static void ShowStat(UdpRxCtxT *pCtx)
{
	int nCrtnTime = oalGetTimeMs();
	if(nCrtnTime > pCtx->nPrevStatTimeMs + 1000){
		if(pCtx->nNoOutDataTimeMs > 1000) {
			printf("UDP Reader Buffer overrun !!!\n");
			pCtx->nNoOutDataTimeMs  = 0;
		}

		if(pCtx->nNoInDataTimeMs > 1000) {
			printf("UDP Reader port=%d No input data !!!\n", pCtx->usRxPort, (pCtx->nTotoalBytes - pCtx->nPrevTotoalBytes) * 8);
			pCtx->nNoInDataTimeMs  = 0;
		} else {
			printf("UDP Reader port=%d Bitrate=%9d\n", pCtx->usRxPort, (pCtx->nTotoalBytes - pCtx->nPrevTotoalBytes) * 8);
			pCtx->nPrevStatTimeMs  = nCrtnTime;
			pCtx->nPrevTotoalBytes = pCtx->nTotoalBytes;
		}
	}
}

static void *thrdUdprxStreaming(void *pArg)
{
	fd_set rfds;
	struct timeval tv;
	int ret = -1;
	int	selectRet;
	int nMaxLen = 21 * 188;
	char *pData = (char *)malloc(nMaxLen);
	unsigned long ulFlags = 0;	
	UdpRxCtxT *pCtx = (UdpRxCtxT *)pArg;

	pCtx->fStreaming = 1;


	while(1) {
		ret = -1;
		while(pCtx->pConn->IsFull(pCtx->pConn) && pCtx->nUiCmd != STRM_CMD_STOP){
			//DBG_LOG(DBGLVL_WARN,("start=%d end=%d",pCtx->pConn->pdpCtx->start, pCtx->pConn->pdpCtx->end))
			OAL_TASK_SLEEP(1)
			pCtx->nNoOutDataTimeMs += 1;
			ShowStat(pCtx);
		}
		
		pCtx->nNoOutDataTimeMs = 0;

		if(pCtx->nUiCmd == STRM_CMD_STOP) {
			DBG_LOG(DBGLVL_SETUP, ("Setting EoS due to User Command."));
			ulFlags = OMX_BUFFERFLAG_EOS;
		} else {
			FD_ZERO(&rfds);
			FD_SET(pCtx->mhSock, &rfds);
			tv.tv_sec = 0; 
			tv.tv_usec = 100000;

			selectRet = select(pCtx->mhSock+1, &rfds, NULL, NULL, &tv);
			if(selectRet == 0) {
				pCtx->nNoInDataTimeMs += 100;
				ShowStat(pCtx);
				continue;
			} else if(selectRet == -1) {
				DBG_LOG(DBGLVL_ERROR,("Socket select error on %d. Exiting Streaming Task.", pCtx->usRxPort));
				break;
			}
			ret = recv(pCtx->mhSock, pData, nMaxLen, 0);
			if(ret > 0) {
				pCtx->nTotoalBytes += ret;
				pCtx->nNoInDataTimeMs = 0;
			}

			ShowStat(pCtx);
			//DBG_LOG(DBGLVL_TRACE, ("Bytes read %d\n", ret));
		}
		pCtx->pConn->Write(pCtx->pConn, pData, ret, ulFlags, 0);

		if(pCtx->nUiCmd == STRM_CMD_STOP){
			pCtx->fStreaming = 0;
			break;
		}

	}

	free(pData);
	fprintf(stderr,"%s:exiting\n", __FUNCTION__);
	return NULL;
}

int udprxSetOption(StrmCompIf *pSrc, int nCmd, char *pOptionData)
{
	UdpRxCtxT *pCtx = (UdpRxCtxT *)pSrc->pCtx;
	return 0;
}

int udprxStart(StrmCompIf *pSrc)
{
	int nThreadId;
	UdpRxCtxT *pCtx = (UdpRxCtxT *)pSrc->pCtx;
#ifdef WIN32
	pCtx->hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thrdUdprxStreaming, pCtx, 0, &nThreadId);
#else
	pthread_create(&pCtx->hThread, NULL, thrdUdprxStreaming, pCtx);
#endif
	(pCtx);
	return 0;
}

int udprxStop(StrmCompIf *pSrc)
{
	void *res;
	int nTimeOut = 1000000; // 100 milli sec
	UdpRxCtxT *pCtx = (UdpRxCtxT *)pSrc->pCtx;
	DBG_LOG(DBGLVL_TRACE, ("Enter fStreaming=%d", pCtx->fStreaming));

	if(pCtx->fStreaming) {
		pCtx->nUiCmd = STRM_CMD_STOP;
		DBG_LOG(DBGLVL_TRACE, ("fStreaming=%d", pCtx->fStreaming));
		while(pCtx->fStreaming && nTimeOut > 0) {
			nTimeOut -= 1000;
			OAL_TASK_SLEEP(1)
		}
		DBG_LOG(DBGLVL_TRACE, ("fStreaming=%d nTimeOut rem=%d", pCtx->fStreaming, nTimeOut));
	}


	if(pCtx->fStreaming) {
		// Close the file to force EoS
		DBG_LOG(DBGLVL_TRACE, ("Force EoS by closing source"));
		if (pCtx->mhSock != -1)  {
			close (pCtx->mhSock);
			pCtx->mhSock = -1;
		}

		nTimeOut = 1000000;
		while(pCtx->fStreaming && nTimeOut > 0) {
			nTimeOut -= 1000;
			OAL_TASK_SLEEP(1)
		}
	}

	if(pCtx->fStreaming) {
		DBG_LOG(DBGLVL_SETUP, ("[Thread did nit exit. Cancelling the thread..."));
#ifdef WIN32
#else
		pthread_cancel(pCtx->hThread);
#endif
		DBG_LOG(DBGLVL_SETUP, ("[pthread_join the thread: Begin"));
#ifdef WIN32
		WaitForSingleObject(pCtx->hThread,0);	return 0;
#else
		pthread_join (pCtx->hThread, (void **) &res);
#endif
		DBG_LOG(DBGLVL_SETUP, ("pthread_join the thread: Done]"));
	}

	return 0;
}

int udprxSetClkSrc(struct _StrmCompIf *pComp, void *pClk)
{
	return 0;
}
void * udprxGetClkSrc(struct _StrmCompIf *pComp)
{
	return NULL;
}

int udprxSetInputConn(UdpRxCtxT *pCtx)
{
	return 0;
}

int udprxSetOutputConn(StrmCompIf *pComp, int nCOnnNum, ConnCtxT *pConn)
{
	UdpRxCtxT *pCtx = pComp->pCtx;
	pCtx->pConn = pConn;
	return 0;
}

void udprxDelete(StrmCompIf *pSrc)
{
	free(pSrc->pCtx);
	free(pSrc);
}

StrmCompIf *udprxCreate()
{
	StrmCompIf *pComp = (StrmCompIf *)malloc(sizeof(StrmCompIf));
	pComp->pCtx =  (void *)malloc(sizeof(UdpRxCtxT));
	memset(pComp->pCtx, 0x00, sizeof(UdpRxCtxT));

	pComp->Open= udprxOpen;
	pComp->SetOption = udprxSetOption;
	pComp->SetInputConn= udprxSetInputConn;
	pComp->SetOutputConn= udprxSetOutputConn;
	pComp->SetClkSrc = udprxSetClkSrc;
	pComp->GetClkSrc = udprxGetClkSrc;
	pComp->Start = udprxStart;
	pComp->Stop = udprxStop;
	pComp->Close = udprxClose;
	pComp->Delete = udprxDelete;
	return pComp;
}

//=================================================================================================
#if 0
int main(int argc, char* argv[])
{
	StrmCompIf *pStrmSrc;
#ifdef WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        printf("WSAStartup failed with error: %d\n", err);
        return 1;
    }
#endif
	pStrmSrc = udprxCreate();
	if(pStrmSrc->Open(pStrmSrc, "udp://224.1.1.1:1234") == 0)	{
		pStrmSrc->Start(pStrmSrc);
		while(1) {
			char szCmd[32];
			scanf("%s",szCmd);
			if (strcmp(szCmd,"s") == 0)
				break;
		}
		pStrmSrc->Stop(pStrmSrc);
		pStrmSrc->Close(pStrmSrc);
	}
	return 0;
}

#endif
