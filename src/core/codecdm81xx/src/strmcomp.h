#ifndef __STRM_COMP_H__
#define __STRM_COMP_H__
#include "strmconn.h"

typedef enum _STRM_CMD_T
{
	STRM_CMD_NONE,
	STRM_CMD_RUN,
	STRM_CMD_PAUSE,
	STRM_CMD_STOP
} STRM_CMD_T;

typedef enum _STRM_EVENT_T
{
	STRM_EVENT_NONE,
	STRM_EVENT_DISCONTINUITY,
	STRM_EVENT_FORMAT_CHANGE,
	STRM_EVENT_OMX_ERROR,
	STRM_EVENT_EOS
} STRM_EVENT_T;

struct _StrmCompIf;

typedef struct _StrmCompIf
{
	int (*Open)(struct _StrmCompIf *pComp, const char *szResource);
	int (*AllocateResource)(struct _StrmCompIf *pComp);
	int (*SetOption)(struct _StrmCompIf *pComp, int nCmd, char *pOptionData);
	
	// Cirular bugger based connection
	int (*SetInputConn)(struct _StrmCompIf *pComp, int nConnNum, ConnCtxT *pConn);
	int (*SetOutputConn)(struct _StrmCompIf *pComp, int nCOnnNum, ConnCtxT *pConn);

	// Component/Pipe based connection
	int (*SetInputConn2)(struct _StrmCompIf *pChain, int nInSlot, void *pExtSrcComp, int nRemotePort);
	int (*SetOutputConn2)(struct _StrmCompIf *pChain, int nOutSlot, void *pExtSinkComp, int nRemotePort);

	int (*SetClkSrc)(struct _StrmCompIf *pComp, void *);
	void *(*GetClkSrc)(struct _StrmCompIf *pComp);
	int (*Start)(struct _StrmCompIf *pComp);
	int (*Stop)(struct _StrmCompIf *pComp);
	void (*Close)(struct _StrmCompIf *pComp);
	void (*Delete)(struct _StrmCompIf *pComp);
	void *pCtx;
} StrmCompIf;

#endif