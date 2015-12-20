#ifndef __JD_MPEG_DASH_SGMT_H__
#define __JD_MPEG_DASH_SGMT_H__

#define MPD_UPLOADER_TYPE_S3			1
#define MPD_UPLOADER_TYPE_DISC			2
#define MPD_UPLOADER_TYPE_MEM			3

#define MPD_UPLOAD_UNKNOWN_STATE		0
#define MPD_UPLOAD_STATE_STOP			1
#define MPD_UPLOAD_STATE_RUN			2
#define MPD_UPLOAD_ERROR_CONN_FAIL		0x80000001
#define MPD_UPLOAD_ERROR_XFR_FAIL		0x80000001
#define MPD_UPLOAD_ERROR_INPUT_TIMEOUT  0x80000002

#define MPD_FLAG_HAS_SPS                0x00000001
#define MPD_FLAG_DISCONT                0x00010000
#include "Mpd.h"
#include "JdAwsContext.h"

int mpdWriteFrameData(void *pCtx, char *pData, int nLen, int fVideo, int fDisCont,  unsigned long ulPtsMs);
int mpdEndOfSeq(void *pCtx);

void *mpdPublishStart(
		int		    nTotalTimeMs,				///< Total time of stream
		void	    *pSegmenter,				///< Segmenter to be used
		CMpdRepresentation	    *pMpdRep, 
		const char	*pszSrcM3u8Url,             ///< Play list file name
		const char	*pszDestParentUrl,			///< Network folder name
		const char	*pszBucketOrSvrRoot, 
		CJdAwsContext  *pServerCtxt,
		int         nStartIndex,
		int         nDestType
		);		

void mpdPublishStop(void *pUploadCtx);
int mpdPublishGetStats(void *pUploadCtx, int *pnState, int *pnStreamInTime, int *nLostBufferTime,  int *pnStreamOutTime, int *nSegmentsTime);

void mpgdshSetDebugLevel(int nLevel);

void *mpdCreateSegmenter(CMpdRepresentation *pMpdRep);

void mpdDeleteSegmenter(void *pSegmenter);

void *mpdCreateMpdGenerator();

void mpdDeleteMpdGenerator(void *pMpdGen);

#endif //__JD_MPEG_DASH_SGMT_H__
