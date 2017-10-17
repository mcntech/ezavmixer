#include <stdio.h>
#if defined (WIN32)
#include <Windows.h>
#elif defined(LINUX)
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdint.h>

#include "Onvif.h"

void print_result(ONVIF_DEVICE_DISCOVERY_RESPONSE_t discovery_response) {

	u_int16_t loop;
	if ((discovery_response.no_of_camera_found > 0)
			&& (discovery_response.discovery_result != NULL)) {
		printf("[%d] ONVIF IP Camera found in network\n",
				discovery_response.no_of_camera_found);

		for (loop = 0; loop < discovery_response.no_of_camera_found; loop++) {
			printf("ONVIF IP Camera Found at [%s]\n",
					discovery_response.discovery_result[loop].ip_addr);
		}

	} else {
		printf("No ONVIF IP Camera found in network\n");
	}
}

int main(int argc, char **argv) {

	ONVIF_DEVICE_DISCOVERY_REQ_t discovery_request;

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

	discovery_request.callback = print_result;
	discovery_request.user_data = NULL;

	ONVIF_Device_Discover(discovery_request);
#ifdef WIN32
	 WSACleanup();
#endif

	return EXIT_SUCCESS;
}
