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
//#include "JdAwsS3.h"
#include "JdAwsRest.h"
#include "../include/SegmentWriteS3.h"
#include "JdDbg.h"
#include "awsv4.h"

#define PORT_NUMBER 			80
#define HTTP_VERSION 			"HTTP/1.1"
#define DEFAULT_USER_AGENT		"HTTP OnyxCloudRecorder"
#define VERSION					"1.1"
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


//=====================================================================


#define ENABLE_OPENSSL

static inline const char* MethodToStr(const CJdAwsS3Request::EJdAwsS3RequestMethod &method)
{
    switch (method)
    {
        case CJdAwsS3Request::PUT:
            return "PUT";
        case CJdAwsS3Request::GET:
            return "GET";
        case CJdAwsS3Request::REMOVE:
            return "DELETE";
        case CJdAwsS3Request::HEAD:
            return "HEAD";
        default:
            assert(0);
    }
    return NULL;
}

JD_STATUS CJdAwsS3::CreateSignature(const CJdAwsS3Request &request,
                                    /*OUT*/ std::string &authorization)
{
	 std::string signature;
    if (request.methodM == CJdAwsS3Request::UNDEFINED ||
        !request.pContextM ||
        !request.pathM.size() ||
        request.amzHeaderNamesM.size() != request.amzHeaderValuesM.size()) {
        return JD_ERROR_INVALID_ARG;
    }

    std::ostringstream ss;
    //
    // Determine the signature
    //
    // For query authentication:
    //
    // StringToSign = HTTP-Verb + "\n" +
    // Content-MD5 + "\n" +
	// Content-Type + "\n" +
	// Expires + "\n" +
	// CanonicalizedAmzHeaders +
	// CanonicalizedResource;
    //
    // For normal authentication:
    //
    // StringToSign = HTTP-Verb + "\n" +
    // Content-MD5 + "\n" +
	// Content-Type + "\n" +
	// Date + "\n" +
	// CanonicalizedAmzHeaders +
	// CanonicalizedResource;
    //
    const char *pMethod = MethodToStr(request.methodM);
    ss << pMethod << "\n";

	if(request.contentMd5M.length()){
		ss << request.contentMd5M;
	}
    ss << "\n";

	if(request.contentTypeM.length()) {
		ss << request.contentTypeM;
	}
    ss << "\n";

	if (request.expiresM.size())
        ss << request.expiresM;
    else {
		std::string reqDate = CJdAwsContext::GetUTCString(request.dateM);
        ss << reqDate;
	}
    ss << "\n";

    if (request.amzHeaderNamesM.size() > 0) {
        int size = request.amzHeaderNamesM.size();
        for (int c = 0; c < size; c++) {
            ss << request.amzHeaderNamesM[c] << ":" << request.amzHeaderValuesM[c] << "\n";
        }
    }

	if (request.bucketNameM.size()) {
        ss << "/" << request.bucketNameM;
    }
    ss << request.pathM;

	if (request.subResourceM.size()) {
        ss << "?" << request.subResourceM;
    }

	signature = ss.str();

	unsigned char pEncryptedResult[EVP_MAX_MD_SIZE];
    unsigned int encryptedResultSize;
    //
    // Compute signature
    //

    HMAC(EVP_sha1(), request.pContextM->GetSecretKey().c_str(),
         request.pContextM->GetSecretKey().size(),
         (const unsigned char*) signature.c_str(), signature.size(),
         pEncryptedResult, &encryptedResultSize);
	CJdAwsContext::ToBase64((const char*) pEncryptedResult, encryptedResultSize, signature);

	authorization = "AUTHORIZATION: AWS ";
	authorization += request.pContextM->GetId() + ":" + signature;
	return JD_OK;
}

JD_STATUS CJdAwsS3::CreateSignatureV4(const CJdAwsS3Request &request,
                                    /*OUT*/ std::string &authorization)
{
	std::string signature;
    /*
    Step 1 is to define the verb (GET, POST, etc.)--already done.
    StringToSign = HTTP-Verb + "\n" +
    */
    std::string method;
	if(request.methodM == request.PUT) {
		method = "PUT";
	} else if (request.methodM == request.GET) {
		method = "GET";
	} else if (request.methodM == request.REMOVE) {
		method = "DLETE";
	}

	/*
    Step 2: Create canonical URI
    */
    const std::string base_uri(request.hostM);
    const std::string path(request.bucketNameM + "/" + request.pathM);

    /*
	Step 3: Create the canonical query string.
    */
    const std::string query_args(request.queryStringM);


    const std::string uri_str(/*base_uri +*/ path + "?" + query_args);
    URI uri;
    uri = URI(uri_str);
    //uri.normalize();
    std::string canonical_uri = canonicalize_uri(uri);
    std::string canonical_query = canonicalize_query(uri);

    /*
    Step 4: Create the canonical headers and signed headers.
    and value must be trimmed and lowercase, and sorted in ASCII order.
	Note that there is a trailing \n.
    canonical_headers = 'host:' + host + '\n' + 'x-amz-date:' + amzdate + '\n'
    */
    std::vector<std::string> headers;
    headers.push_back("host:" + request.hostM);
    headers.push_back("Content-type:" + request.contentTypeM);
    headers.push_back("x-amz-date:" + ISO8601_date(request.dateM));
	headers.push_back("x-amz-content-sha256:" + request.contentSha256);

    std::map<std::string,std::string> canonical_headers_map;
	canonicalize_headers(headers, canonical_headers_map);

    std::string headers_string = map_headers_string(canonical_headers_map);

    // Step 5: Create the list of signed headers
	// # in the canonical_headers list, delimited with ";" and in alpha order.
	// # Note: The request can include any headers; canonical_headers and
	// # signed_headers lists those that you want to be included in the
	// # hash of the request. "Host" and "x-amz-date" are always required.
    // signed_headers = 'host;x-amz-date'
    //

    std::string signed_headers = map_signed_headers(canonical_headers_map);

    // Step 6: Create payload hash
    //
    //std::string payload = "UNSIGNED-PAYLOAD";
    std::string sha256_payload = request.contentSha256;


    // Step 7: Combine elements to create create canonical request
    // canonical_request = method + '\n' + canonical_uri + '\n' + canonical_querystring
    // + '\n' + canonical_headers + '\n' + signed_headers + '\n' + payload_hash
    //
    // Content-MD5 + "\n" +
	// Content-Type + "\n" +
	// Expires + "\n" +
	// CanonicalizedAmzHeaders +
	// CanonicalizedResource;
    //
    // For normal authentication:
    //
    // StringToSign = HTTP-Verb + "\n" +
    // Content-MD5 + "\n" +
	// Content-Type + "\n" +
	// Date + "\n" +
	// CanonicalizedAmzHeaders +
	// CanonicalizedResource;
    //

    std::string canonical_request = canonicalize_request(method,
                                                               canonical_uri,
                                                               canonical_query,
                                                               headers_string,
                                                               signed_headers,
															   sha256_payload);



    //Step 8. Create a digest (hash) of the canonical request by using the same algorithm that you used to hash
    //the payload.The hashed canonical request must be represented as a string of lowercase hexademical
    //characters.
	// f536975d06c0309214f805bb90ccff089219ecd68b2577efef23edd43b7e1a59
    std::string hashed_canonical_request = sha256_base16(canonical_request);


    /*
    Step 2.1 Start with the algorithm designation, followed by a newline character. This value is the hashing
    algorithm that you're using to calculate the digests in the canonical request. (For SHA256,
    AWS4-HMAC-SHA256 is the algorithm.)
    AWS4-HMAC-SHA256\n
	*/
    const std::string  STRING_TO_SIGN_ALGO = "AWS4-HMAC-SHA256";
    /*
    Step 2.2
    Append the request date value, followed by a newline character. The date is specified by using the
    ISO8601 Basic format via the x-amz-date header in the YYYYMMDD'T'HHMMSS'Z' format. This
    value must match the value you used in any previous steps.
    const char *pMethod = MethodToStr(request.methodM);
    ss << pMethod << "\n";
    20110909T233600Z\n
    */

    const std::time_t request_date = request.dateM;

    /*
    2.3 Append the credential scope value, followed by a newline character.This value is a string that includes
    the date (just the date, not the date and time), the region you are targeting, the service you are
    requesting, and a termination string ("aws4_request") in lowercase characters. The region and
    service name strings must be UTF-8 encoded.
    20110909/us-east-1/iam/aws4_request\n
	*/
    const std::string region(request.regionM);
    const std::string service(request.serviceM);

    std::string my_credential_scope = credential_scope(request_date,region,service);

    /*
    2.4 Append the hash of the canonical request that you created in task 1. This value is not followed by a
    newline character. The hashed canonical request must be lowercase base-16 encoded, as defined
    by Section 8 of RFC 4648.
    e.g. 3511de7e95d28ecd39e9513b642aee07e54f4941150d8df8bf94b328ef7e55e2
	*/

    /*
     AWS4-HMAC-SHA256
	20110909T233600Z
	20110909/us-east-1/iam/aws4_request
	3511de7e95d28ecd39e9513b642aee07e54f4941150d8df8bf94b328ef7e55e2
    */
    std::string my_string_to_sign = string_to_sign(STRING_TO_SIGN_ALGO,
                                                 request_date,
                                                 my_credential_scope,
                                                 hashed_canonical_request);
    /*
    3.1 Derive your signing key. To do this, you use your secret access key to create a series of hash-based
    message authentication codes (HMACs) as shown by the following pseudocode, where HMAC(key,
    data) represents an HMAC-SHA256 function that returns output in binary format.The result of each
    hash function becomes input for the next one

    kSecret = Your AWS Secret Access Key
	kDate = HMAC("AWS4" + kSecret, Date)
	kRegion = HMAC(kDate, Region)
	kService = HMAC(kRegion, Service)
	kSigning = HMAC(kService, "aws4_request")

	HMAC(HMAC(HMAC(HMAC("AWS4" + kSecret,"20110909"),"us-east-1"),"iam"),"aws4_request")

	196 175 177 204 87 113 216 113 118 58 57 62 68 183 3 87 27 85 204 40 66 77
	26 94 134 218 110 211 193 84 164 185
	*/
    /*
    3.2 Calculate the signature. To do this, use the signing key that you derived and the string to sign as
    inputs to the keyed hash function. After you've calculated the signature as a digest, convert the binary
    value to a hexadecimal representation.
	signature = HexEncode(HMAC(derived-signing-key, string-to-sign))

	e.g. 5d672d79c15b13162d9279b0855cfba6789a8edb4c82c400e06b5924a6f2b5d7
    */

    const std::string secret = request.pContextM->GetSecretKey();//"wJalrXUtnFEMI/K7MDENG+bPxRfiCYEXAMPLEKEY";
    signature = calculate_signature(request_date,
									 secret,
									 region,
									 service,
									 my_string_to_sign);

    /*
    4. You can include signing information by adding it to header named Authorization. The contents of the
    header are created after you have calculated the signature as described in the preceding steps, so the
    Authorization header is not included in the list of signed headers.
    The following pseudocode shows the construction of the Authorization header.

    Authorization: algorithm Credential=access key ID/credential scope, SignedHead
	ers=SignedHeaders, Signature=signature

	the authorization header would appear as a continuous line of text. The version below has been formatted
	for readability
	e.g.
	Authorization: AWS4-HMAC-SHA256
	Credential=AKIDEXAMPLE/20110909/us-east-1/iam/aws4_request,
	SignedHeaders=content-type;host;x-amz-date,
	Signature=5d672d79c15b13162d9279b0855cfba6789a8edb4c82c400e06b5924a6f2b5d7

	*/
	std::string delim_space = " ";
	std::string delim_comma = ",";
	authorization = "AUTHORIZATION: AWS4-HMAC-SHA256" + delim_space;
		authorization += "Credential=" +  my_credential_scope + delim_comma;
		authorization += "SignedHeaders=" + signed_headers + delim_comma;
		authorization += "Signature=" + signature;

	return JD_OK;
}


JD_STATUS CJdAwsS3::MakeStandardUri(const CJdAwsS3Request &request,
                                    /*OUT*/ std::string &uri)
{
    std::ostringstream ss;
    std::string queryString;
    ss << request.subResourceM;
    if (request.subResourceM.size() && request.queryStringM.size()) {
        ss << "&";
    }
    ss << request.queryStringM;
    queryString = ss.str();
    ss.str("");
    if (request.fUseHttpsM)
        ss << "https://";
    else
        ss << "http://";
	if(request.bucketNameM.length())
		ss << request.bucketNameM << "." ;
	ss << request.hostM  <<  request.pathM;
    if (queryString.size()) {
		std::string Encode;
		URI::encode(queryString,"",Encode);
        ss << "?" << Encode;
    }
    uri = ss.str();
    return JD_OK;
}

JD_STATUS CJdAwsS3::MakeHost(const CJdAwsS3Request &request,
                                    /*OUT*/ std::string &host)
{
    std::ostringstream ss;

	if(request.bucketNameM.length())
		ss << request.bucketNameM << "." ;
	ss << request.hostM;
    host = ss.str();
    return JD_OK;
}

//=============================================================================================
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
	printf("%s\n", reqbuff.c_str());
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
	const char *pData,
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
	int nStatusCode = 0;
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
	}
	*httpStatus = nStatusCode;
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

	if(request.signatureVresionM == request.V4) {
        ss <<  "DATE: " << ISO8601_date(request.dateM) << "\r\n";
	} else {
		std::string dateOut;
		dateOut = CJdAwsContext::GetUTCString(request.dateM);
		ss <<  "DATE: " << dateOut << "\r\n";
	}
	ss << signature << "\r\n";
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
        !request.hostM.size()) {
    	JDBG_LOG(CJdDbg::LVL_ERR, ("%s:%s:%d MakeRequest Inavlid", DBG_TAG, __FUNCTION__, __LINE__));
        return JD_ERROR_INVALID_ARG;
    }
	std::string authorization;

  JD_STATUS ret = JD_ERROR;
    if(request.signatureVresionM == request.V2)
    	ret = CJdAwsS3::CreateSignature(request, authorization);
    else if (request.signatureVresionM == request.V4)
    	ret = CJdAwsS3::CreateSignatureV4(request, authorization);
  
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
	ret = MakeHttpHeaders(request, authorization, &headers);

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

