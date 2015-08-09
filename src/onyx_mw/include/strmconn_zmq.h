#ifndef __STRM_COMN_ZMQ_H__
#define __STRM_COMN_ZMQ_H__
#include "strmconn.h"

#ifdef __cplusplus              
extern "C"
{                               
#endif

ConnCtxT *CreateZmqStrmConn(int fServer, const char *pszSockName, int nMaxBufferSize);
void DeleteZmqStrmConn(ConnCtxT *pCtx);

#ifdef __cplusplus              /* required for headers that might */
}                               /* be compiled by a C++ compiler */
#endif

#endif

