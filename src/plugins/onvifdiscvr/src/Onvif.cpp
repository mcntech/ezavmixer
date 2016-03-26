
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
#include "Onvif.h"
#include "JdOsal.h"

#define ONVIF_MULTICAST_ADDRESS  		"239.255.255.250"
#define ONVIF_MULTICAST_PORT  			3702
#define ONVIF_ENTRY_POINT_TAG			":XAddrs>"
#define MAX_RECEIVE_MESSAGE_TIME_OUT 	3
#define MAX_MULTICAST_MESSAGE_LEN 		(1024 *3)

#if defined (WIN32)
char *strcasestr(const char *haystack, const char *needle)
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
unsigned int GetSeed()
{
	uint randNo = 0;
	struct timeval system_time;
	gettimeofday(&system_time, NULL);
	randNo = (system_time.tv_sec % 3600) + 1;
}
#endif

static void parse_discovry_response(
	char                            *recv_msg,
	ONVIF_DEVICE_DISCOVERY_RESULT_t **discovery_result,
	u_int16_t                       *no_of_camera_found
	) 
{
	char * searchPtr = NULL;
	char * endPtr = NULL;
	uint16_t loop = 0;
	ONVIF_DEVICE_DISCOVERY_RESULT_t * head_node = (*discovery_result);

	/*
	 * Sample Response
	 <?xml version="1.0" encoding="UTF-8"?>
	 <SOAP-ENV:Envelope xmlns:SOAP-ENV="http://www.w3.org/2003/05/soap-envelope"
	 xmlns:SOAP-ENC="http://www.w3.org/2003/05/soap-encoding"
	 xmlns:wsa="http://schemas.xmlsoap.org/ws/2004/08/addressing"
	 xmlns:d="http://schemas.xmlsoap.org/ws/2005/04/discovery"
	 xmlns:dn="http://www.onvif.org/ver10/network/wsdl">
	 <SOAP-ENV:Header>
	 <wsa:MessageID>uuid:17cccdd4-a43a-4096-9039-000F7C06</wsa:MessageID>
	 <wsa:RelatesTo>uuid:40417336-34c04c88-01894527-4b7d7747</wsa:RelatesTo>
	 <wsa:To SOAP-ENV:mustUnderstand="true">http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</wsa:To>
	 <wsa:Action SOAP-ENV:mustUnderstand="true">http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches</wsa:Action>
	 <d:AppSequence SOAP-ENV:mustUnderstand="true" MessageNumber="41" InstanceId="1379915976"></d:AppSequence>
	 <SubscriptionId>0</SubscriptionId>
	 </SOAP-ENV:Header>
	 <SOAP-ENV:Body>
	 <d:ProbeMatches><d:ProbeMatch>
	 <wsa:EndpointReference><wsa:Address>urn:uuid:0074d1ad-0e01-431c-92b2-000F7C06</wsa:Address></wsa:EndpointReference>
	 <d:Types>dn:NetworkVideoTransmitter</d:Types>
	 <d:Scopes>onvif://www.onvif.org/type/video_encoder
	 onvif://www.onvif.org/type/audio_encoder
	 onvif://www.onvif.org/hardware/
	 onvif://www.onvif.org/name/TCM3401
	 onvif://www.onvif.org/location/ </d:Scopes>

	 ## This is of our interest
	 <d:XAddrs>http://192.168.102.179/onvif/device_service</d:XAddrs>

	 <d:MetadataVersion>1</d:MetadataVersion>
	 </d:ProbeMatch></d:ProbeMatches>
	 </SOAP-ENV:Body>
	 </SOAP-ENV:Envelope>
	 */
	// Get Position of XAddrs in which entry point is given
	if ((searchPtr = strcasestr(recv_msg, ONVIF_ENTRY_POINT_TAG)) != NULL) {
		searchPtr = (searchPtr + strlen(ONVIF_ENTRY_POINT_TAG));

		if ((endPtr = strchr(searchPtr, '<')) != NULL) {
			// Check if it is in format http://ipAdderss/RelativePath
			if (strncmp(searchPtr, "http://", strlen("http://")) == 0) {
				endPtr = searchPtr = (searchPtr + strlen("http://"));

				while (isdigit(endPtr[0]) || (endPtr[0] == '.'))
					endPtr++;

				if ((endPtr - searchPtr) < MAX_IP_ADDRESS_WIDTH) {

					for (loop = 0; loop < (*no_of_camera_found); loop++) {
						if (strncmp(head_node[loop].ip_addr, searchPtr,
								(endPtr - searchPtr)) == 0) {
							// Just to Indicate that Camera is already found
							loop = ((*no_of_camera_found) + 1);
							break;
						}
					}

					// Check if Camera is already found
					if (loop <= (*no_of_camera_found)) {
						// Allocate memory to Store Result
						head_node = (ONVIF_DEVICE_DISCOVERY_RESULT_t *)realloc(head_node, (((*no_of_camera_found) + 1)	* sizeof(ONVIF_DEVICE_DISCOVERY_RESULT_t)));

						if (head_node != NULL) {
							(*discovery_result) = head_node;
							head_node = &((*discovery_result)[*no_of_camera_found]);
							strncpy(head_node->ip_addr, searchPtr, (endPtr - searchPtr));
							head_node->ip_addr[(endPtr - searchPtr)] = '\0';

							// Try to parse HTTP Port
							if (sscanf((endPtr + 1), "%hd", &head_node->port) != 1) {
								// Set to Default if in case camera don't provide it
								head_node->port = 80;
							}

							(*no_of_camera_found)++;
						}
					}
				}
			}
		}
	}
}

/**
 * ONVIF_Device_Discover() - Discovers ONVIF Devices in Network
 * @discovery_request: input parameter
 *
 * Returns number of ONVIF Devices found in network.
 */
int ONVIF_Device_Discover(ONVIF_DEVICE_DISCOVERY_REQ_t discovery_request) {
	struct sockaddr_in sock_addr;
	short conn_fd;
	fd_set readFd;
	short retVal;
	int reuse = 1;
	struct timeval time;
	char *buffer;
	int length;
	uint randNo = 0;
	ONVIF_DEVICE_DISCOVERY_RESULT_t *discovery_result = NULL;
	u_int16_t no_of_camera_found = 0;
	ONVIF_DEVICE_DISCOVERY_RESPONSE_t discovery_response;
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

	/*
	 * Flow Of Function
	 *
	 *	1) Prepare Message
	 *	2) Multicast Message
	 *	3) Receive Responses of Devices if Time Out/ Error Occurs go to Step 7)
	 *	4) Parse Necessary Information
	 *	5) Store It in Search Result
	 *	6) go to Step 3)
	 *	7) Give callback to user with the information of Devices Found
	 */

	// Allocate temporary memory
	buffer = (char *) malloc(MAX_MULTICAST_MESSAGE_LEN);

	if (buffer != NULL) {
		randNo = GetSeed();

		// Generate Multicast Message
		length = sprintf(buffer, onvif_device_discovery_message,
				rand_r(&randNo), rand_r(&randNo), rand_r(&randNo),
				rand_r(&randNo));

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

				// Multicast Discovery Message
				if (sendto(conn_fd, buffer, length, MSG_NOSIGNAL,
						(struct sockaddr*) &sock_addr, sizeof(sock_addr))
						< length) {
					printf("Sending Datagram Failed ");

				} else {
					time.tv_sec = MAX_RECEIVE_MESSAGE_TIME_OUT;
					time.tv_usec = 0;

					FD_ZERO(&readFd);
					FD_SET(conn_fd, &readFd);

					// Receive Response of Camera
					do {
						retVal = select(conn_fd + 1, &readFd, NULL, NULL,
								&time);

						if ((retVal > 0) && (FD_ISSET(conn_fd, &readFd))) {
							if ((length = recv(conn_fd, buffer,
									(MAX_MULTICAST_MESSAGE_LEN - 1),
									MSG_NOSIGNAL)) > 0) {
								buffer[length] = '\0';
								// NOTE: here we can directly get IP of camera from sender address
								// But parsing further for verification
								// Extract Result from response
								parse_discovry_response(buffer,
										&discovery_result, &no_of_camera_found);
							} else {
								printf("Failed to read\n");
							}
						}

						if (retVal < 0) {
							printf("Error in message receive \n");
						}

						if (retVal <= 0) {
//							printf("Time out occurs in message receive \n");
						}
					} while (retVal > 0);
				}
			}

			close(conn_fd);
		}

		free(buffer);
	}

	if (discovery_request.callback != NULL) {
		discovery_response.discovery_result = discovery_result;
		discovery_response.no_of_camera_found = no_of_camera_found;
		discovery_response.user_data = discovery_request.user_data;
		discovery_request.callback(discovery_response);
	}

	if ((no_of_camera_found > 0) && (discovery_result != NULL)) {
		free(discovery_result);
	}
	return no_of_camera_found;
}



static void parse_discovry_response_single(
	char                            *recv_msg,
	ONVIF_DEVICE_DISCOVERY_RESULT_t *discovery_result
	)
{
	char * searchPtr = NULL;
	char * endPtr = NULL;
	uint16_t loop = 0;
	ONVIF_DEVICE_DISCOVERY_RESULT_t  *node = discovery_result;

	// Get Position of XAddrs in which entry point is given
	if ((searchPtr = strcasestr(recv_msg, ONVIF_ENTRY_POINT_TAG)) != NULL) {
		searchPtr = (searchPtr + strlen(ONVIF_ENTRY_POINT_TAG));

		if ((endPtr = strchr(searchPtr, '<')) != NULL) {
			// Check if it is in format http://ipAdderss/RelativePath
			if (strncmp(searchPtr, "http://", strlen("http://")) == 0) {
				endPtr = searchPtr = (searchPtr + strlen("http://"));

				while (isdigit(endPtr[0]) || (endPtr[0] == '.'))
					endPtr++;

				if ((endPtr - searchPtr) < MAX_IP_ADDRESS_WIDTH) {
						if (node != NULL) {
							strncpy(node->ip_addr, searchPtr, (endPtr - searchPtr));
							node->ip_addr[(endPtr - searchPtr)] = '\0';

							// Try to parse HTTP Port
							if (sscanf((endPtr + 1), "%hd", &node->port) != 1) {
								// Set to Default if in case camera don't provide it
								node->port = 80;
							}
						}
				}
			}
		}
	}
}

int m_fRun = 0;

void *onvifdicvrThread(void *pArg) {
	struct sockaddr_in sock_addr;
	short conn_fd;
	fd_set readFd;
	short retVal;
	int reuse = 1;
	struct timeval time;
	char *buffer;
	int length;
	uint randNo = 0;

	ONVIF_DEVICE_DISCOVERY_REQ_t *discovery_request  = (ONVIF_DEVICE_DISCOVERY_REQ_t *)pArg;

	ONVIF_DEVICE_DISCOVERY_RESULT_t *discovery_result = NULL;
	u_int16_t no_of_camera_found = 0;
	ONVIF_DEVICE_DISCOVERY_RESPONSE_t discovery_response;
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

	/*
	 * Flow Of Function
	 *
	 *	1) Prepare Message
	 *	2) Multicast Message
	 *	3) Receive Responses of Devices if Time Out/ Error Occurs go to Step 7)
	 *	4) Parse Necessary Information
	 *	5) Store It in Search Result
	 *	6) go to Step 3)
	 *	7) Give callback to user with the information of Devices Found
	 */

	// Allocate temporary memory
	buffer = (char *) malloc(MAX_MULTICAST_MESSAGE_LEN);

	if (buffer != NULL) {
		randNo = GetSeed();

		// Generate Multicast Message
		length = sprintf(buffer, onvif_device_discovery_message,
				rand_r(&randNo), rand_r(&randNo), rand_r(&randNo),
				rand_r(&randNo));

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

				// Multicast Discovery Message
				if (sendto(conn_fd, buffer, length, MSG_NOSIGNAL,
						(struct sockaddr*) &sock_addr, sizeof(sock_addr))
						< length) {
					printf("Sending Datagram Failed ");

				} else {
					time.tv_sec = MAX_RECEIVE_MESSAGE_TIME_OUT;
					time.tv_usec = 0;

					FD_ZERO(&readFd);
					FD_SET(conn_fd, &readFd);

					// Receive Response of Camera
					do {
						retVal = select(conn_fd + 1, &readFd, NULL, NULL,
								&time);

						if ((retVal > 0) && (FD_ISSET(conn_fd, &readFd))) {
							if ((length = recv(conn_fd, buffer,
									(MAX_MULTICAST_MESSAGE_LEN - 1),
									MSG_NOSIGNAL)) > 0) {
								buffer[length] = '\0';
								// NOTE: here we can directly get IP of camera from sender address
								// But parsing further for verification
								// Extract Result from response
								parse_discovry_response(buffer,
										&discovery_result, &no_of_camera_found);

								ONVIF_DEVICE_DISCOVERY_RESULT_t discovery_result;
								parse_discovry_response_single(buffer, &discovery_result);
								if (discovery_request->callback != NULL) {
									discovery_response.discovery_result = &discovery_result;
									discovery_response.no_of_camera_found = 1;
									discovery_response.user_data = discovery_request->user_data;
									discovery_request->callback(discovery_response);
								}
							} else {
								printf("Failed to read\n");
							}
						}

						if (retVal < 0) {
							printf("Error in message receive \n");
						}

						if (retVal <= 0) {
//							printf("Time out occurs in message receive \n");
						}
					} while (retVal > 0);
				}
			}

			close(conn_fd);
		}

		free(buffer);
	}

	return 0;
}

static ONVIF_DEVICE_DISCOVERY_REQ_t g_discovery_request;
#ifdef WIN32
	HANDLE              m_thrdHandleOnvif;
#else
	pthread_t			m_thrdHandleOnvif;
#endif

int onvifdicvrStart(ONVIF_DEVICE_DISCOVERY_REQ_t discovery_request) {
	g_discovery_request = discovery_request;
	jdoalThreadCreate((void **)&m_thrdHandleOnvif, onvifdicvrThread, (void *)&g_discovery_request);
}

int onvifdicvrSop() {
	m_fRun = 0;
	if(m_thrdHandleOnvif){
		jdoalThreadJoin((void *)m_thrdHandleOnvif, 3000);
	}

}
