
#ifndef _PUBLISH_CLNT_BASE_H_
#define _PUBLISH_CLNT_BASE_H_

#ifdef WIN32
#include <winsock2.h>
#else // Linux
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef PLATFORM_ANDROID
//#include <execinfo.h>
#endif

#endif
#include <assert.h>
#include <stdio.h>
#ifndef __PLAYER_BASE_H__
#define __PLAYER_BASE_H__
#include <stdlib.h>
#include <fcntl.h>

#include <signal.h>


#define STRM_CONN_VID 1
#define STRM_CONN_AUD 2

class CPlayerBase
{
public:
	virtual int  getStatus(std::string url, std::string &status) = 0;

	virtual int  addServer(std::string url) = 0;
	virtual int  removeServer(std::string url) = 0;
	virtual int  subscribeStream(std::string url,  int substrmId) = 0;
	virtual int  unsubscribeStream(std::string url,  int substrmId) = 0;

	virtual int  getData(std::string url,  int substrmId, char *pData, int numBytes) = 0;
	virtual long long getPts(std::string url, int substrmId) = 0;
	virtual long long getClkUs(std::string url) = 0;
	virtual int  getCodecType(std::string url, int substrmId) = 0;
	virtual int  getNumAvailFrames(std::string url, int substrmId) = 0;

	virtual int startServer(std::string url) = 0;
	virtual int stopServer(std::string url) = 0;

};

#endif

#endif // _PUBLISH_CLNT_BASE_H_
