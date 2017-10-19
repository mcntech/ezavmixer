#include <stdio.h>
#if defined (WIN32)
#include <Windows.h>
#elif defined(LINUX)
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include "OnvifSrvr.h"


int main(int argc, char **argv) {

	ONVIF_DEVICE_INFO_t deviceInfo;

	get_host_address(&deviceInfo);
	onvifSrvrStart(&deviceInfo);
#ifdef WIN32
	 WSACleanup();
#endif
	while(1){
		usleep(1000*1000);
	}
	onvifSrvrStop(&deviceInfo);
	return EXIT_SUCCESS;
}
