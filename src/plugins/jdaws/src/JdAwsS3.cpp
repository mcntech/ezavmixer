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
#include <vector>
#include "awsv4.h"

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
        ss << ISO8601_date(request.dateM);
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

JD_STATUS CJdAwsS3::MakeQueryStringUri(const CJdAwsS3Request &request,
                                       /*OUT*/ std::string &uri)
{
    if (!request.expiresM.size()) {
        return JD_ERROR_INVALID_ARG;
    }
    std::string signature;
	std::string authorization;
    JD_STATUS status = JD_ERROR;
    if(request.signatureVresionM == request.V2)
    	status = CJdAwsS3::CreateSignature(request, signature);
    else if (request.signatureVresionM == request.V4)
    	status = CJdAwsS3::CreateSignatureV4(request, authorization);
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
				std::string Encoded;
				URI::encode(request.amzHeaderNamesM[i],"",Encoded);
                ss << Encoded << "&";
            }
        }
    }
    /* insert aws credentials */
    ss << "AWSAccessKeyId=" << request.pContextM->GetId() << "&";
    {
		std::string Encoded;
		URI::encode(signature,"",Encoded);
        ss << "Signature=" << Encoded << "&";
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

