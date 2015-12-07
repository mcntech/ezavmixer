#define	HLS_OUTPUT_FS               2
#ifndef __SEGMENT_WRITE_BASE_H__
#define __SEGMENT_WRITE_BASE_H__

#include <ctime>

#define CONTENT_STR_TYPE			"Content-Type"
#define CONTENT_STR_HTML			"text/html"
//#define CONTENT_STR_M3U8			"application/vnd.apple.mpegurl"
#define CONTENT_STR_M3U8			"audio/x-mpegurl"
#define CONTENT_STR_MP2T			"application/octet-stream"
#define CONTENT_STR_DEF				"application/octet-stream"

class CSegmentWriteBase
{
public:
	virtual int Start(const char *pszParent, const char *pszFile, const std::time_t request_date, int nTotalLen, char *pData, int nLen, const char *pContentType) = 0;
	virtual int Continue(char *pData, int nLen) = 0;
	virtual int End(char *pData, int nLen) = 0;
	virtual int Send(const char *pszParent, const char *pszFile, const std::time_t request_date, const char *pData, int nLen, const char *pContentType, int nTimeOut) = 0;
	virtual int Delete(const char *pszParentUrl, const char *pszFile) = 0;

public:
	void *m_pCtx;
};
#endif // __HLS_OUT_BASE_H__
