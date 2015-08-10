#ifndef __INPUT_STREM_BASE_H__
#define __INPUT_STREM_BASE_H__
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
	INPUT_TYPE_STRMCONN_ZMQ
} INPUT_TYPE_T;

#define IPC_SOCK_PORT_NAME_SIZE 32

typedef struct _EXT_PARAM_STRMCONN_IPC_T
{
	char szAudSocketRxName[IPC_SOCK_PORT_NAME_SIZE];
	char szAudSocketTxName[IPC_SOCK_PORT_NAME_SIZE];
	char szVidSocketRxName[IPC_SOCK_PORT_NAME_SIZE];
	char szVidSocketTxName[IPC_SOCK_PORT_NAME_SIZE];
} EXT_PARAM_STRMCONN_IPC_T;


class  CStrmInBase
{
public:
	CStrmInBase();
	virtual ~CStrmInBase(){}
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
	//union {
	//	EXT_PARAM_STRMCONN_IPC_T strmconn_ipc;
	//} ExtParam;

};

#endif // __INPUT_STREM_BASE_H__