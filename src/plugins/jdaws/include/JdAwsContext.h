/*
 *  JdAwsContext.h
 *
 *  Copyright 2011 MCN Technologies Inc.. All rights reserved.
 *
 */

#ifndef __JD_AWS_CONTEXT_H__
#define __JD_AWS_CONTEXT_H__

#include <string>

class CJdAwsContext {
public:
    static void ToBase64(const char* aContent, 
                         size_t aContentSize,
                         /*OUT*/ std::string &strOut);
    
    /**
     * @return The current date formatted for the 'DATE' header in a HTTP
     *         request.
     */
    static void GetCurrentDate(/*OUT*/ std::string &date);
    
    /**
     * @param pId The AWS id
     * @param pSecretKey The AWS secret key
     * @param pUserAgent The value to use for the USER AGENT header when sending
     *                   out HTTP requests. If NULL then the USER AGENT header will
     *                   not be sent.
     */
    CJdAwsContext(const char *pId,
                  const char *pSecretKey,
				  int         signatureVersion,
                  const char *pUserAgent,
                  const char *pDefaulthost,
                  const char *pUserToken = NULL,
                  const char *pProductToken = NULL);
    CJdAwsContext(){}
    CJdAwsContext(CJdAwsContext &JdAwsContext)
    {

    }
    const std::string& GetId() const { return idM; }
    const std::string& GetSecretKey() const { return secretKeyM; }
    const std::string& GetUserAgent() const { return userAgentM; }
    const std::string& GetDefaultHost() const { return defaultHostM; }
    const std::string& GetUserToken() const { return userTokenM; }
    const std::string& GetProductToken() const { return productTokenM; }
    int GetSignatureVersion() const { return signatureVersionM; }

public:
    std::string idM;
    std::string secretKeyM;
    std::string userAgentM;
    /* default host */
    std::string defaultHostM;
    /* user token for Amazon DevPay */
    std::string userTokenM;
    /* product token for Amazon DevPay */
    std::string productTokenM;
    /* product token for Amazon DevPay */
    int signatureVersionM;
};

#endif // __JD_AWS_CONTEXT_H__
