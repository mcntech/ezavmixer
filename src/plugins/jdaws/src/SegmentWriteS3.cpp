
#include "../include/SegmentWriteS3.h"
#include "awsv4.h"

CSegmentWriteS3::CSegmentWriteS3(
			CJdAwsContext *pAwsContext,
			const char *pszBucket
			/*const char *pszHost,
			const char *pszAccessId,
			const char *pszSecKey,
			int         signVersion*/)
		: m_AwsContext(*pAwsContext/*pszAccessId, pszSecKey, signVersion, NULL, pszHost*/)
{
	m_Bucket.assign(pszBucket);
}


int CSegmentWriteS3::Start(
			const char *pszParent,
			const char *pszFile,
			int        nTotalLen,	/* Total length of content including subsequent continue */
			char       *pData,
			int        nLen,		/* Length of data of this call */
			const char *pContentType
			)
{
    int    httpStatus;
	int nTimeOut = 30;
	size_t size = nLen;
	CJdAwsS3Request request(&m_AwsContext);

	request.methodM = CJdAwsS3Request::PUT;

	if(pContentType != NULL && strlen(pContentType)){
		request.contentTypeM.assign(pContentType);
	} else {
		request.contentTypeM.assign(CONTENT_STR_DEF);
	}

	if (pszParent) {
		std::ostringstream objectname;
		objectname << "/" << pszParent << "/" << pszFile;
		request.pathM.assign(objectname.str());
	} else {
		request.pathM.assign(pszFile);
	}
	request.bucketNameM.assign(m_Bucket);

	request.contentLengthM = nTotalLen;
	request.fUseHttpsM = false; //gUseHttps;

	//CJdAwsContext::GetCurrentDate(request.dateM);

	JD_STATUS status = CJdAwsS3HttpConnection::MakeRequest(request, m_SessionContext);
	if(status != JD_OK) goto Exit;

	if(pData) {
		status = JdAwsWriteHttpRequest(m_SessionContext.pHandleM,  pData, &size, 30);
	}

Exit:
    return status;
}

int CSegmentWriteS3::Continue(char *pData, int nLen)
{
    int			httpStatus;
	JD_STATUS	status;
	int nTimeOut = 30;
	size_t size = nLen;
    status = JdAwsWriteHttpRequest(m_SessionContext.pHandleM,  pData, &size, 30);
	return (status == JD_OK);
}

int CSegmentWriteS3::End(char *pData, int nLen)
{
    int			httpStatus;
	JD_STATUS	status;
	int nTimeOut = 30;
	size_t size = nLen;
	if(pData && nLen > 0) {
		status = JdAwsWriteHttpRequest(m_SessionContext.pHandleM,  pData, &size, 30);
		if(status != JD_OK) goto Exit;
	}
    status = JdAwsEndHttpRequest(m_SessionContext.pHandleM, nTimeOut);
	if(status != JD_OK) goto Exit;
    status = JdAwsGetHttpResponse(m_SessionContext.pHandleM, NULL, NULL, NULL,
                                     &httpStatus, nTimeOut);
	JdAwsCloseHttpConnection(m_SessionContext.pHandleM);
	m_SessionContext.pHandleM = 0;

Exit:
    return(status == JD_OK);
}

int CSegmentWriteS3::Send(const char *pszParent, const char *pszFile, const std::time_t request_date, char *pData, int nLen, const char *pContentType, int nTimeOut)
{
	int result = JD_OK;
    int httpStatus;
	size_t size = nLen;
	//return HttpPutResource(pszParent, pszFile, pData, nMaxLen, nTimeOut, CONTENT_TYPE_HTML, AMZ_ACL_PUBLIC_READ_WRITE);

	CJdAwsS3Request request(&m_AwsContext);

	request.methodM = CJdAwsS3Request::PUT;
	request.pContextM = &m_AwsContext;
	//request.hostM.assign("s3.amazonaws.com");
	request.contentSha256 = sha256_base16(pData);

	if(pContentType != NULL && strlen(pContentType)){
		request.contentTypeM.assign(pContentType);
	} else {
		request.contentTypeM.assign(CONTENT_STR_DEF);
	}

	request.serviceM = "s3";
	request.queryStringM = "";

	if (pszParent) {
		std::ostringstream objectname;
		objectname << "/" << pszParent << "/" << pszFile;
		request.pathM.assign(objectname.str());
	} else {
		request.pathM.assign(pszFile);
	}
	request.bucketNameM.assign(m_Bucket);

	request.contentLengthM = size;
	request.fUseHttpsM = false; //gUseHttps;
	request.dateM = request_date;

	CJdAwsS3HttpResponse response;
	JD_STATUS status = CJdAwsS3HttpConnection::MakeRequest(request, response);
    if(status != JD_OK){
		result = JD_ERROR_CONNECTION;
		goto Exit;
	}
    assert(response.pHandleM);
    status = JdAwsWriteHttpRequest(response.pHandleM,  pData, &size, 30);
    if(status != JD_OK){
		result = JD_ERROR_PUT_WRITE;
		goto Exit;
	}
    status = JdAwsEndHttpRequest(response.pHandleM, nTimeOut);
    assert(status == JD_OK);
    status = JdAwsGetHttpResponse(response.pHandleM, NULL, NULL, NULL,
                                     &httpStatus, nTimeOut);

    if(status != JD_OK){
		result = JD_ERROR_PUT_RESPONSE;
		goto Exit;
	}
    if(httpStatus != 200){
		result = JD_ERROR_PUT_REJECT;
	}
Exit:
	return result;
}

int CSegmentWriteS3::Delete(const char *pszParent, const char *pszFile)
{
    int    httpStatus;
	int nTimeOut = 30;
	CJdAwsS3Request request(&m_AwsContext);

	request.methodM = CJdAwsS3Request::REMOVE;

	if (pszParent) {
		std::ostringstream objectname;
		objectname << "/" << pszParent << "/" << pszFile;
		request.pathM.assign(objectname.str());
	} else {
		request.pathM.assign(pszFile);
	}
	request.bucketNameM.assign(m_Bucket);

	request.fUseHttpsM = false; //gUseHttps;
	/*
	TODO
	CJdAwsContext::GetCurrentDate(request.dateM);
		*/
	JD_STATUS status = CJdAwsS3HttpConnection::MakeRequest(request, m_SessionContext);

	JdAwsCloseHttpConnection(m_SessionContext.pHandleM);
	m_SessionContext.pHandleM = 0;

	if(status != JD_OK) goto Exit;

Exit:
    return status;
}
