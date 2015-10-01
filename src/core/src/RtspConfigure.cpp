#include "RtspConfigure.h"
#include "string.h"
#include <string>
#include "uimsg.h"

CRtspCommonConfig::CRtspCommonConfig(CConfigBase *pConfig)
{
	std::string tmp;
	m_fEnableMux =  pConfig->getl(SECTION_RTSP_COMMON, KEY_RTSP_COMMON_ENABLE_MUX,   0);
	m_fEnableVid =  pConfig->getl(SECTION_RTSP_COMMON, KEY_RTSP_COMMON_ENABLE_VID,   1);
	m_fEnableAud =  pConfig->getl(SECTION_RTSP_COMMON, KEY_RTSP_COMMON_ENABLE_AUD,   1);
	tmp = pConfig->gets(SECTION_RTSP_COMMON, KEY_RTSP_COMMON_INPUT0);
	strncpy(szStreamName, tmp.c_str(), 128);
}

CRtspCommonConfig::CRtspCommonConfig(const char *_pszStreamName, const char *_pszInput, int fEnableVid, int fEnableAud, int fEnableMux)
{
	strncpy(szStreamName, _pszStreamName, 128);
	strncpy(m_szInput0, _pszInput, 128);
	m_fEnableAud = fEnableAud;
	m_fEnableVid = fEnableVid;
	m_fEnableMux = fEnableMux;
}
