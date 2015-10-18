
#ifndef _PUBLISH_CLNT_BASE_H_
#define _PUBLISH_CLNT_BASE_H_

#ifdef WIN32
#include <winsock2.h>
#else // Linux
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef PLATFORM_ANDROID
#include <execinfo.h>
#endif

#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <signal.h>

class CPublishClntBase
{
public:
	virtual int AddPublishServer(std::string url, std::string appName, int localRtpPort=0, int remoteRtpPort=0, int serverPort=554) = 0;
	virtual int RemovePublishServer(std::string url) = 0;

	virtual int sendAudioData(const char *pData, int numBytes, long Pts, int Flags) = 0;
	virtual int sendVideoData(const char *pData, int numBytes, long Pts, int Flags) = 0;

	virtual int start() = 0;
	virtual int stop() = 0;
};

#endif // _PUBLISH_CLNT_BASE_H_
