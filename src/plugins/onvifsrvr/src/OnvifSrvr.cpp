
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#if defined (WIN32)
#include <winsock2.h>
#include <fcntl.h>
#include <io.h>
#include <Ws2ipdef.h>
#define close	closesocket
#define snprintf	_snprintf
#define write	_write
#define MSG_NOSIGNAL	0
#elif defined(SYS_BIOS)
// TODO
#else
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#include <errno.h>

#include <stdlib.h>
#include <ctype.h>
#include "OnvifSrvr.h"
#include "JdOsal.h"
#include "JdDbg.h"

#define ONVIF_MULTICAST_ADDRESS  		"239.255.255.250"
#define ONVIF_MULTICAST_PORT  			3702
#define ONVIF_ENTRY_POINT_TAG			":XAddrs>"
#define MAX_RECEIVE_MESSAGE_TIME_OUT 	300
#define MAX_MULTICAST_MESSAGE_LEN 		(1024 *3)

static int modDbgLevel = CJdDbg::LVL_TRACE;


int mcast_recv_socket(char* multicastIP, char* multicastPort, int multicastRecvBufSize) {

    int sock;
    struct addrinfo   hints  = { 0 };    /* Hints for name lookup */
    struct addrinfo*  localAddr = 0;         /* Local address to bind to */
    struct addrinfo*  multicastAddr = 0;     /* Multicast Address */
    int yes=1;

    int optval=0;
    socklen_t optval_len = sizeof(optval);
    int dfltrcvbuf;


    /* Resolve the multicast group address */
    hints.ai_family = PF_UNSPEC;
    hints.ai_flags  = AI_NUMERICHOST;
    int status;
    if ((status = getaddrinfo(multicastIP, NULL, &hints, &multicastAddr)) != 0) {
	    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
	    goto error;
    }


    /*
       Get a local address with the same family (IPv4 or IPv6) as our multicast group
       This is for receiving on a certain port.
    */
    hints.ai_family   = multicastAddr->ai_family;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags    = AI_PASSIVE; /* Return an address we can bind to */
    if ( getaddrinfo(NULL, multicastPort, &hints, &localAddr) != 0 ) {
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
	goto error;
    }


    /* Create socket for receiving datagrams */
    if ( (sock = socket(localAddr->ai_family, localAddr->ai_socktype, 0)) < 0 ) {
	perror("socket() failed");
	goto error;
    }



    /*
     * Enable SO_REUSEADDR to allow multiple instances of this
     * application to receive copies of the multicast datagrams.
     */
    if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char*)&yes,sizeof(int)) == -1) {
	perror("setsockopt");
	goto error;
    }

    /* Bind the local address to the multicast port */
    if ( bind(sock, localAddr->ai_addr, localAddr->ai_addrlen) != 0 ) {
	perror("bind() failed");
		goto error;
    }

    /* get/set socket receive buffer */
    if(getsockopt(sock, SOL_SOCKET, SO_RCVBUF,(char*)&optval, &optval_len) !=0) {
	perror("getsockopt");
	goto error;
    }
    dfltrcvbuf = optval;
    optval = multicastRecvBufSize;
    if(setsockopt(sock,SOL_SOCKET,SO_RCVBUF,(char*)&optval,sizeof(optval)) != 0) {
	perror("setsockopt");
	goto error;
    }
    if(getsockopt(sock, SOL_SOCKET, SO_RCVBUF,(char*)&optval, &optval_len) != 0) {
	perror("getsockopt");
	goto error;
    }
    printf("tried to set socket receive buffer from %d to %d, got %d\n",
	   dfltrcvbuf, multicastRecvBufSize, optval);




    /* Join the multicast group. We do this seperately depending on whether we
     * are using IPv4 or IPv6.
     */
    if ( multicastAddr->ai_family  == PF_INET &&
	 multicastAddr->ai_addrlen == sizeof(struct sockaddr_in) ) /* IPv4 */
	{
	    struct ip_mreq multicastRequest;  /* Multicast address join structure */

	    /* Specify the multicast group */
	    memcpy(&multicastRequest.imr_multiaddr,
		   &((struct sockaddr_in*)(multicastAddr->ai_addr))->sin_addr,
		   sizeof(multicastRequest.imr_multiaddr));

	    /* Accept multicast from any interface */
	    multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);

	    /* Join the multicast address */
	    if ( setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &multicastRequest, sizeof(multicastRequest)) != 0 ) {
		perror("setsockopt() failed");
		goto error;
	    }
	}
    else if ( multicastAddr->ai_family  == PF_INET6 &&
	      multicastAddr->ai_addrlen == sizeof(struct sockaddr_in6) ) /* IPv6 */
	{
	    struct ipv6_mreq multicastRequest;  /* Multicast address join structure */

	    /* Specify the multicast group */
	    memcpy(&multicastRequest.ipv6mr_multiaddr,
		   &((struct sockaddr_in6*)(multicastAddr->ai_addr))->sin6_addr,
		   sizeof(multicastRequest.ipv6mr_multiaddr));

	    /* Accept multicast from any interface */
	    multicastRequest.ipv6mr_interface = 0;

	    /* Join the multicast address */
	    if ( setsockopt(sock, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, (char*) &multicastRequest, sizeof(multicastRequest)) != 0 ) {
		perror("setsockopt() failed");
		goto error;
	    }
	}
    else {
	perror("Neither IPv4 or IPv6");
  	goto error;
    }



    if(localAddr)
	freeaddrinfo(localAddr);
    if(multicastAddr)
	freeaddrinfo(multicastAddr);

    return sock;

 error:
    if(localAddr)
	freeaddrinfo(localAddr);
    if(multicastAddr)
	freeaddrinfo(multicastAddr);

    return -1;
}


#if defined (WIN32)
static char *strcasestr(const char *haystack, const char *needle)
{
	int nlen = strlen(needle);
	int hlen = strlen(haystack) - nlen + 1;
	int i;
	for (i = 0; i < hlen; i++) {
		int j;
		for (j = 0; j < nlen; j++) {
			unsigned char c1 = haystack[i+j];
			unsigned char c2 = needle[j];
			if (toupper(c1) != toupper(c2))
				 goto next;
		}
	return (char *) haystack + i;
	next:
		;
	}	
	return NULL;
}
int rand_r(unsigned int* seed) 
{
	srand(* seed);
	return rand();
} 

unsigned int GetSeed()
{
	return GetTickCount();
}
#else
static unsigned int GetSeed()
{
	uint randNo = 0;
	struct timeval system_time;
	gettimeofday(&system_time, NULL);
	randNo = (system_time.tv_sec % 3600) + 1;
}
#endif

static void createSrvrDiscoveryResponse(
	char                            *ipAddr,
	char                            *szResponse
	) 
{
	char  *responseTemplate =
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
	 "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\""
	 "xmlns:SOAP-ENC=\"http://www.w3.org/2003/05/soap-encoding\""
	 "xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\""
	 "xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\""
	 "xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\">"
	 "<SOAP-ENV:Header>"
	 "<wsa:MessageID>uuid:17cccdd4-a43a-4096-9039-000F7C06</wsa:MessageID>"
	 "<wsa:RelatesTo>uuid:40417336-34c04c88-01894527-4b7d7747</wsa:RelatesTo>"
	 "<wsa:To SOAP-ENV:mustUnderstand=\"true\">http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</wsa:To>"
	 "<wsa:Action SOAP-ENV:mustUnderstand=\"true\">http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches</wsa:Action>"
	 "<d:AppSequence SOAP-ENV:mustUnderstand=\"true\" MessageNumber=\"41\" InstanceId=\"1379915976\"></d:AppSequence>"
	 "<SubscriptionId>0</SubscriptionId>"
	 "</SOAP-ENV:Header>"
	 "<SOAP-ENV:Body>"
	 "<d:ProbeMatches><d:ProbeMatch>"
	 "<wsa:EndpointReference><wsa:Address>urn:uuid:0074d1ad-0e01-431c-92b2-000F7C06</wsa:Address></wsa:EndpointReference>"
	 "<d:Types>dn:NetworkVideoTransmitter</d:Types>"
	 "<d:Scopes>onvif://www.onvif.org/type/video_encoder"
	 "onvif://www.onvif.org/type/audio_encoder"
	 "onvif://www.onvif.org/hardware/"
	 "onvif://www.onvif.org/name/TCM3401"
	 "onvif://www.onvif.org/location/ </d:Scopes>"
	 "<d:XAddrs>http://%s/onvif/device_service</d:XAddrs>"
	 "<d:MetadataVersion>1</d:MetadataVersion>"
	 "</d:ProbeMatch></d:ProbeMatches>"
	 "</SOAP-ENV:Body>"
	 "</SOAP-ENV:Envelope>";
	sprintf(szResponse, responseTemplate, ipAddr);

}


int parse_request(char *reuestBuff) {
	int result = 0;
	static const char *onvif_device_discovery_message =

			"<?xml version='1.0' encoding='utf-8'?><soap:Envelope xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\" "
					"xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" xmlns:wsadis=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\""
					"xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\">"
					"<soap:Header>"
					"<wsadis:MessageID>uuid:%08x-%08x-%08x-%08x</wsadis:MessageID>"
					"<wsadis:To>urn:schemas-xmlsoap-org:ws:2005:04:discovery</wsadis:To>"
					"<wsadis:Action>http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe</wsadis:Action>"
					"</soap:Header>"
					"<soap:Body>"
					"<d:Probe>"
					"<d:Types/><d:Scopes/>"
					"</d:Probe>"
					"</soap:Body>"
					"</soap:Envelope>";

	return result;
}


int m_fRun = 0;
short m_fd;

void *onvifSrvrThread(void *pArg) {
	struct sockaddr_in sock_addr;
	fd_set             readFd;
	short              retVal;
	int                reuse = 1;
	struct timeval     time;
	char               *buffer;
	int                length;
	uint               randNo = 0;

	ONVIF_DEVICE_INFO_t *deviceInfo  = (ONVIF_DEVICE_INFO_t *)pArg;

	struct sockaddr_storage peer_addr = {0};
	socklen_t peer_addrlen = sizeof(peer_addr);

	// Allocate temporary memory
	buffer = (char *) malloc(MAX_MULTICAST_MESSAGE_LEN);

	if (buffer == NULL) {
		goto ErrorOrExit;
	}
	// create socket to multicast message
	m_fd = mcast_recv_socket(ONVIF_MULTICAST_ADDRESS, "3702", MAX_MULTICAST_MESSAGE_LEN);

	// Receive Response of Camera
	while (m_fRun) {
		printf("Waiting for request... \n");
		 int bytes = 0;
		time.tv_sec = MAX_RECEIVE_MESSAGE_TIME_OUT;
	     if ((bytes = recvfrom(m_fd, buffer, MAX_MULTICAST_MESSAGE_LEN, 0, (struct sockaddr *)&peer_addr, &peer_addrlen)) < 0) {
	    	 printf("Read failed %s\n", strerror(errno));
	     } else {
	    	 char host[64];
	    	 getnameinfo((struct sockaddr *)&peer_addr, peer_addrlen, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
	    	 char ipbuff[INET_ADDRSTRLEN];
	    	 inet_ntop(peer_addr.ss_family, &(((struct sockaddr_in *)&peer_addr)->sin_addr), ipbuff, INET_ADDRSTRLEN);
	    	 short port = ((struct sockaddr_in *)&peer_addr)->sin_port;
	    	 printf("from IP address %s %s port=%d\n", host, ipbuff, port);

	    	 createSrvrDiscoveryResponse(deviceInfo->ip_addr, buffer);
			if (sendto(m_fd, buffer, strlen(buffer), MSG_NOSIGNAL, (struct sockaddr*) &peer_addr, sizeof(peer_addr)) < length) {
				printf("Sending Datagram Failed ");

			}
	     }
	}

	close(m_fd);

ErrorOrExit:
	free(buffer);

	return 0;
}

static ONVIF_DEVICE_INFO_t g_deviceInfo;
#ifdef WIN32
	HANDLE              m_thrdHandleOnvif;
#else
	pthread_t			m_thrdHandleOnvif;
#endif

int onvifSrvrStart(ONVIF_DEVICE_INFO_t *deviceInfo) {
	g_deviceInfo = *deviceInfo;
	m_fRun = 1;
	jdoalThreadCreate((void **)&m_thrdHandleOnvif, onvifSrvrThread, (void *)&g_deviceInfo);
	return 0;
}

int onvifSrvrStop() {
	m_fRun = 0;
	close(m_fd);
	if(m_thrdHandleOnvif){
		jdoalThreadJoin((void *)m_thrdHandleOnvif, 3000);
	}
	return 0;
}
