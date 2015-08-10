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
#include "minini.h"
#include "AvMixer.h"
#include "JdDbg.h"

static int  modDbgLevel = CJdDbg::LVL_TRACE;

int CAvMixer::StartInputStreams(const char *pszConfFile, const char *pszAvmexerId, int nLayoutOption)
{
	int res = 0;
	int i;
	int fEnableAudio;
	int nNumInputs = 0;
	char szInputId[16];
	JDBG_LOG(CJdDbg::LVL_TRACE,("Enter"));
	if(nLayoutOption > 0) {
		m_nLayoutId = nLayoutOption;
	} else {
		m_nLayoutId = 1;
	}
	
	JDBG_LOG(CJdDbg::LVL_SETUP,("LaoutId=%d\n", m_nLayoutId));
	fEnableAudio =  ini_getl(ONYX_MW_SECTION, ENABLE_AUDIO, 1, ONYX_PUBLISH_FILE_NAME);	
	nNumInputs = ini_getl(pszAvmexerId, AVMIXER_INPUT_COUNT, 0, pszConfFile);	
	
	for (i=0; i < nNumInputs; i++) {
		INPUT_TYPE_T nInputType;
		sprintf(szInputId, "%s%d",AVMIXER_INPUT_PREFIX, i);
		ini_gets(pszAvmexerId, szInputId, "",m_szInputSessionName, 64, pszConfFile);
		CAvmixInputStrm *pInputStream = CStreamUtil::GetStreamParamsFromCfgDb(m_szInputSessionName, pszConfFile);
		nInputType = pInputStream->nInputType;
		JDBG_LOG(CJdDbg::LVL_SETUP,("InputSection=%s resource=%s\n", m_szInputSessionName,m_szInputUri));

		if(nInputType != INPUT_TYPE_UNKNOWN) {
			m_listAvmixInputs.push_back(pInputStream);
			JDBG_LOG(CJdDbg::LVL_SETUP,("Opening %s input=%s\n", m_szInputSessionName, m_szInputUri));
			GetPlayerSessionUserOverrides(pInputStream, pszConfFile, m_szInputSessionName, fEnableAudio);
			ini_gets(m_szInputSessionName, "vid_codec", "h264", pInputStream->vid_codec_name, MAX_CODEC_NAME_SIZE, pszConfFile);
			if(CStreamUtil::InitInputStrm(pInputStream, i) == 0) {
				res = omxalPlayStream(m_pEngine, pInputStream, pInputStream->mpInputBridge->GetDataSource1(), pInputStream->mpInputBridge->GetDataSource2());
				if(pInputStream->mpInputBridge) {
					pInputStream->mpInputBridge->StartStreaming();
				}

			} else {
				delete pInputStream;
				JDBG_LOG(CJdDbg::LVL_ERR,("!!! Can not careate session %s or %s !!!\n", m_szInputSessionName,pszConfFile));
			}
		} else {
			JDBG_LOG(CJdDbg::LVL_ERR,("!!! Can not find %s or %s !!!\n", m_szInputSessionName,pszConfFile));
		}
	}
	JDBG_LOG(CJdDbg::LVL_TRACE,("Leave"));	
	return res;
}

int CAvMixer::StopInputStreams()
{

	int i;
	JDBG_LOG(CJdDbg::LVL_TRACE,("Enter"));
	for (std::vector<CAvmixInputStrm *>::iterator it = m_listAvmixInputs.begin(); it != m_listAvmixInputs.end(); it++) {
		CAvmixInputStrm *pInputStream = *it;
		JDBG_LOG(CJdDbg::LVL_TRACE,("Stopping Playback Seesion %d", i));
		if(pInputStream) {
			CStreamUtil::DeinitInputStrm(pInputStream);
			omxalStopStream(pInputStream);
			delete pInputStream;
		}
	}
	m_listAvmixInputs.clear();
	JDBG_LOG(CJdDbg::LVL_TRACE,("Leave"));
	return 0;
}

int CAvMixer::StartCodec(const char *pszConfFile, int nLayoutOption)
{
	int res = 0;
	int i;
	int fEnableAudio;
	AUD_CHAN_LIST    *pAChanList = NULL;
	DISP_WINDOW_LIST *pWndList = NULL;

	JDBG_LOG(CJdDbg::LVL_TRACE,("Enter"));
	if(nLayoutOption > 0) {
		m_nLayoutId = nLayoutOption;
	} else {
		m_nLayoutId = 1;
	}

	fEnableAudio =  ini_getl(ONYX_MW_SECTION, ENABLE_AUDIO, 1, ONYX_PUBLISH_FILE_NAME);

	GetVidMixerConfig(pszConfFile, m_nLayoutId, &pWndList);

	if(fEnableAudio) {
		GetAudMixerConfig(pszConfFile, m_nLayoutId, &pAChanList);
	}
	m_pEngine = omxalInit(pszConfFile,  pWndList, pAChanList);

#if defined(EN_IPC_STRM_CONN)
	m_pVidCompOut = CreateIpcStrmConn(VID_ENC_RX, VID_ENC_TX, 1024*1024);
#else
	m_pVidCompOut = CreateStrmConn(MAX_VID_FRAME_SIZE, 4);
#endif

	if(fEnableAudio) {
#if defined(EN_IPC_STRM_CONN)
		m_pAudMixerOut = CreateIpcStrmConn(AUD_ENC_RX, AUD_ENC_TX, 16*1024);
#else
		m_pAudMixerOut = CreateStrmConn(MAX_AUD_FRAME_SIZE, 4);
#endif
	}
#if defined(EN_IPC_STRM_CONN)
	omxalCreateRecorder(m_pEngine, NULL, NULL);
#else
	omxalCreateRecorder(m_pEngine, m_pVidCompOut, m_pAudMixerOut);
#endif	
	JDBG_LOG(CJdDbg::LVL_TRACE,("Leave"));	
	return res;
}

int CAvMixer::StopEncodeSubsystem(int nSwitchId)
{
	JDBG_LOG(CJdDbg::LVL_TRACE,("Enter"));

	JDBG_LOG(CJdDbg::LVL_TRACE,("DeleteEngine"));
	if(m_pEngine) {
		omxalDeinit(m_pEngine);
	}

	//if(m_pOutputStream) {
	//	delete m_pOutputStream;
	//	m_pOutputStream = NULL;
	//}
	if(m_pVidCompOut) {
		delete m_pVidCompOut;
		m_pVidCompOut = NULL;
	}

	JDBG_LOG(CJdDbg::LVL_TRACE,("Leave"));
	return 0;
}
