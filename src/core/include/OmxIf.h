#ifndef __OMX_IF_H__
#define __OMX_IF_H__

#include "OpenMAXAL/OpenMAXAL.h"
#include  "onyx_omxext_api.h"
#include "StrmInBridgeBase.h"
#define MAX_URI_SIZE 128

typedef enum _INPUT_TYPE_T {
	INPUT_TYPE_UNKNOWN,
	INPUT_TYPE_FILE,
	INPUT_TYPE_RTSP,
	INPUT_TYPE_HLS,
	INPUT_TYPE_CAPTURE,
	INPUT_TYPE_STRMCONN,	    ///< 
	INPUT_TYPE_STRMCONN_IPC,    ///< 
	INPUT_TYPE_AVMIXER,         ///< 
	INPUT_TYPE_STRMCONN_ZMQ,
	INPUT_TYPE_INPROC
} INPUT_TYPE_T;

#define IPC_SOCK_PORT_NAME_SIZE 32

typedef struct _EXT_PARAM_STRMCONN_IPC_T
{
	char szAudSocketRxName[IPC_SOCK_PORT_NAME_SIZE];
	char szAudSocketTxName[IPC_SOCK_PORT_NAME_SIZE];
	char szVidSocketRxName[IPC_SOCK_PORT_NAME_SIZE];
	char szVidSocketTxName[IPC_SOCK_PORT_NAME_SIZE];
} EXT_PARAM_STRMCONN_IPC_T;


class  CInputStrmBase
{
public:
	CInputStrmBase();
	virtual ~CInputStrmBase(){}
public:
	int  nCodecSessionId;
	char pszInputUri[MAX_URI_SIZE];
	char vid_codec_name[128];
	char aud_codec_name[128];
	INPUT_TYPE_T nInputType;
	int  nSampleRate;
	int  nFrameRate;
	int  nLatency;
	int  nWidth;
	int  nHeight;
	int  nDeinterlace;
	int  nPcrPid;
	int  nAudPid;
	int  nVidPid;
	int  nSelectProg;

	int  fEnableAud;
	int  fEnableVid;

	int nCmd;
	int nPort;

	CStrmInBridgeBase *mpInputBridge;
	union {
		EXT_PARAM_STRMCONN_IPC_T strmconn_ipc;
	} ExtParam;

};

class  CAvmixInputStrm : public CInputStrmBase
{
public:
	CAvmixInputStrm()
	{
		mpInputBridge = NULL;
		playerObject = NULL;
		playerPlay = NULL;
		playerExt = NULL;
	}
	~CAvmixInputStrm(){}
    XAObjectItf           playerObject;
    XAPlayItf             playerPlay;
    XAConfigExtensionsItf playerExt;
};


typedef struct MIXER_SESSION_T
{
	int  nCodecSessionId;
	int nCmd;
	int nPort;

	int  nSampleRate;
	int  nFrameRate;
	int  nWidth;
	int  nHeight;

	XAObjectItf        mixerObject;
} MIXER_SESSION_T;

typedef struct ENGINE_T
{
	//int                   nSessionId;
	XAObjectItf           engineObject;
	XAEngineItf           engineEngine;
	
	XAObjectItf           recorderObject;	
	MIXER_SESSION_T       *pMixerSession;
} ENGINE_T;

ENGINE_T *omxalInit(const char *pszConfFile,  DISP_WINDOW_LIST *pWndList, AUD_CHAN_LIST *pAChanList);
void omxalDeinit(ENGINE_T *pSession);

int omxalCreateRecorder(ENGINE_T *pEngine, void *vidDataSnk, void *audDataSnk);

int omxalPlayStream(ENGINE_T *pEngine, CAvmixInputStrm *pSession, XADataSource *pDataSrc1, XADataSource *pDataSrc2);
void omxalStopStream(CAvmixInputStrm *pSession);

#endif // __OMX_IF_H__
