#ifndef ONVIF_H_
#define ONVIF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#if defined (WIN32)
#elif defined(LINUX) 
#include <unistd.h>
#else
// TODO
#endif

#include <stdlib.h>

#define MAX_IP_ADDRESS_WIDTH	(16)

typedef struct _CAMERA_DESCRIPT {
	char ip_addr[MAX_IP_ADDRESS_WIDTH];
	unsigned short port;

} CAMERA_DESCRIPT;

typedef struct _ONVIF_CAMERA_LIST{

    CAMERA_DESCRIPT *camera_descript; // array of camera information found in search
    int no_of_camera_found; // no of camera found in search
} ONVIF_CAMERA_LIST;


typedef void (*device_discovery_callback_t)(void *pClientCtx, int eventId);

typedef struct _OnvifDeviceItf
{
    void *(*Open)();
    void (*Close)(void *pCtx);
    int (*Start)(void *pCtx, device_discovery_callback_t callback, void *pClientCtx);
    int (*Stop)(void *pCtx);
    ONVIF_CAMERA_LIST *(*GetCameraList)(void *pCtx);
} OnvifDeviceItf;

OnvifDeviceItf *GetOnvifDeviceItf();

#ifdef __cplusplus
}
#endif

#endif /* ONVIF_H_ */
