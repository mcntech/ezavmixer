#ifndef __STRM_COMN_IPC_H__
#define __STRM_COMN_IPC_H__
#include "strmconn.h"

#ifdef __cplusplus              
extern "C"
{                               
#endif

ConnCtxT *CreateIpcStrmConn(const char *pszClientName, const char *pszServerName, int nMaxBufferSize);
void DeleteIpcStrmConn(ConnCtxT *pCtx);

#ifdef __cplusplus              /* required for headers that might */
}                               /* be compiled by a C++ compiler */
#endif

#endif

