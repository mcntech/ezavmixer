#ifndef __AD_MANAGER_H__
#define __AD_MANAGER_H__

#ifdef WIN32
//#include <Windows.h>
#include <winsock2.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#endif

#include "tinyxml.h"

//http://54.88.61.100/revive/www/delivery/fc.php?script=bannerTypeHtml:vastInlineBannerTypeHtml:vastInlineHtml&zones=pre-roll:0.0-0%3D1&nz=1&source=&r=R0.8214839450083673&block=0&format=vast&charset=UTF-8
class CAdManager
{
public:
	enum AD_TYPE_T
	{
		AD_TYPE_STATIC,
		AD_TYPE_STATIC_JIT,
		AD_TYPE_DYNAMIC,
		AD_TYPE_DYNAMIC_JIT
	};
	CAdManager(const char *szVastTag);
	~CAdManager();
	int GetAdType();
	TiXmlNode *GetMpdPeriod(int nTime);
	AD_TYPE_T m_AdType;
	char *m_szVastTag;
	int   m_nZoneId;
};

#endif //__AD_MANAGER_H__