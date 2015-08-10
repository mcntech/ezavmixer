#include <vector>
#include "strmconn.h"
#include "StrmInBase.h"

class CAvMixer
{
public:
	CAvMixer()
	{
		m_pVidCompOut = NULL;
		m_pAudMixerOut = NULL;
	}

	// CStrmInBridgeBase
	int StartStreaming(void) { return 0;}
	int StopStreaming(void){ return 0;}

	int Start(const char *pszConfFile, const char *pszAvMixerId, int nLayoutOption)
	{
		StartCodec(pszConfFile, nLayoutOption);
		StartInputStreams(pszConfFile, pszAvMixerId, nLayoutOption);
		return 0;
	}
	void Stop()
	{
		StopInputStreams();
		StopEncodeSubsystem(0);
	}
	int StopInputStreams();
	int StopEncodeSubsystem(int nSwitchId);
	int StartCodec(const char *pszConfFile, int nLayoutOption);
	int StartInputStreams(const char *pszConfFile, const char *pszAvmexerId, int nLayoutOption);
	CStrmInBridgeBase *GetOutputConn()
	{
		//int result = 0;
		//CStrmConnWrapper *pOutput = new CStrmConnWrapper(m_pAudMixerOut != NULL, m_pVidCompOut != NULL, m_pAudMixerOut, m_pVidCompOut, &result);
		//return pOutput;
		return NULL;
	}
public:
	//ENGINE_T            *m_pEngine;
	ConnCtxT            *m_pVidCompOut;
	ConnCtxT            *m_pAudMixerOut;
	std::vector <CStrmInBase *> m_listAvmixInputs;
	
	int                 m_nLayoutId;

	char               m_szInputSessionName[64];
	char               m_szInputUri[MAX_URI_SIZE];
	
	//char               szOutputSection[MAX_FILE_NAME_SIZE];
	//COutputStream      *m_pOutputStream;
};