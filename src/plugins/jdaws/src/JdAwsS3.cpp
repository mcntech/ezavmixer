/*
 *  JdAwsS3.cpp
 *
 *  Copyright 2011 MCN Technologies Inc.. All rights reserved.
 *
 */

#include <assert.h>
#include <openssl/hmac.h>
#include <sstream>
#include "JdAwsS3.h"
#include <string>

#define ENABLE_OPENSSL

char *jd_url_encode(const char *)
{
	return NULL;
}

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
                                    /*OUT*/ std::string &signature)
{
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
    else 
        ss << request.dateM;
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
#if 0
	printf("Signature Plain:%s\n", signature.c_str());
#endif
#ifdef ENABLE_OPENSSL
    HMAC(EVP_sha1(), request.pContextM->GetSecretKey().c_str(),  
         request.pContextM->GetSecretKey().size(),
         (const unsigned char*) signature.c_str(), signature.size(),
         pEncryptedResult, &encryptedResultSize);   
#else
	// TODO enable openssl
#endif
	CJdAwsContext::ToBase64((const char*) pEncryptedResult, encryptedResultSize, signature);
#if 0
	printf("Key %s\n", request.pContextM->GetSecretKey().c_str());
	printf("Signature Encrypt: %s\n", signature.c_str());
#endif
	return JD_OK;
}

JD_STATUS CJdAwsS3::CreateSignatureV4(const CJdAwsS3Request &request,
                                    /*OUT*/ std::string &signature)
{
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
    // # Step 1 is to define the verb (GET, POST, etc.)--already done.
    // StringToSign = HTTP-Verb + "\n" +
    //Step 2: Create canonical URI
    //
    // Step 3: Create the canonical query string.
    //
    // Step 4: Create the canonical headers and signed headers.
    // # and value must be trimmed and lowercase, and sorted in ASCII order.
	// # Note that there is a trailing \n.
    // canonical_headers = 'host:' + host + '\n' + 'x-amz-date:' + amzdate + '\n'
    //
    // Step 5: Create the list of signed headers
	// # in the canonical_headers list, delimited with ";" and in alpha order.
	// # Note: The request can include any headers; canonical_headers and
	// # signed_headers lists those that you want to be included in the
	// # hash of the request. "Host" and "x-amz-date" are always required.
    // signed_headers = 'host;x-amz-date'
    //
    // Step 6: Create payload hash
    //
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
    else
        ss << request.dateM;
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
#if 0
	printf("Signature Plain:%s\n", signature.c_str());
#endif
#ifdef ENABLE_OPENSSL
    HMAC(EVP_sha1(), request.pContextM->GetSecretKey().c_str(),
         request.pContextM->GetSecretKey().size(),
         (const unsigned char*) signature.c_str(), signature.size(),
         pEncryptedResult, &encryptedResultSize);
#else
	// TODO enable openssl
#endif
	CJdAwsContext::ToBase64((const char*) pEncryptedResult, encryptedResultSize, signature);
#if 0
	printf("Key %s\n", request.pContextM->GetSecretKey().c_str());
	printf("Signature Encrypt: %s\n", signature.c_str());
#endif
	return JD_OK;
}

JD_STATUS CJdAwsS3::MakeQueryStringUri(const CJdAwsS3Request &request,
                                       /*OUT*/ std::string &uri)
{
    if (!request.expiresM.size()) {
        return JD_ERROR_INVALID_ARG;
    }
    std::string signature;
    JD_STATUS status = CJdAwsS3::CreateSignature(request, signature);
    if (status != JD_OK) {
        return status;
    }
    std::ostringstream ss;
    /* insert subresource and user specified querystring into the querystring */
    std::string queryString;
    {
        bool fInsertAmp = false;
        if (request.subResourceM.size()) {
            ss << request.subResourceM;
            fInsertAmp = true;
        }
        if (request.queryStringM.size()) {
            if (fInsertAmp) {
                ss << "&";
            }
            ss << request.queryStringM;
            fInsertAmp = true;
        }
        if (fInsertAmp) {
            ss << "&";
        }
    }
    /* insert headers into the querystring */
    {
        int size = request.amzHeaderNamesM.size();
        if (size > 0) {
            for (int i = 0; i < size; i++) {
                ss << request.amzHeaderNamesM[i] << "=";
                char *pEncoded = jd_url_encode(request.amzHeaderValuesM[i].c_str());
                ss << pEncoded << "&";
                free(pEncoded);
            }
        }
    }
    /* insert aws credentials */
    ss << "AWSAccessKeyId=" << request.pContextM->GetId() << "&";
    {
        char *pEncoded = jd_url_encode(signature.c_str());
        ss << "Signature=" << pEncoded << "&";
        free(pEncoded);
    }
    ss << "Expires=" << request.expiresM;
    queryString = ss.str();
    ss.str("");
    if (request.fUseHttpsM) 
        ss << "https://";
    else 
        ss << "http://";
    ss << request.hostM;
    if (request.bucketNameM.size()) {
        ss << "/" << request.bucketNameM;
    } 
    ss << request.pathM << "?" << queryString;
    uri = ss.str();
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
    ss << request.bucketNameM << "." << request.hostM  <<  request.pathM;
    if (queryString.size()) {
        char *pEncode = jd_url_encode(queryString.c_str());
        ss << "?" << pEncode;
        free(pEncode);
    }
    uri = ss.str();
    return JD_OK;
}

JD_STATUS CJdAwsS3::MakeHost(const CJdAwsS3Request &request,
                                    /*OUT*/ std::string &host)
{
    std::ostringstream ss;

    ss << request.bucketNameM << "." << request.hostM;
    host = ss.str();
    return JD_OK;
}
// Version 4
#define MAX_ENCRYPT_SIZE 1024
bool HmacSHA256(std::string data, std::string in_key, std::string &out_key)
{
	std::string algorithm = "HmacSHA256";
	//KeyedHashAlgorithm kha = KeyedHashAlgorithm.Create(algorithm);
	//kha.Key = key;
	//return kha.ComputeHash(Encoding.UTF8.GetBytes(data));
	unsigned char EncryptedResult[MAX_ENCRYPT_SIZE];
	unsigned int encryptedResultSize = 0;
    HMAC(EVP_sha1(), in_key.c_str(), in_key.size(),
         (const unsigned char*) data.c_str(), data.size(),
         EncryptedResult, &encryptedResultSize);
    // TODO out_key = EncryptedResult;
	return true;
}

bool getSignatureKey(std::string key, std::string dateStamp, std::string regionName, std::string serviceName, std::string &sigKey)
{
	std::string kSecret = "AWS4" + key;
	std::string kDate;
	HmacSHA256(dateStamp, kSecret, kDate);
	std::string  kRegion;
	HmacSHA256(regionName, kDate, kRegion);
	std::string kService;
	HmacSHA256(serviceName, kRegion, kService);
	HmacSHA256("aws4_request", kService, sigKey);
	return true;
}
