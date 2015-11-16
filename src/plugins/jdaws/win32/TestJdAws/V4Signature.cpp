#include <iostream>
#include "awsv4.h"
const std::string POST = "POST";
const std::string  STRING_TO_SIGN_ALGO = "AWS4-HMAC-SHA256";

template<typename T, size_t N>
T * end(T (&ra)[N]) {
    return ra + N;
}

int TestV4Signature() {

    // 20110909T233600Z
    struct std::tm t;
    t.tm_sec = 0;
    t.tm_min = 36;
    t.tm_hour = 16;
    t.tm_mon = 2;
    t.tm_year = 2012 - 1900;
    t.tm_isdst = -1; 
    t.tm_mday = 15;   
    const std::time_t request_date = std::mktime(&t);

    const std::string region("us-east-1");
    const std::string service("iam");

    const std::string base_uri("http://iam.amazonaws.com/");
    const std::string query_args("");
    const std::string uri_str(base_uri + "?" + query_args);

    URI uri;
    try {
        uri = URI(uri_str);
    } catch (std::exception& e) {
        throw std::runtime_error(e.what());
    }
    //uri.normalize();

    const auto canonical_uri = canonicalize_uri(uri);
    
    const auto canonical_query = canonicalize_query(uri);

	  const char *c_headers[] = {"host: iam.amazonaws.com",
            "Content-type: application/x-www-form-urlencoded; charset=utf-8",
			"x-amz-date: 20110909T233600Z"};
    const std::vector<std::string> headers(c_headers, end(c_headers));
    
    const auto canonical_headers_map = canonicalize_headers(headers);
    if (canonical_headers_map.empty()) {
        std::cerr << "headers malformed" << std::endl;
        std::exit(1);
    }
    const auto headers_string = map_headers_string(canonical_headers_map);
    const auto signed_headers = map_signed_headers(canonical_headers_map);

    const std::string payload("Action=ListUsers&Version=2010-05-08");
    auto sha256_payload = sha256_base16(payload); 
    
    const auto canonical_request = canonicalize_request(POST,
                                                               canonical_uri,
                                                               canonical_query,
                                                               headers_string,
                                                               signed_headers,
                                                               payload);
    
    std::cout << "--\n" << canonical_request << "\n--\n" << std::endl;

    auto hashed_canonical_request = sha256_base16(canonical_request); 
    std::cout << hashed_canonical_request << std::endl;

    std::string my_credential_scope = credential_scope(request_date,region,service);

    std::string my_string_to_sign = string_to_sign(STRING_TO_SIGN_ALGO,
                                                request_date,
                                                my_credential_scope,
                                                hashed_canonical_request);

    std::cout << "--\n" << string_to_sign << "\n----\n" << std::endl;

    const std::string secret = "wJalrXUtnFEMI/K7MDENG+bPxRfiCYEXAMPLEKEY";
    
    auto signature = calculate_signature(request_date, 
                                                secret,
                                                region,
                                                service,
                                                my_string_to_sign);
    
    std::cout << signature << std::endl;
    return 0;
}
/*

key = 'wJalrXUtnFEMI/K7MDENG+bPxRfiCYEXAMPLEKEY'
dateStamp = '20120215'
regionName = 'us-east-1'
serviceName = 'iam'

kSecret  = '41575334774a616c725855746e46454d492f4b374d44454e472b62507852666943594558414d504c454b4559'
kDate    = '969fbb94feb542b71ede6f87fe4d5fa29c789342b0f407474670f0c2489e0a0d'
kRegion  = '69daa0209cd9c5ff5c8ced464a696fd4252e981430b10e3d3fd8e2f197d7a70c'
kService = 'f72cfd46f26bc4643f06a11eabb6c0ba18780c19a8da0c31ace671265e3c87fa'
kSigning = 'f4780e2d9f65fa895f9c67b32ce1baf0b0d8a43505a000a1a9e090d414db404d'

*/