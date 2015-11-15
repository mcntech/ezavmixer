/*
 *  JdAwsS3JdAwsHttpConnection.cpp
 *
 *  Copyright 2011 MCN Technologies Inc.. All rights reserved.
 *
 */

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

#include "JdAwsContext.h"
#include "JdAwsS3.h"
#include "JdAwsS3UpnpHttpConnection.h"
#include "../include/SegmentWriteS3.h"
#include "JdDbg.h"

#define PORT_NUMBER 			80
#define HTTP_VERSION 			"HTTP/1.0"
#define DEFAULT_USER_AGENT		"HTTP NTVRenderer"
#define VERSION					"1.0"
#define DEFAULT_READ_TIMEOUT	30		/* Seconds to wait before giving up
										 *	when no data is arriving */
#define DEFAULT_WRITE_TIMEOUT	30
#define REQUEST_BUF_SIZE 		1024
#define HEADER_BUF_SIZE 		1024
#define DEFAULT_PAGE_BUF_SIZE 	1024 * 200	/* 200K should hold most things */
#define DEFAULT_REDIRECTS       3       /* Number of HTTP redirects to follow */

/* Error sources */
#define FETCHER_ERROR	0
#define ERRNO			1
#define H_ERRNO			2

#ifndef MSG_NO_SIGNAL
#define MSG_NOSIGNAL	0
#endif

typedef enum _CONTENT_TYPE_T
{
	CONTENT_TYPE_NONE,
	CONTENT_TYPE_HTML,
	CONTENT_TYPE_M3U8,
	CONTENT_TYPE_MP2T
}CONTENT_TYPE_T;

typedef enum _AMZ_ACL_T
{
	AMZ_ACL_NONE,
	AMZ_ACL_PRIVATE,
	AMZ_ACL_PUBLIC_READ,
	AMZ_ACL_PUBLIC_READ_WRITE,
	AMZ_ACL_PUBLIC_AUTH_READ,
	AMZ_ACL_PUBLIC_OWNER_READ,
	AMZ_ACL_PUBLIC_OWNER_FULL_CTRL
}AMZ_ACL_T;

#define X_AMZ_ACL					"x-amz-acl"

#define ACL_STR_PRIVATE				"private" 
#define ACL_STR_PUBLIC_READ			"public-read"
#define ACL_STR_PUBLIC_READ_WRITE	"public-read-write"
#define ACL_STR_AUTH_READ			"authenticated-read" 
#define ACL_STR_OWNER_READ			"bucket-owner-read"
#define ACL_STR_OWNER_FULL_CTRL		"bucket-owner-full-control"

#define PORT_NUMBER 			80

#define DBG_TAG                      "JdAws"

static int  modDbgLevel = CJdDbg::LVL_STRM;

int ReadHeader(int sock, char *headerPtr);

typedef struct _JDAWS_CONNECTION
{
	int  m_Sock;
	char *m_pszHost;
} JDAWS_CONNECTION;

int get_file_size(const char *pFilePath, char *arg)
{
	//TODO;
	return 0;
}


/*
 * Determines if the given NULL-terminated buffer is large enough to
 * 	concatenate the given number of characters.  If not, it attempts to
 * 	grow the buffer to fit.
 * Returns:
 *	0 on success, or
 *	JD_ERROR on error (original buffer is unchanged).
 */
int CheckBufSize(char **buf, int *bufsize, int more)
{
	char *tmp;
	int roomLeft = *bufsize - (strlen(*buf) + 1);
	if(roomLeft > more)
		return 0;
	tmp = (char *)realloc(*buf, *bufsize + more + 1);
	if(tmp == NULL)
		return JD_ERROR;
	*buf = tmp;
	*bufsize += more + 1;
	return 0;
}
#if 0
int GetS3Headers(int nAmzAcl, int nContentType, char *pData, int nMaxLen)
{
	char szTmpBuf[128];
	int nUsedLen = 0;
	if(nAmzAcl != AMZ_ACL_NONE){
		switch(nAmzAcl)
		{
		case 	AMZ_ACL_PRIVATE:
			sprintf(szTmpBuf,X_AMZ_ACL ": " ACL_STR_PRIVATE "\r\n");
			break;
		case 	AMZ_ACL_PUBLIC_READ:
			sprintf(szTmpBuf,X_AMZ_ACL ": " ACL_STR_PUBLIC_READ "\r\n");
			break;
		case 	AMZ_ACL_PUBLIC_AUTH_READ:
			sprintf(szTmpBuf,X_AMZ_ACL ": " ACL_STR_AUTH_READ "\r\n");
			break;
		case 	AMZ_ACL_PUBLIC_OWNER_READ:
			sprintf(szTmpBuf,X_AMZ_ACL ": " ACL_STR_OWNER_READ "\r\n");
			break;
		case 	AMZ_ACL_PUBLIC_OWNER_FULL_CTRL:
			sprintf(szTmpBuf,X_AMZ_ACL ": " ACL_STR_OWNER_FULL_CTRL "\r\n");
			break;
		case AMZ_ACL_PUBLIC_READ_WRITE:
		default:
			sprintf(szTmpBuf,X_AMZ_ACL ": " ACL_STR_PUBLIC_READ_WRITE "\r\n");
			break;

		}
		int nLen = strlen(szTmpBuf);
		if(nUsedLen + nLen < nMaxLen){
			nUsedLen += nLen;
			strcpy(pData, szTmpBuf);
		}
	}
	if(nContentType != CONTENT_TYPE_NONE){
		switch(nContentType)
		{
		case 	CONTENT_TYPE_HTML:
			sprintf(szTmpBuf,CONTENT_STR_TYPE ": " CONTENT_STR_HTML "\r\n");
			break;
		case 	CONTENT_TYPE_M3U8:
			sprintf(szTmpBuf,CONTENT_STR_TYPE ": " CONTENT_STR_M3U8 "\r\n");
			break;
		case 	CONTENT_TYPE_MP2T:
			sprintf(szTmpBuf,CONTENT_STR_TYPE ": " CONTENT_STR_MP2T "\r\n");
			break;
		default:
			sprintf(szTmpBuf,CONTENT_STR_TYPE ": " CONTENT_STR_DEF "\r\n");
			break;

		}
		int nLen = strlen(szTmpBuf);
		if(nUsedLen + nLen < nMaxLen){
			nUsedLen += nLen;
			strcat(pData, szTmpBuf);
		} else {
			fprintf(stderr, "buffer overflow%s:%d\n",__FUNCTION__,__LINE__);
		}
	}
	return nUsedLen;
}
#endif
//int MakeHttpRequest(void *handle, const char *url_tmp, int nContentLen)
JD_STATUS JdAwsMakeHttpRequest(
    JdAws_HttpMethod method,
	const char       *hostName,
	const char       *bucketName,
	const char       *objectName,
	void             *handle,
	char             *headers,                                    
	const char       *contentType,
	int              nContentLen,
	int              timeout)
{
	JD_STATUS res = JD_ERROR;

	std::ostringstream httpReq;
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:%s : Begin\n", DBG_TAG, __FUNCTION__));
	JDAWS_CONNECTION *pConn = (JDAWS_CONNECTION *)handle;
	int sock = pConn->m_Sock;
	char *host;// = pConn->m_pszHost;
	int bufsize = REQUEST_BUF_SIZE;
	int tempSize;
	
	if(method == JDAWS_HTTPMETHOD_PUT) {
		httpReq << "PUT " << objectName << " " << HTTP_VERSION << "\r\n";
	} else if (method == JDAWS_HTTPMETHOD_DELETE) {
		httpReq << "DELETE " << objectName << " " << HTTP_VERSION << "\r\n";
	}
	httpReq << headers;
	if(nContentLen) {
		httpReq << "Content-Length: " << nContentLen << "\r\n";
	}

	httpReq << "Connection: Close\r\n";
	if(contentType != NULL) {
		httpReq << "Content-Type: " << contentType << "\r\n";
	}
	httpReq << "\r\n";

	std::string reqbuff = httpReq.str();
	int ret = send(sock, reqbuff.c_str(), reqbuff.length(), MSG_NOSIGNAL);
	if(ret == reqbuff.length())	{
		res = JD_OK;
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:%s : End\n",DBG_TAG, __FUNCTION__));
	return res;
}


JD_STATUS JdAwsMakeHttpRequestV4(
    JdAws_HttpMethod method,
	const char       *hostName,
	const char       *bucketName,
	const char       *objectName,
	void             *handle,
	char             *headers,
	const char       *contentType,
	int              nContentLen,
	int              timeout)
{
	JD_STATUS res = JD_ERROR;

	std::ostringstream httpReq;
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:%s : Begin\n", DBG_TAG, __FUNCTION__));
	JDAWS_CONNECTION *pConn = (JDAWS_CONNECTION *)handle;
	int sock = pConn->m_Sock;
	char *host;// = pConn->m_pszHost;
	int bufsize = REQUEST_BUF_SIZE;
	int tempSize;

	if(method == JDAWS_HTTPMETHOD_PUT) {
		httpReq << "PUT " << objectName << " " << HTTP_VERSION << "\r\n";
	} else if (method == JDAWS_HTTPMETHOD_DELETE) {
		httpReq << "DELETE " << objectName << " " << HTTP_VERSION << "\r\n";
	}
	httpReq << headers;
	if(nContentLen) {
		httpReq << "Content-Length: " << nContentLen << "\r\n";
	}

	httpReq << "Connection: Close\r\n";
	if(contentType != NULL) {
		httpReq << "Content-Type: " << contentType << "\r\n";
	}
	httpReq << "\r\n";

	std::string reqbuff = httpReq.str();
	int ret = send(sock, reqbuff.c_str(), reqbuff.length(), MSG_NOSIGNAL);
	if(ret == reqbuff.length())	{
		res = JD_OK;
	}
	JDBG_LOG(CJdDbg::LVL_TRACE, ("%s:%s : End\n",DBG_TAG, __FUNCTION__));
	return res;
}

JD_STATUS JdAwsOpenHttpConnection(const char *host, void **pHandleM, int timeoutM)
{
	int sock;										/* Socket descriptor */
	struct sockaddr_in sa;							/* Socket address */
	struct hostent *hp;								/* Host entity */
	int ret;
    int port;
    char *p;
	
    /* Check for port number specified in URL */
    p = (char *)strchr(host, ':');
    if(p) {
        port = atoi(p + 1);
        *p = '\0';
	} else {
		port = PORT_NUMBER;
	}
	hp = gethostbyname(host);
	if(hp == NULL) {  
		JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%s: gethostbyname %s failed\n",DBG_TAG, __FUNCTION__,host));
		return JD_ERROR; 
	}
		
	/* Copy host address from hostent to (server) socket address */
	memcpy((char *)&sa.sin_addr, (char *)hp->h_addr, hp->h_length);
	sa.sin_family = hp->h_addrtype;		/* Set service sin_family to PF_INET */
	sa.sin_port = htons(port);      	/* Put portnum into sockaddr */

	sock = socket(hp->h_addrtype, SOCK_STREAM, 0);
	if(sock == JD_ERROR) {  
		JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%s: socket failed\n", DBG_TAG, __FUNCTION__));
		return JD_ERROR; 
	}

	ret = connect(sock, (struct sockaddr *)&sa, sizeof(sa));
	if(ret == JD_ERROR) {
		JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%s: connect failed\n",DBG_TAG, __FUNCTION__));
		return JD_ERROR; 
	}

	JDAWS_CONNECTION *pConn = (JDAWS_CONNECTION *)malloc(sizeof(JDAWS_CONNECTION));
	pConn->m_Sock = sock;
	pConn->m_pszHost = strdup(host);
	*pHandleM = (void *)pConn;
	return JD_OK;
}



JD_STATUS JdAwsWriteHttpRequest(
	void *handle,
	char *pData,
	size_t *size,
	int nTimeOut)
{
	fd_set wfds;
	struct timeval tv;
	char headerBuf[HEADER_BUF_SIZE];

	int	selectRet;
	int bytesWritten = 0;
	int reqlen;
	/* Begin reading the body of the file */
	JDAWS_CONNECTION *pConn = (JDAWS_CONNECTION *)handle;
	int sock = pConn->m_Sock;
	int nLen = *size;
	FD_ZERO(&wfds);
	FD_SET(sock, &wfds);


	while (bytesWritten < nLen) {
		reqlen = nLen - bytesWritten;
		tv.tv_sec = nTimeOut; 
		tv.tv_usec = 0;

		if(nTimeOut > 0)
			selectRet = select(sock+1, NULL, &wfds, NULL, &tv);
		else		/* No timeout, can block indefinately */
			selectRet = select(sock+1, NULL, &wfds, NULL, NULL);

		if(selectRet == 0) {
			JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%s : select return 0\n",DBG_TAG, __FUNCTION__));
			return JD_ERROR;
		} else if(selectRet == JD_ERROR)	{
			JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%s : select failed\n",DBG_TAG, __FUNCTION__));
			return JD_ERROR;
		}

		int ret = send(sock, pData + bytesWritten, reqlen, MSG_NOSIGNAL);
		if(ret == -1)	{
			JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%s : send failed\n",DBG_TAG, __FUNCTION__));
			return JD_ERROR;
		}
		bytesWritten += ret;
		if(reqlen != ret){
			JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%s : partial write reqlen=%d written=%d\n",DBG_TAG,__FUNCTION__, reqlen, ret));
		}
	}
	return JD_OK;
}

JD_STATUS 
JdAwsEndHttpRequest(
	void *handle,
	int timeout)
{
	return JD_OK;
}

int ReadHeader(int sock, char *pBuff)
{
	fd_set rfds;
	int timeout = 30;
	struct timeval tv;
	int bytesRead = 0, newlines = 0, ret, selectRet;
	char *headerPtr = pBuff;
	while(newlines != 2 && bytesRead < HEADER_BUF_SIZE - 1) {
		FD_ZERO(&rfds);
		FD_SET(sock, &rfds);
		tv.tv_sec = timeout; 
		tv.tv_usec = 0;

		if(timeout >= 0)
			selectRet = select(sock+1, &rfds, NULL, NULL, &tv);
		else		/* No timeout, can block indefinately */
			selectRet = select(sock+1, &rfds, NULL, NULL, NULL);
		
		if(selectRet <= 0)	{
			JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%s : select failed\n",DBG_TAG, __FUNCTION__));
			goto Exit; 
		}

		ret = recv(sock, headerPtr, 1, 0);
		if(ret <= 0) {  
			JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%s : recv failed\n",DBG_TAG,__FUNCTION__));
			goto Exit; 
		}
		bytesRead++;

		if(*headerPtr == '\r'){			/* Ignore CR */

			/* Basically do nothing special, just don't set newlines
			 *	to 0 */
			headerPtr++;
			continue;
		}
		else if(*headerPtr == '\n')		/* LF is the separator */
			newlines++;
		else
			newlines = 0;

		headerPtr++;
	}
Exit:
	if(newlines == 2){
		headerPtr -= 3;		/* Snip the trailing LF's */
		*headerPtr = '\0';
	} else {
		JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%s : no new lines. may be cgi\n",DBG_TAG, __FUNCTION__));
		pBuff[bytesRead] = 0x00;
	}
	return bytesRead;
}

JD_STATUS
JdAwsGetHttpResponse(
	void *handle,
	char *headers,   
	char **contentType,
	int *contentLength,
	int *httpStatus,
	int timeout)
{
	int ret = JD_ERROR;
	char *charIndex;
	int nStatusCode;
	char headerBuf[HEADER_BUF_SIZE];

	JDAWS_CONNECTION *pConn = (JDAWS_CONNECTION *)handle;
	int sock = pConn->m_Sock;

	/* Grab enough of the response to get the metadata */
	ret = ReadHeader(sock, headerBuf);	/* errorSource set within */
	if(ret < 0) { 
		JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%s : ReadHeader Failed\n",DBG_TAG, __FUNCTION__));
		goto Exit; 
	} else if(ret == 0) {
		JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%s : Connection Closed\n",DBG_TAG, __FUNCTION__));
		goto Exit; 
	}

	/* Get the return code */
	charIndex = strstr(headerBuf, "HTTP/");
	if(charIndex == NULL) { 
		JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%s : Header Invalid\n",DBG_TAG, __FUNCTION__));
		goto Exit; 
	}

	while(*charIndex != ' ')
		charIndex++;
	charIndex++;

	ret = sscanf(charIndex, "%d", &nStatusCode);
	if(ret != 1){
		JDBG_LOG(CJdDbg::LVL_ERR, ( "%s:% : Failed to get Status Code\n",DBG_TAG,__FUNCTION__));
		goto Exit;
	}
	if(nStatusCode < 200 || nStatusCode > 307)	{
		JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%s : Status Code=%d\n",DBG_TAG,__FUNCTION__,nStatusCode));
		ret = JD_ERROR;
	} else {
		*httpStatus = nStatusCode;
	}
Exit:
	return JD_OK;
}

int JdAwsCloseHttpConnection(
	void *handle)
{
	JDAWS_CONNECTION *pConn = (JDAWS_CONNECTION *)handle;

	if(pConn){
		if(pConn->m_Sock > 0)
			close(pConn->m_Sock);
		if(pConn->m_pszHost)
			free(pConn->m_pszHost);
	}
	free(pConn);
	return 0;
}

JD_STATUS CJdAwsS3HttpConnection::MakeHttpHeaders(const CJdAwsS3Request &request,
                                                      const std::string &signature,
                                                      char **pHeader)
{
    std::string host; 
    std::ostringstream ss;

    if (request.bucketNameM.size()) {
        ss << request.bucketNameM << "." << request.hostM;
        host = ss.str();
    }
    else {
        host = request.hostM;
    }
    ss.str("");
    ss <<  "HOST: " << host << "\r\n";
    if (request.dateM.size()) {
        ss <<  "DATE: " << request.dateM << "\r\n";
    }
    ss << "AUTHORIZATION: " << "AWS " << request.pContextM->GetId() << ":" << signature << "\r\n";
    if (request.amzHeaderNamesM.size()) {
        int size = request.amzHeaderNamesM.size();
        for (int i = 0; i < size; i++) {
            ss << request.amzHeaderNamesM[i] << ": " << request.amzHeaderValuesM[i] << "\r\n";
        }
    }
    if (request.otherHeadersM.size()) {
        int size = request.otherHeadersM.size();
        for (int i = 0; i < size; i++) {
            ss << request.otherHeadersM[i];
        }
    }
    std::string tmp = ss.str();
	*pHeader = strdup( tmp.c_str());
    return JD_OK;
}


static JdAws_HttpMethod GetJdAwsHttpMethod(const CJdAwsS3Request::EJdAwsS3RequestMethod &method)
{
    switch (method)
    {
        case CJdAwsS3Request::PUT:
            return JDAWS_HTTPMETHOD_PUT;
        case CJdAwsS3Request::GET:
            return JDAWS_HTTPMETHOD_GET;
        case CJdAwsS3Request::REMOVE:
            return JDAWS_HTTPMETHOD_DELETE;
        case CJdAwsS3Request::HEAD:
            return JDAWS_HTTPMETHOD_HEAD;
        default:
            assert(0);
    }
    return JDAWS_HTTPMETHOD_GET;
}

JD_STATUS CJdAwsS3HttpConnection::MakeRequest(const CJdAwsS3Request &request,
                                                  CJdAwsS3HttpResponse &response)
{
	JD_STATUS status = JD_ERROR;
    if (request.methodM == CJdAwsS3Request::UNDEFINED || 
        !request.pContextM ||
        !request.hostM.size() || !request.pathM.size() || !request.bucketNameM.size()) {
    	JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%s:%d MakeRequest Inavlid", DBG_TAG, __FUNCTION__, __LINE__));
        return JD_ERROR_INVALID_ARG;
    }
    std::string signature;
    JD_STATUS ret = CJdAwsS3::CreateSignature(request, signature);
    if (ret != JD_OK) {
    	JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%s:%d CreateSignature Failed", DBG_TAG, __FUNCTION__, __LINE__));
        return ret;
    }
    std::string uri;
    ret = CJdAwsS3::MakeStandardUri(request, uri);
    if (ret != JD_OK) {
        return ret;
    }
    
    std::string host;
    ret = CJdAwsS3::MakeHost(request, host);
    if (ret != JD_OK) {
        return ret;
    }

    char *headers;
    ret = MakeHttpHeaders(request, signature, &headers);
    if (ret != JD_OK) 
        return ret;
	status = JdAwsOpenHttpConnection(host.c_str(), &response.pHandleM, request.timeoutM);

    if (status == JD_OK) {
        const char *pContentType = request.contentTypeM.size() > 0 ? request.contentTypeM.c_str() : NULL;
        status = JdAwsMakeHttpRequest(GetJdAwsHttpMethod(request.methodM), 
										request.hostM.c_str(), 
										request.bucketNameM.c_str(),
										request.pathM.c_str(), 
										response.pHandleM, 
										headers, 
										pContentType, 
										request.contentLengthM, 
										request.timeoutM);
	}
    free(headers);
    if (status != JD_OK) {
    	JDBG_LOG(CJdDbg::LVL_ERR, ("%s:Error issuing request to uri: %s\n", DBG_TAG, uri.c_str()));
    }
	return status;
}

#define _BUF_SIZE_ 2048

JD_STATUS CJdAwsS3HttpConnection::UploadFile(CJdAwsS3Request &request,
                                                 const char *pFilePath)
{
    request.methodM = CJdAwsS3Request::PUT;
    size_t length = get_file_size(pFilePath, NULL);
    if (length <= 0) {
    	JDBG_LOG(CJdDbg::LVL_ERR, ("%s:File not found: %s\n", DBG_TAG, pFilePath));
        return JD_ERROR;
    }
    FILE* pInput = fopen(pFilePath, "rb");
    if (!pInput) {
    	JDBG_LOG(CJdDbg::LVL_ERR, ("%s:Error opening file: %s\n", DBG_TAG, pFilePath));
        return JD_ERROR;
    }

    request.contentLengthM = length;
    CJdAwsS3HttpResponse response;
    JD_STATUS status = CJdAwsS3HttpConnection::MakeRequest(request, response);
    if (status != JD_OK) {
        return status;
    }

    char pBuf[_BUF_SIZE_];
    size_t bufSize = _BUF_SIZE_;
    size_t amountRead;
    bool fEof = false;
    int timeout = request.timeoutM;
    while (status == JD_OK && !fEof) {
        amountRead = fread(pBuf, 1, bufSize, pInput);
        if (amountRead != bufSize) {
            fEof = true;
        }
        if (amountRead > 0) {
            status = JdAwsWriteHttpRequest(response.pHandleM, pBuf, &amountRead, timeout);
        }
    }

    fclose(pInput);
    if (status == JD_OK) {
        status = JdAwsEndHttpRequest(response.pHandleM, timeout);
    }
    int httpStatus;
    if (status == JD_OK) {
        status = JdAwsGetHttpResponse(response.pHandleM, NULL, NULL, NULL,
                                         &httpStatus, timeout);
    }
    status = JD_ERROR;
    if (status == JD_OK) {
        if (httpStatus != 200) {
        	JDBG_LOG(CJdDbg::LVL_ERR, ("%s:Finished with incorrect http status: %d\n", DBG_TAG, httpStatus));
        }
        else {
            status = JD_OK;
        }
    }
    
    return status;
}

JD_STATUS CJdAwsS3HttpConnection::UploadBuffer(CJdAwsS3Request &request,
                                                   const char *pBuf,
                                                   size_t bufSize)
{
    request.methodM = CJdAwsS3Request::PUT;
    request.contentLengthM = bufSize;
    CJdAwsS3HttpResponse response;
    JD_STATUS status = CJdAwsS3HttpConnection::MakeRequest(request, response);
    if (status != JD_OK) {
    	JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%s:%d MakeRequest Failed",DBG_TAG, __FUNCTION__, __LINE__));
        return status;
    }
    status = JD_OK;
    int timeout = request.timeoutM;
    status = JdAwsWriteHttpRequest(response.pHandleM, (char*) pBuf, &bufSize, timeout);
    if (status == JD_OK) {
        status = JdAwsEndHttpRequest(response.pHandleM, timeout);
    }
    int httpStatus;
    if (status == JD_OK) {
        status = JdAwsGetHttpResponse(response.pHandleM, NULL, NULL, NULL,
                                         &httpStatus, timeout);
    }
    
    status = JD_ERROR;
    if (status == JD_OK) {
        if (httpStatus != 200) {
        	JDBG_LOG(CJdDbg::LVL_ERR, ("%s:Finished with incorrect http status: %d\n", DBG_TAG, httpStatus));
        }
        else 
            status = JD_OK;
    }
    
    return status;
}

JD_STATUS CJdAwsS3HttpConnection::DownloadFile(CJdAwsS3Request &request,
                                                   const char *pDest)
{
#ifdef COMPLETE
    request.methodM = CJdAwsS3Request::GET;
    CJdAwsS3HttpResponse response;
    JD_STATUS status = CJdAwsS3HttpConnection::MakeRequest(request, response);
    if (status != JD_OK) {
        return status;
    }
    int status = JdAwsEndHttpRequest(response.pHandleM, request.timeoutM);;
    if (status != JD_OK) {
        return JD_ERROR;
    }
    int httpStatus;
    status = JdAwsGetHttpResponse(response.pHandleM, NULL, 
                                     NULL, NULL, &httpStatus, request.timeoutM);
    if (status != JD_OK) {
        return JD_ERROR;
    }
    if (httpStatus != 200) {
        JD_LOG_ERR("Finished with incorrect http status: %d\n", httpStatus);
        return JD_ERROR;
    }
    CJdWebFileDownloadJob *pJob = CJdWebFileDownloadJob::Create(_BUF_SIZE_,
                                                                        request.timeoutM,
                                                                        response.pHandleM,
                                                                        pDest,
                                                                        NULL);
    response.pHandleM = NULL;
    if (!pJob) {
        return JD_ERROR;
    }
    (*pJob)();
    if (pJob->IsSuccess()) {
        status = JD_ERROR;
    }
    else 
        status = JD_OK;
    delete pJob;
    return status;
#else
	return JD_OK;
#endif
}

JD_STATUS CJdAwsS3HttpConnection::Delete(CJdAwsS3Request &request)
{
    request.methodM = CJdAwsS3Request::REMOVE;
    CJdAwsS3HttpResponse response;
    JD_STATUS status = CJdAwsS3HttpConnection::MakeRequest(request, response);
    if (status != JD_OK) {
    	JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%s:%d Delete Failed", DBG_TAG, __FUNCTION__, __LINE__));
        return status;
    }

    int timeout = request.timeoutM;
    if (status == JD_OK) {
        status = JdAwsEndHttpRequest(response.pHandleM, timeout);
    }
    int httpStatus;
    if (status == JD_OK) {
        status = JdAwsGetHttpResponse(response.pHandleM, NULL, NULL, NULL,
                                         &httpStatus, timeout);
    }
    
    status = JD_ERROR;
    if (status == JD_OK) {
        if (httpStatus != 204) {
        	JDBG_LOG(CJdDbg::LVL_ERR, ("%s:Finished with incorrect http status: %d\n", DBG_TAG, httpStatus));
        }
        else 
            status = JD_OK;
    }
    
    return status;
}

CJdAwsS3HttpResponse::~CJdAwsS3HttpResponse()
{
    if (pHandleM) {
        JdAwsCloseHttpConnection(pHandleM);
    }
}

