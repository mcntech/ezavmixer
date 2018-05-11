#ifndef __JDWS__
#define __JDWS__

#include <string>
using namespace std;
#define MAX_REPLY_SIZE  1024

class CJdWsService
{
    public:
        virtual std::string ProcessWsRequest(std::string request) = 0;
};

class CJdWs
{
public:
    static CJdWs *Create(int nPort);
    virtual int Stop() = 0;
    virtual int Start() = 0 ;
    virtual int RegisterService(CJdWsService *pService, std::string serviceName) = 0;
    virtual int UnregisterService(std::string  serviceName) = 0;
};

#endif