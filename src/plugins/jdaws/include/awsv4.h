#ifndef AWSV4_HPP
#define AWSV4_HPP

#include <stdio.h>
#include <string.h>

#include <stdexcept>
#include <algorithm>
#include <map>
#include <ctime>
#include <iostream>
#include <sstream>
#include <vector>
//#include "Poco/URI.h"
//#include "Poco/StringTokenizer.h"

#include "openssl/sha.h"
#include "openssl/hmac.h"

class CAwsV4
{
public:
	CAwsV4();
    void sha256(const std::string str, unsigned char outputBuffer[SHA256_DIGEST_LENGTH]);
    
    const std::string sha256_base16(const std::string);

    //const std::string canonicalize_uri(const Poco::URI& uri) noexcept;
    const std::string canonicalize_uri(const std::string &uri);
    
    //const std::string canonicalize_query(const Poco::URI& uri) noexcept;
    const std::string canonicalize_query(const std::string& uri);
    
    const std::map<std::string,std::string> canonicalize_headers(const std::vector<std::string>& headers);
    
    const std::string map_headers_string(const std::map<std::string,std::string>& header_key2val);
    
    const std::string map_signed_headers(const std::map<std::string,std::string>& header_key2val);
    
    const std::string canonicalize_request(const std::string& http_request_method,
                                           const std::string& canonical_uri,
                                           const std::string& canonical_query_string,
                                           const std::string& canonical_headers,
                                           const std::string& signed_headers,
                                           const std::string& payload);

    const std::string string_to_sign(const std::string& algorithm,
                                     const std::time_t& request_date,
                                     const std::string& credential_scope,
                                     const std::string& hashed_canonical_request);
    
    const std::string ISO8601_date(const std::time_t& t);
    
    const std::string utc_yyyymmdd(const std::time_t& t);

    const std::string credential_scope(const std::time_t& t, 
                                       const std::string region,
                                       const std::string service);

    const std::string calculate_signature(const std::time_t& request_date, 
                                          const std::string secret,
                                          const std::string region,
                                          const std::string service,
                                          const std::string string_to_sign);
    
};

#endif
