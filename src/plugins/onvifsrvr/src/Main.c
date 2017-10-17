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
#include <ifaddrs.h>
#include <string.h>

#include "OnvifSrvr.h"


int main(int argc, char **argv) {

	ONVIF_DEVICE_INFO_t deviceInfo;

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

	struct ifaddrs * ifAddrStruct=NULL;
	struct ifaddrs * ifa=NULL;
	void * tmpAddrPtr=NULL;
	memset(deviceInfo.ip_addr, 0x00, sizeof(deviceInfo.ip_addr));
	getifaddrs(&ifAddrStruct);

	for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr) {
			continue;
		}
		if (ifa->ifa_addr->sa_family == AF_INET) {
			// is a valid IP4 Address
			tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
			char addressBuffer[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
			if(strstr("lo", ifa->ifa_name)) {
				continue;
			} else if (strlen(deviceInfo.ip_addr) < (MAX_IP_ADDRESS_WIDTH * (MAX_NUM_IP_ADDRESSES - 1))){
				if(strlen(deviceInfo.ip_addr)) {
					strcat(deviceInfo.ip_addr," ");
				}
				strcat(deviceInfo.ip_addr, addressBuffer);
			}
		}
	}

	if (ifAddrStruct!=NULL)
		freeifaddrs(ifAddrStruct);
	deviceInfo.port = 59427;

	printf("IP Addresses %s port %d\n", deviceInfo.ip_addr, deviceInfo.port);

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
