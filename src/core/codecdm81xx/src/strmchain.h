#ifndef __STRM_CHAIN_H__
#define __STRM_CHAIN_H__
#include "strmcomp.h"

struct _StrmChainIf;

typedef struct _StrmChainIf
{
	int (*Open)(struct _StrmChainIf *pChain, void *pOptions);
	int (*SetOption)(struct _StrmChainIf *pChain, int nCmd, char *pOptionData);
	int (*SetInputConn)(struct _StrmChainIf *pChain, int nConnNum, ConnCtxT *pConn);
	int (*SetOutputConn)(struct _StrmChainIf *pChain, int nCOnnNum, ConnCtxT *pConn);
	int (*SetClkSrc)(struct _StrmChainIf *pChain, void *);
	void *(*GetClkSrc)(struct _StrmChainIf *pChain);
	int (*Start)(struct _StrmChainIf *pChain);
	int (*Stop)(struct _StrmChainIf *pChain);
	void (*Close)(struct _StrmChainIf *pChain);
	void (*Delete)(struct _StrmChainIf *pChain);
	void *pCtx;
} StrmChainIf;

#endif