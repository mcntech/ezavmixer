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
	strncpy(m_szStreamName, tmp.c_str(), 128);
}

CRtspCommonConfig::CRtspCommonConfig(const char *_pszStreamName, int fEnableVid, int fEnableAud, int fEnableMux)
{
	if(_pszStreamName) {
		strncpy(m_szStreamName, _pszStreamName, 128);
	}
	m_fEnableAud = fEnableAud;
	m_fEnableVid = fEnableVid;
	m_fEnableMux = fEnableMux;
}
