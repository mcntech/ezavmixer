/*
 *  JdAwsS3HttpConnection.h
 *
 *  Copyright 2011 MCN Technologies Inc.. All rights reserved.
 *
 */

#ifndef __JD_AWS_S3_UPNP_HTTP_CONNECTION_H__
#define __JD_AWS_S3_UPNP_HTTP_CONNECTION_H__

#include "JdAwsContext.h"
#include "JdAwsS3Request.h"



#include "JdAwsS3Request.h"

typedef enum
{
	JD_OK,
	JD_ERROR = 0x80000000,
	JD_ERROR_INVALID_ARG  = 0x80000001,
	JD_ERROR_CONNECTION   = 0x80000002,
	JD_ERROR_PUT_WRITE    = 0x80000003,
	JD_ERROR_PUT_RESPONSE = 0x80000004,
	JD_ERROR_PUT_REJECT   = 0x80000005
} JD_STATUS;

class CJdAwsS3
{
public:
    /**
     * Creates a signature for use with the Amazon S3 service.
     *
     * @note In case of query authentication, the returned signatured won't be URL encoded.
     * It's the caller's responsibility to do that
     *
     * @return JD_OK if successful.
     *         JD_ERROR_INVALID_ARG If a required parameter in the request object is not present.
     *         JD_ERROR  otherwise.
     */
    static JD_STATUS CreateSignature(const CJdAwsS3Request &request,
                                     /*OUT*/ std::string &signature);

    static JD_STATUS CreateSignatureV4(const CJdAwsS3Request &request,
                                        /*OUT*/ std::string &signature);

    /**
     * Makes a standard URI for issue requests programatically.
     */
    static JD_STATUS MakeStandardUri(const CJdAwsS3Request &request,
                                     /*OUT*/ std::string &uri);

	static JD_STATUS MakeHost(const CJdAwsS3Request &request,
                                    /*OUT*/ std::string &host);
};

typedef enum 
{
	JDAWS_HTTPMETHOD_PUT, 
	JDAWS_HTTPMETHOD_GET, 
	JDAWS_HTTPMETHOD_DELETE, 
	JDAWS_HTTPMETHOD_HEAD 
}
JdAws_HttpMethod;
/**
 * Contains the information sent by the Amazon S3 service in response to a request.
 */
class CJdAwsS3HttpResponse
{
public:
    CJdAwsS3HttpResponse() : pHandleM(NULL) {}
    ~CJdAwsS3HttpResponse();

    /** 
     * The handle to the underlying HTTP stack, for further interaction. 
     * This object owns the handle. To take ownership of the handle from this 
     * object, set the field to NULL. */
    void *pHandleM;  
};

class CJdAwsS3HttpConnection 
{
public:
    /**
     * Concactenates request headers for use withh the pupnp HTTP Client API.
     *
     * @param signature The signature of the request
     */
    static JD_STATUS MakeHttpHeaders(const CJdAwsS3Request &request,
                                     const std::string &signature,
                                     char **headers);
    
    /**
     * Makes a request to the Amazon S3 service
     *
     * The response object will be initialized with responses from the server if the request
     * has 0 content length. Otherwise the caller is expected to use the HTTP SDK to process
     * the response from the server.
     * 
     * @return JD_OK if successful. 
     *         JD_ERROR_INVALID_ARG If a required parameter in the request object is not present. 
     *         JD_ERROR  otherwise.
     */
    static JD_STATUS MakeRequest(const CJdAwsS3Request &request,
                                 /*OUT*/ CJdAwsS3HttpResponse &response);

    /**
     * Uploads a file to the Amazon S3 service. 
     *
     * @param request Contains information for making the request
     *
     * @return JD_OK if successful.
     *         JD_ERROR otherwise.
     */
    static JD_STATUS UploadFile(CJdAwsS3Request &request,
                                const char *pFilePath);
    
    /**
     * Uploads a buffer to the Amazon S3 service. 
     *
     * @param request Contains information for making the request
     *
     * @return JD_OK if successful.
     *         JD_ERROR otherwise.
     */
    static JD_STATUS UploadBuffer(CJdAwsS3Request &request,
                                  const char *pBuf,
                                  size_t bufSize);
    
    /**
     * @return A non-NULL pointer if sucessful. NULL otherwise.
     */
    static JD_STATUS DownloadFile(CJdAwsS3Request &request,
                                  const char *pDest);
    
    /**
     * Deletes the resource (bucket or object) specified in the request.
     *
     * @return JD_OK if successful.
     *         JD_ERROR otherwise.
     */
    static JD_STATUS Delete(CJdAwsS3Request &request);

};

JD_STATUS JdAwsWriteHttpRequest(
	void *handle,
	const char *buf,
	size_t *size,
	int timeout);

JD_STATUS JdAwsEndHttpRequest(
	void *handle,
	int timeout);

JD_STATUS JdAwsGetHttpResponse(
	void *handle,
	char *headers,   
	char **contentType,
	int *contentLength,
	int *httpStatus,
	int timeout);

int JdAwsCloseHttpConnection(
	void *handle);

#endif // __JD_AWS_S3_UPNP_HTTP_CONNECTION_H__
