#ifndef __IAM_JDAWS_H__
#define __IAM_JDAWS_H__

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
#include <ctime>
#include "stdlib.h"
#include <fcntl.h>
#include <assert.h>
//#include <openssl/hmac.h>
#include <sstream>
#include "JdAwsS3.h"
#include "JdAwsRest.h"
#include "Iam.h"

class CIam
{
public:
	CIam(const char *pszHost, const char *pszAccessId, const char *pszSecKey);
	int Get(const char *query, const char *contentType, const std::time_t request_date, const char *pPayLoad, int nLen, int nTimeOut);

private:
	CJdAwsContext m_AwsContext;
	CJdAwsS3HttpResponse m_SessionContext;
};

#endif //__HLSOUT_JDAWS_H__
