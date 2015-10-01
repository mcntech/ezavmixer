#ifndef __RTSP_CONFIGURE_H__
#define __RTSP_CONFIGURE_H__

#include "ConfigBase.h"
class CRtspCommonConfig
{
public:
	CRtspCommonConfig(CConfigBase *pConfig);
	CRtspCommonConfig(const char *_pszStreamName, const char *_pszInput, int fEnableVid, int fEnableAud, int fEnableMux);
public:
	char szStreamName[128];
	char m_szInput0[128];
	int            m_fEnableAud;
	int            m_fEnableVid;
	int            m_fEnableMux;
};

#endif // __RTSP_CONFIGURE_H__
