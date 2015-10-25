#ifndef __HLSOUT_JDAWS_H__
#define __HLSOUT_JDAWS_H__

#ifdef WIN32
#include <winsock2.h>
#include <io.h>
#define close	   closesocket
#define snprintf	_snprintf
#define write	   _write
#else
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <strings.h>
#include <netdb.h>
#include <string.h>
#define O_BINARY	0
#endif
#include "stdlib.h"
#include <fcntl.h>
#include <assert.h>
//#include <openssl/hmac.h>
#include <sstream>
#include "JdAwsS3.h"
#include "SegmentWriteBase.h"
#include "JdAwsS3UpnpHttpConnection.h"

class CSegmentWriteS3 : public CSegmentWriteBase
{
public:
	CSegmentWriteS3(const char *pszCfgFile);
	CSegmentWriteS3( const char *pszBucket, const char *pszHost, const char *pszAccessId, const char *pszSecKey);
	int Start(const char *pszParent, const char *pszFile, int nTotalLen, char *pData, int nLen, const char *pContentType);
	int Continue(char *pData, int nLen);
	int End(char *pData, int nLen);
	int Send(const char *pszParent, const char *pszFile, char *pData, int nLen, const char *pContentType, int nTimeOut);
	int Delete(const char *pszParentUrl, const char *pszFile);

private:
	std::string   m_Bucket;
	CJdAwsContext m_AwsContext;
	CJdAwsS3HttpResponse m_SessionContext;
};

#endif //__HLSOUT_JDAWS_H__
