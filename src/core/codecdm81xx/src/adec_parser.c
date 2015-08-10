#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "dec_clock.h"
#include "dbglog.h"
#include "strmcomp.h"
#include "dbglog.h"

typedef struct _AusParserBaseIf
{
	int fStreaming;
	int  nUiCmd;
} AudParserBaseIf;

/* Audio chunking */
typedef struct
{
	unsigned char state;
} Aud_ChunkingCtx;

typedef struct
{
	AudParserBaseIf ParserBase;
	FILE *fp;
	unsigned int frameNo;
	unsigned int frameSize;
	unsigned char *readBuf;
	unsigned char *chunkBuf;

	Aud_ChunkingCtx inBuf;
	Aud_ChunkingCtx outBuf;
	unsigned int bytes;
	unsigned int tmp;
	unsigned char firstParse;
	unsigned int chunkCnt;

	int          fUseDemux;
	int          fEoS;

	unsigned long long crnt_aud_pts;
	unsigned long long crnt_vid_dts;
	ConnCtxT   *pConn;
} AAC_ParsingCtx;


static int ParserReadData(AudParserBaseIf *pComp, ConnCtxT *pConn, unsigned char *pData, int lenData, 
	unsigned long long *pPts, int *pfEos
)
{
	int ret = 0;
	unsigned long ulFlags = 0;
	// TODO : Add termination condition
	while(pConn->IsEmpty(pConn) && pComp->nUiCmd !=  STRM_CMD_STOP){
		//DBG_LOG(DBGLVL_WARN,("start=%d end=%d",pConn->pdpCtx->start, pConn->pdpCtx->end))
#ifdef WIN32
		Sleep(1);
#else
		usleep(1000);
#endif
	}

	if(pComp->nUiCmd ==  STRM_CMD_STOP) {
		ulFlags = OMX_BUFFERFLAG_EOS;
	} else {
		ret = pConn->Read(pConn, pData, lenData, &ulFlags, pPts);
	}

	if(ulFlags & OMX_BUFFERFLAG_EOS) {
		*pfEos = 1;
	} else {
		*pfEos = 0;
	}
	return ret;
}
