
#ifndef ONVIF_H_
#define ONVIF_H_

#include <stdio.h>
#if defined (WIN32)
#define u_int16_t unsigned short
#define uint16_t  unsigned short
#define uint      unsigned int 
#elif defined(LINUX) 
#include <unistd.h>
#else
// TODO
#endif

#include <stdlib.h>

#define MAX_IP_ADDRESS_WIDTH	(16)

typedef struct {
	char ip_addr[MAX_IP_ADDRESS_WIDTH];
	u_int16_t port;

} ONVIF_DEVICE_DISCOVERY_RESULT_t;

typedef struct {

	ONVIF_DEVICE_DISCOVERY_RESULT_t *discovery_result; // array of camera information found in search
	u_int16_t no_of_camera_found; // no of camera found in search
	void *user_data; // user data

} ONVIF_DEVICE_DISCOVERY_RESPONSE_t;

typedef struct {
	// Caller will be given response in this callback
	void (*callback)(
			ONVIF_DEVICE_DISCOVERY_RESPONSE_t discovery_response);
	void *user_data; // user data

} ONVIF_DEVICE_DISCOVERY_REQ_t;

/**
 * ONVIF_Device_Discover() - Discovers ONVIF Devices in Network
 * @discovery_request: input parameter
 *
 * Returns number of ONVIF Devices found in network.
 */
int ONVIF_Device_Discover(ONVIF_DEVICE_DISCOVERY_REQ_t discovery_request);

#endif /* ONVIF_H_ */
