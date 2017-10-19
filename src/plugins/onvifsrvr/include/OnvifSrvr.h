
#ifndef __ONVIF_SRVR_H__
#define __ONVIF_SRVR_H__

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
#define MAX_NUM_IP_ADDRESSES    (4)

typedef struct {
	char ip_addr[MAX_IP_ADDRESS_WIDTH * MAX_NUM_IP_ADDRESSES];
	u_int16_t port;
} ONVIF_DEVICE_INFO_t;

#ifdef __cplusplus
extern "C" {
#endif
	int get_host_address(ONVIF_DEVICE_INFO_t *deviceInfo);
	int onvifSrvrStart(ONVIF_DEVICE_INFO_t *deviceInfo);
	int onvifSrvrStop();

#ifdef __cplusplus
}
#endif

#endif /* __ONVIF_SRVR_H__ */
