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
#endif
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include "udptx.h"

#define SOCKET_ERROR   -1
#define DEFAULT_READ_TIMEOUT	10		/* Seconds to wait before giving up


#define CHECK_ALLOC(x) if(x==NULL) {fprintf(stderr,"malloc failed at %s %d, exiting..\n", __FUNCTION__, __LINE__); goto Exit;}

/* Globals */
static int timeout = DEFAULT_READ_TIMEOUT;

int CUdpTx::CreateSession()
{
		int sock;										/* Socket descriptor */
		struct sockaddr_in dest_sa;							/* Socket address */
		struct sockaddr_in local_sa;
		int ret;

		sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(sock == -1) {  
			return -1; 
		}
		int on = 1;
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&on, sizeof(on)) < 0) {
			fprintf(stderr,"setsockopt(SO_REUSEADDR) failed");
			return -1;
		}

        int windowSize = 1024*1024;
        int windowSizeLen = sizeof(int);
        if ( setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&windowSize, windowSizeLen) == SOCKET_ERROR )  {
            int err = -1;
        }

		memset(&local_sa, 0, sizeof(local_sa));
		local_sa.sin_family = AF_INET;
		local_sa.sin_port = htons(mLocalRtpPort);
		local_sa.sin_addr.s_addr = htonl(INADDR_ANY);

		if ( bind(sock, (struct sockaddr*)&local_sa, sizeof(struct sockaddr)) != 0 ) {
			return -1; 
		}

		if(mMulticast) {
		}

#ifdef EN_USE_PEER_PORT
		/* Copy host address from hostent to (server) socket address */
		memcpy((char *)&dest_sa.sin_addr, (char *)pDestAddr, sizeof(in_addr));

		dest_sa.sin_port = htons(usRtpDestPort);      	/* Put portnum into sockaddr */
		dest_sa.sin_family = AF_INET;			/* Set service sin_family to PF_INET */
		ret = connect(sock, (sockaddr *)&dest_sa, sizeof(dest_sa));
		if(ret == -1) {  
			perror("connect");
			return -1; 
		}
#endif
		m_hSock = sock;
		return 0;
	}


int CUdpTx::Write(char *pData, int lLen)
{
	fd_set rfds;
	struct timeval tv;
	int ret = -1;
	int	selectRet;
	int bytesWritten= 0;
	/* Begin reading the body of the file */

	struct sockaddr_in peerAddr;
	peerAddr.sin_addr = m_SockAddr;
	peerAddr.sin_port = htons(mRemoteRtpPort);
	peerAddr.sin_family = PF_INET;

	ret = sendto(m_hSock, pData, lLen, 0, (struct sockaddr *)&peerAddr, sizeof(struct sockaddr));
	if(ret == -1)	{
		return -1;
	}
	return lLen;
}
