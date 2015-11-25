/*
 *  TestJdAws.cpp
 *
 *  Copyright 2011 MCN Technologies Inc. All rights reserved.
 *
 */

#include <WinSock2.h>
#include <assert.h>
#include <openssl/ssl.h>
#include <sstream>
#include <string>
#include <time.h>
#include <ctime>

// #define TEST_PUT_V4
//#define TEST_IAM_GET
#define TEST_S3_PUT_V4
#include "../../include/SegmentWriteS3.h"

#ifdef TEST_IAM_GET
#include "../../include/Iam.h"
#endif

#include "JdAwsRest.h"
#include "JdHttpClnt2.h"
#include "JdAwsConfig.h"

//#define RUN_SERVER_TESTS
#define DEBUG_JD_AWS_S3_SESSION_TEST
#ifndef DEBUG_JD_AWS_S3_SESSION_TEST
#undef JD_DEBUG_PRINT
#define JD_DEBUG_PRINT(format, ...)
#endif


//#define TEST_SIGNATURE_V4
extern int TestV4Signature();

//ithread_cond_t gCond;
//ithread_mutex_t gMutex;
bool gTestFinished = false;
//CJdWorkerThread *gpThread;
bool gUseHttps = false;

#define BUCKET_NAME "tv20"

#if 0
class CDelegate : public CJdAwsWebFileDownloadJobDelegate 
{
public:        
    virtual void OnUpdate(unsigned int length,
                          unsigned int total,
                          int status)
    {
        if (status != JD_OK) {
            assert(status == UPNP_E_FINISH);
            JD_LOG_INFO("Finished download\n");
            ithread_mutex_lock(&gMutex);
            gTestFinished = true;
            ithread_cond_broadcast(&gCond);
            ithread_mutex_unlock(&gMutex);
        }
    }
};

class CTestSignature
{
public:
    static void Run()
    {
        /* note the userId, and secret key here are dummy values obtained from
         * http://docs.amazonwebservices.com/AmazonS3/latest/dev/RESTAuthentication.html
         */
        CJdAwsContext context("0PN5J17HBGZHT7JJ3X82", 
                              "uV3F3YluFJax1cknvbcGwgjvx4QpvB+leU8dUj2o",
                              "JdAwsS3SessionTest",
                              "s3.amazonaws.com");        
        
        {
            /* Example Object GET */
            CJdAwsS3Request request(&context);
            std::string signature;
            request.pContextM = &context;
            request.methodM = CJdAwsS3Request::GET;
            request.hostM = "s3.amazonaws.com";
            request.bucketNameM = "johnsmith";
            request.pathM = "/photos/puppy.jpg";
            request.dateM = "Tue, 27 Mar 2007 19:36:42 +0000";
            JD_STATUS status = CJdAwsS3::CreateSignature(request, signature);
            assert(status == JD_OK);
            assert(signature == "xXjDGYUmKxnwqr5KXNPGldn5LbA=");
        }
        {
            /* Example Object PUT */
            CJdAwsS3Request request(&context);
            std::string signature;
            request.pContextM = &context;
            request.methodM = CJdAwsS3Request::PUT;
            request.hostM = "s3.amazonaws.com";
            request.bucketNameM = "johnsmith";
            request.pathM = "/photos/puppy.jpg";
            request.dateM = "Tue, 27 Mar 2007 21:15:45 +0000";
            request.contentLengthM = 94328;
            request.contentTypeM = "image/jpeg";
            JD_STATUS status = CJdAwsS3::CreateSignature(request, signature);
            assert(status == JD_OK);
            assert(signature == "hcicpDDvL9SsO6AkvxqmIWkmOuQ=");
        }
        {
            /* Example List */
            CJdAwsS3Request request(&context);
            std::string signature;
            request.pContextM = &context;
            request.methodM = CJdAwsS3Request::GET;
            request.hostM = "s3.amazonaws.com";
            request.bucketNameM = "johnsmith";
            request.pathM = "/";
            request.dateM = "Tue, 27 Mar 2007 19:42:41 +0000";
            request.queryStringM = "prefix=photos&max-keys=50&marker=puppy";
            JD_STATUS status = CJdAwsS3::CreateSignature(request, signature);
            assert(status == JD_OK);
            assert(signature == "jsRt/rhG+Vtp88HrYL706QhE4w4=");
        }
        {
            /* Example Fetch */
            CJdAwsS3Request request(&context);
            std::string signature;
            request.pContextM = &context;
            request.methodM = CJdAwsS3Request::GET;
            request.hostM = "s3.amazonaws.com";
            request.bucketNameM = "johnsmith";
            request.pathM = "/";
            request.dateM = "Tue, 27 Mar 2007 19:44:46 +0000";
            request.subResourceM = "acl";
            JD_STATUS status = CJdAwsS3::CreateSignature(request, signature);
            assert(status == JD_OK);
            assert(signature == "thdUi9VAkzhkniLj96JIrOPGi0g=");
        }
        {
            /* Example Delete */
            CJdAwsS3Request request(&context);
            std::string signature;
            request.pContextM = &context;
            request.methodM = CJdAwsS3Request::DELETE;
            request.hostM = "s3.amazonaws.com";
            request.bucketNameM = "johnsmith";
            request.pathM = "/photos/puppy.jpg";
            request.amzHeaderNamesM.push_back("x-amz-date");
            request.amzHeaderValuesM.push_back("Tue, 27 Mar 2007 21:20:26 +0000");
            JD_STATUS status = CJdAwsS3::CreateSignature(request, signature);
            assert(status == JD_OK);
            assert(signature == "k3nL7gH3+PadhTEVn5Ip83xlYzk=");
        }
        {
            /* Example Upload */
            CJdAwsS3Request request(&context);
            std::string signature;
            request.pContextM = &context;
            request.methodM = CJdAwsS3Request::PUT;
            request.hostM = "static.johnsmith.net:8080";
            request.bucketNameM = "static.johnsmith.net";
            request.pathM = "/db-backup.dat.gz";
            request.contentMd5M = "4gJE4saaMU4BqNR0kLY+lw==";
            request.contentTypeM = "application/x-download";
            request.dateM = "Tue, 27 Mar 2007 21:06:08 +0000";
            request.contentLengthM = 5913339;
            request.amzHeaderNamesM.push_back("x-amz-acl");
            request.amzHeaderValuesM.push_back("public-read");
            request.amzHeaderNamesM.push_back("x-amz-meta-checksumalgorithm");
            request.amzHeaderValuesM.push_back("crc32");
            request.amzHeaderNamesM.push_back("x-amz-meta-filechecksum");
            request.amzHeaderValuesM.push_back("0x02661779");
            request.amzHeaderNamesM.push_back("x-amz-meta-reviewedby");
            request.amzHeaderValuesM.push_back("joe@johnsmith.net,jane@johnsmith.net");
            request.otherHeadersM.push_back("Content-Disposition: attachment; filename=database.dat");
            request.otherHeadersM.push_back("Content-Encoding: gzip");
            JD_STATUS status = CJdAwsS3::CreateSignature(request, signature);
            assert(status == JD_OK);
            assert(signature == "C0FlOtU8Ylb9KDTpZqYkZPX91iI=");
        }
        {
            /* Example List All My Buckets */
            CJdAwsS3Request request(&context);
            std::string signature;
            request.methodM = CJdAwsS3Request::GET;
            request.hostM = "s3.amazonaws.com";
            request.pathM = "/";
            request.dateM = "Wed, 28 Mar 2007 01:29:59 +0000";
            JD_STATUS status = CJdAwsS3::CreateSignature(request, signature);
            assert(status == JD_OK);
            assert(signature == "Db+gepJSUbZKwpx1FR0DLtEYoZA=");
        }
        {
            /* Example Unicode Keys */
            CJdAwsS3Request request(&context);
            std::string signature;
            request.methodM = CJdAwsS3Request::GET;
            request.hostM = "s3.amazonaws.com";
            request.pathM = "/dictionary/fran%C3%A7ais/pr%c3%a9f%c3%a8re";
            request.dateM = "Wed, 28 Mar 2007 01:49:49 +0000";
            JD_STATUS status = CJdAwsS3::CreateSignature(request, signature);
            assert(status == JD_OK);
            assert(signature == "dxhSBHoI6eVSPcXJqEghlUzZMnY=");
        }
        {
            /* Example Query String Request Authentication */
            CJdAwsS3Request request(&context);
            std::string signature;
            request.pContextM = &context;
            request.methodM = CJdAwsS3Request::GET;
            request.hostM = "s3.amazonaws.com";
            request.bucketNameM = "johnsmith";
            request.pathM = "/photos/puppy.jpg";
            request.expiresM = "1175139620";
            JD_STATUS status = CJdAwsS3::CreateSignature(request, signature);
            assert(status == JD_OK);
            char *pSignature = jd_url_encode(signature.c_str());
            assert(strcmp(pSignature, "rucSbH0yNEcP9oM2XNlouVI3BH4%3D") == 0);
        }
    }
};

class CTestGet
{
public:
#if 0
    static void Run(CJdAwsContext *pContext)
    {
        JD_DEBUG_PRINT("Making GET Request\n");
        CJdAwsS3Request request(pContext);
        request.methodM = CJdAwsS3Request::GET;
        request.pContextM = pContext;
        request.hostM.assign("s3.amazonaws.com");
        request.pathM.assign("/test1.txt");
        request.bucketNameM.assign("ntvcam");
        CJdAwsContext::GetCurrentDate(request.dateM);
        request.fUseHttpsM = gUseHttps;
        CJdAwsS3HttpResponse response;
        JD_STATUS status = CJdAwsS3HttpConnection::MakeRequest(request, response);
        assert(status == JD_OK);
        assert(response.pHandleM);
        int upnpstatus = JdAwsEndHttpRequest(response.pHandleM, request.timeoutM);
        assert(upnpstatus == JD_OK);
        int httpStatus;
        upnpstatus = JdAwsGetHttpResponse(response.pHandleM, NULL, 
                                         NULL, NULL, &httpStatus, request.timeoutM);
        assert(upnpstatus == JD_OK);
        JD_DEBUG_PRINT("Finished Making GET Request :%d\n", httpStatus);
        assert(httpStatus == 200);
        CDelegate *pDelegate = new CDelegate();
        CJdWebFileDownloadJob *pJob = CJdWebFileDownloadJob::Create(1024,
                                                                            30,
                                                                            response.pHandleM,
                                                                            "/tmp/dest.txt",
                                                                            pDelegate);
        assert(pJob);
        response.pHandleM = NULL;
        gpThread->AddJob(pJob);
    }
#endif
    static void Run2(CJdAwsContext *pContext)
    {
        JD_DEBUG_PRINT("Making GET Request\n");
        CJdAwsS3Request request(pContext);
        request.pContextM = pContext;
        request.hostM.assign("s3.amazonaws.com");
        request.pathM.assign("/test1.txt");
        request.bucketNameM.assign("ntvcam");
        CJdAwsContext::GetCurrentDate(request.dateM);
        request.fUseHttpsM = gUseHttps;
        JD_STATUS status = CJdAwsS3HttpConnection::DownloadFile(request, "/tmp/dest.txt");
        assert(status == JD_OK);
        JD_DEBUG_PRINT("Finished Making GET Request");
    }
};
#endif

class CTestPut
{
public:
    static void Run(CJdAwsContext *pContext)
    {
        const char* contentType = CONTENT_STR_MP2T;
        int httpStatus;
        const char *pBuf = "blahblahblah";
        size_t size = strlen(pBuf) + 1;
        int timeout = 30;
        CJdAwsS3Request request(pContext);
        request.methodM = CJdAwsS3Request::PUT;
        request.pContextM = pContext;
        request.hostM.assign("s3.amazonaws.com");
        request.pathM.assign("/test1.txt");
        request.bucketNameM.assign(BUCKET_NAME);
        request.contentTypeM.assign(contentType);
        request.contentLengthM = size;
        request.fUseHttpsM = gUseHttps;
		/*
        CJdAwsContext::GetCurrentDate(request.dateM);
		*/
        CJdAwsS3HttpResponse response;
        JD_STATUS status = CJdAwsS3HttpConnection::MakeRequest(request, response);
        assert(status == JD_OK);
        assert(response.pHandleM);
        status = JdAwsWriteHttpRequest(response.pHandleM, (char*) pBuf, &size, 30);
        assert(status == JD_OK);
        status = JdAwsEndHttpRequest(response.pHandleM, timeout);
        assert(status == JD_OK);
        status = JdAwsGetHttpResponse(response.pHandleM, NULL, NULL, NULL,
                                         &httpStatus, timeout);
        assert(status == JD_OK);
        assert(httpStatus == 200);
    }

#if 0
    static void Run2(CJdAwsContext *pContext)
    {
        const char* contentType = "mpeg/ts";
        JD_DEBUG_PRINT("Making PUT Request\n");
        const char *pBuf = "blahblahblah";
        size_t size = strlen(pBuf) + 1;
        CJdAwsS3Request request(pContext);
        request.pContextM = pContext;
        request.hostM.assign("s3.amazonaws.com");
        request.pathM.assign("/test1.txt");
        request.bucketNameM.assign("ntvcam");
        request.contentTypeM.assign(contentType);
        request.contentLengthM = size;
        request.fUseHttpsM = gUseHttps;
        CJdAwsContext::GetCurrentDate(request.dateM);
        JD_STATUS status = CJdAwsS3HttpConnection::UploadBuffer(request, pBuf, size);
        assert(status == JD_OK);
        JD_DEBUG_PRINT("Finished Making PUT Request\n");
    }
    static void Run3(CJdAwsContext *pContext)
    {
        const char* contentType = "mpeg/ts";
        JD_DEBUG_PRINT("Making PUT Request\n");
        CJdAwsS3Request request(pContext);
        request.pContextM = pContext;
        request.hostM.assign("s3.amazonaws.com");
        request.pathM.assign("/test1.txt");
        request.bucketNameM.assign("ntvcam");
        request.contentTypeM.assign(contentType);
        request.fUseHttpsM = gUseHttps;
        CJdAwsContext::GetCurrentDate(request.dateM);
        JD_STATUS status = CJdAwsS3HttpConnection::UploadFile(request, "/tmp/dest.txt");
        assert(status == JD_OK);
        JD_DEBUG_PRINT("Finished Making PUT Request\n");
    }
#endif
};

extern int TestV4Signature();
#if 0
class CTestDelete
{
public:
    static void Run(CJdAwsContext *pContext)
    {
        JD_DEBUG_PRINT("Making DELETE Request\n");
        CJdAwsS3Request request(pContext);
        request.methodM = CJdAwsS3Request::DELETE;
        request.pContextM = pContext;
        request.hostM.assign("s3.amazonaws.com");
        request.pathM.assign("/test1.txt");
        request.bucketNameM.assign("ntvcam");
        CJdAwsContext::GetCurrentDate(request.dateM);
        request.fUseHttpsM = gUseHttps;
        CJdAwsS3HttpResponse response;
        JD_STATUS status = CJdAwsS3HttpConnection::MakeRequest(request, response);
        assert(status == JD_OK);
        assert(response.pHandleM);
        int upnpstatus = JdAwsEndHttpRequest(response.pHandleM, request.timeoutM);
        int httpStatus;
        assert(upnpstatus == JD_OK);
        upnpstatus = JdAwsGetHttpResponse(response.pHandleM, NULL, 
                                         NULL, NULL, &httpStatus, request.timeoutM);
        assert(upnpstatus == JD_OK);
        JD_DEBUG_PRINT("Finished Making DELETE Request: %d\n", httpStatus);
        assert(204 == httpStatus);
    }
    static void Run2(CJdAwsContext *pContext)
    {
        JD_DEBUG_PRINT("Making DELETE Request\n");
        CJdAwsS3Request request(pContext);
        request.pContextM = pContext;
        request.hostM.assign("s3.amazonaws.com");
        request.pathM.assign("/test1.txt");
        request.bucketNameM.assign("ntvcam");
        CJdAwsContext::GetCurrentDate(request.dateM);
        request.fUseHttpsM = gUseHttps;
        JD_STATUS status = CJdAwsS3HttpConnection::Delete(request);
        assert(status == JD_OK);
        JD_DEBUG_PRINT("Finished Making DELETE Request\n");
    }
};
#endif

int main(int argc, const char *argv[])
{

		WSADATA wsaData;
	int iResult;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", iResult);
        return 1;
    }

#if defined(TEST_PUT_V2) || defined(TEST_PUT_V4) 
	const char *pszConfigFile = argv[1];
    char *pBuf = "blahblahblah";
    size_t size = strlen(pBuf) + 1;

	WSADATA wsaData;
	int iResult;

	CJdAwsConfig JdAwsConfig(pszConfigFile);
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        fprintf(stderr, "WSAStartup failed: %d\n", iResult);
        return 1;
    }
#endif
#if 0
    CJdAwsContext context(pId, pSecKey, NULL, "s3.amazonaws.com");
	CTestPut::Run(&context);
#endif
#ifdef TEST_PUT_V2
	CSegmentWriteS3 HlsOut(JdAwsConfig.m_Bucket.c_str(), JdAwsConfig.m_Host.c_str(), JdAwsConfig.m_AccessId.c_str(), JdAwsConfig.m_SecKey.c_str());
	HlsOut.Send("Folder1", "File1", pBuf, size, CONTENT_STR_DEF, 30);
#endif

#ifdef TEST_IAM_GET

	//20110909T233600
    struct std::tm t;
    t.tm_sec = 0; t.tm_min = 36; t.tm_hour = 4;
    t.tm_mon = 8 - 1; t.tm_year = 2015 - 1900; t.tm_isdst = 0; t.tm_mday = 30;   
    const std::time_t request_date = std::mktime(&t);
	const char *pPayload = "";
	CIam iam("iam.amazonaws.com", "AKIDEXAMPLE%2F20150830", "wJalrXUtnFEMI/K7MDENG+bPxRfiCYEXAMPLEKEY");
	iam.Get("Action=ListUsers&Version=2010-05-08", "application/x-www-form-urlencoded; charset=utf-8", request_date, pPayload, strlen(pPayload), 10);
#endif
#ifdef TEST_S3_PUT_V4
    const std::time_t request_date = std::time(nullptr);
	char *pPayload = "TestPayLoad";
	CSegmentWriteS3 S3("educast", "s3.amazonaws.com", "AKIAIDBKBQLSL6W7W2SA", "bQMJdOWWe/nKrVhVEFMybCJcZNxg0tZVhU99Agbc", 1);
	S3.Send("TestFolderV4", "TestFile", request_date, pPayload, strlen(pPayload), CONTENT_STR_DEF, 30);
#endif
#ifdef TEST_SIGNATURE_V4
	TestV4Signature();
#endif
	return 0;
}


