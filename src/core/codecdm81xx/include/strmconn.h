#ifndef __STRM_COMN_H__
#define __STRM_COMN_H__

#ifdef __cplusplus              /* required for headers that might */
extern "C"
{                               /* be compiled by a C++ compiler */
#endif

// The following flags are defined in OMX_Core.h
// Define these flags if OMX_Core.h is not inlcuded
#ifndef OMX_BUFFERFLAG_EOS
#define OMX_BUFFERFLAG_EOS 0x00000001
#endif
#ifndef OMX_BUFFERFLAG_ENDOFFRAME
#define OMX_BUFFERFLAG_ENDOFFRAME 0x00000010
#endif

#ifndef OMX_EXT_BUFFERFLAG_DISCONT
#define OMX_EXT_BUFFERFLAG_DISCONT 0x00010000
#endif

#define OMX_AUD_DYNAMIC_FMT_MASK  0x00FF0000
#define OMX_AUD_DYNAMIC_FMT_PCM   0x00110000
#define OMX_AUD_DYNAMIC_FMT_AC3   0x00120000

struct _ConnCtxT;

/* Opaque buffer element type.  This would be defined by the application. */
typedef struct 
{ 
    unsigned char      *pBuffer;
    unsigned long      nAllocLen;
    unsigned long      nFilledLen;

	unsigned long      nFlags;           /**< buffer specific flags */
	unsigned long long nTimeStamp;
} OMX_BUFFERHEADERTYPE_M;
 


typedef struct _ConnCtxT
{
	void *pPvtCtx;

	int (*Read) (struct _ConnCtxT *pConn, char *pData, int lenData, unsigned long *pulFlags, long long *pllPts);
	int (*Write) (struct _ConnCtxT *pConn, char *pData, int nMaxLen, unsigned long ulFlags, long long llPts);
	int (*IsEmpty) (struct _ConnCtxT *pConn);
	int (*IsFull) (struct _ConnCtxT *pConn);
	int (*Flush) (struct _ConnCtxT *pConn);
	int (*BufferFullness) (struct _ConnCtxT *pConn);
} ConnCtxT;


ConnCtxT *CreateStrmConn(int nBufferSize, int nBufferCount);
void DeleteStrmConn(ConnCtxT *pCtx);

#ifdef __cplusplus              /* required for headers that might */
}                               /* be compiled by a C++ compiler */
#endif

#endif

