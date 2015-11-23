
#include "../include/Iam.h"
#include "awsv4.h"

#define CONTENT_STR_DEF "application/x-www-form-urlencoded; charset=utf-8"
CIam::CIam(
			const char *pszHost,
			const char *pszAccessId,
			const char *pszSecKey)
		: m_AwsContext(pszAccessId, pszSecKey, NULL, pszHost)
{

}


int CIam::Get(const char *szQuery, const char *pContentType, const std::time_t request_date, const char *pPayLoad, int nLen, int nTimeOut)
{
	int result = JD_OK;
    int httpStatus;
	size_t size = nLen;

	CJdAwsS3Request request(&m_AwsContext);

	request.methodM = CJdAwsS3Request::GET;
	request.pContextM = &m_AwsContext;

	if(pContentType != NULL && strlen(pContentType)){
		request.contentTypeM.assign(pContentType);
	} else {
		request.contentTypeM.assign(CONTENT_STR_DEF);
	}

	request.serviceM = "iam";
	request.queryStringM = szQuery;
	request.pathM.assign("");
	request.bucketNameM.assign("");

	request.contentLengthM = size;
	request.fUseHttpsM = false; //gUseHttps;

	//CJdAwsContext::GetCurrentDate(request.dateM);
	request.dateM = request_date;

	request.contentSha256 = sha256_base16(pPayLoad);

	CJdAwsS3HttpResponse response;
	JD_STATUS status = CJdAwsS3HttpConnection::MakeRequest(request, response);
    if(status != JD_OK){
		result = JD_ERROR_CONNECTION;
		goto Exit;
	}
    assert(response.pHandleM);
    status = JdAwsWriteHttpRequest(response.pHandleM,  (char *)pPayLoad, &size, 30);
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