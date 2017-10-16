
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
#endif

#include <stdlib.h>
#include <ctype.h>
#include "OnvifSrvr.h"
#include "JdOsal.h"

#define ONVIF_MULTICAST_ADDRESS  		"239.255.255.250"
#define ONVIF_MULTICAST_PORT  			3702
#define ONVIF_ENTRY_POINT_TAG			":XAddrs>"
#define MAX_RECEIVE_MESSAGE_TIME_OUT 	3
#define MAX_MULTICAST_MESSAGE_LEN 		(1024 *3)

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
short conn_fd;

void *onvifSrvrThread(void *pArg) {
	struct sockaddr_in sock_addr;
	fd_set readFd;
	short retVal;
	int reuse = 1;
	struct timeval time;
	char *buffer;
	int length;
	uint randNo = 0;

	ONVIF_DEVICE_INFO_t *deviceInfo  = (ONVIF_DEVICE_INFO_t *)pArg;


	// Allocate temporary memory
	buffer = (char *) malloc(MAX_MULTICAST_MESSAGE_LEN);

	if (buffer != NULL) {
		// create socket to multicast message
		conn_fd = socket(AF_INET, SOCK_DGRAM, 0);

		if (conn_fd < 0) {
			printf("Creating multicast Socket Failed ");
		} else {
			if (setsockopt(conn_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse,
					sizeof(reuse)) < 0) {
				printf("Setting SO_REUSEADDR error ");
			} else {
				sock_addr.sin_family = AF_INET;
				sock_addr.sin_addr.s_addr = inet_addr(ONVIF_MULTICAST_ADDRESS);
				sock_addr.sin_port = htons(ONVIF_MULTICAST_PORT);

					FD_ZERO(&readFd);
					FD_SET(conn_fd, &readFd);

					// Receive Response of Camera
					while (m_fRun) {
						printf("Waiting for request... ");

						time.tv_sec = MAX_RECEIVE_MESSAGE_TIME_OUT;
						time.tv_usec = 0;

						retVal = select(conn_fd + 1, &readFd, NULL, NULL, &time);

						if ((retVal > 0) && (FD_ISSET(conn_fd, &readFd))) {
							if ((length = recv(conn_fd, buffer, (MAX_MULTICAST_MESSAGE_LEN - 1), MSG_NOSIGNAL)) > 0) {
								printf("Processing request... ");
								buffer[length] = '\0';
								int result = parse_request(buffer);

								if(result == 0) {
									createSrvrDiscoveryResponse(buffer, (char  *)deviceInfo->ip_addr);
									// Multicast Discovery Message
									printf("Sent discovery reply... ");
									if (sendto(conn_fd, buffer, length, MSG_NOSIGNAL,
											(struct sockaddr*) &sock_addr, sizeof(sock_addr)) < length) {
										printf("Sending Datagram Failed ");

									} else {
										// Success
									}


								}
							} else {
								printf("Failed to read socket\n");
							}
						}

						if (retVal < 0) {
							printf("Error in message receive \n");
						}

						if (retVal <= 0) {
							printf("Time out occurs in message receive \n");
						}
					}
				}
			}

			close(conn_fd);
	}

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

int onvifSrvrSop() {
	m_fRun = 0;
	close(conn_fd);
	if(m_thrdHandleOnvif){
		jdoalThreadJoin((void *)m_thrdHandleOnvif, 3000);
	}
	return 0;
}
