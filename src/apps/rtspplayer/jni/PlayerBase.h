
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
	virtual int  getAudioData(std::string url, char *pData, int numBytes) = 0;
	virtual int  getVideoData(std::string url, char *pData, int numBytes) = 0;
	virtual long long getAudioPts(std::string url) = 0;
	virtual long long getVideoPts(std::string url) = 0;
	virtual long long getClkUs(std::string url) = 0;
	virtual int  getVideoCodecType(std::string url) = 0;
	virtual int  getAudioCodecType(std::string url) = 0;
	virtual int  getNumAvailVideoFrames(std::string url) = 0;
	virtual int  getNumAvailAudioFrames(std::string url) = 0;

	virtual int start(std::string url) = 0;
	virtual int stop(std::string url) = 0;

};

#endif

#endif // _PUBLISH_CLNT_BASE_H_
