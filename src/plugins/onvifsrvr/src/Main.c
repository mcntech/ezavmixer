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
			if(ifa->ifa_name && strstr(ifa->ifa_name, "eth")) {
				strcpy(deviceInfo.ip_addr, addressBuffer);
				break;
			}
		}
	}
	if (ifAddrStruct!=NULL)
		freeifaddrs(ifAddrStruct);

	deviceInfo.port = 59427;
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
