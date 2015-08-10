#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#ifdef WIN32
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <execinfo.h>
#endif
#include <fcntl.h>
#include "dbglog.h"

#include "OpenMAXAL.h"
#include "dec_main.h"
#include "cap_main.h"
#include "minini.h"
#include "strmcomp.h"
#include  "onyx_omxext_api.h"

#ifdef WIN32
#include "layout.h"
#else
#include "ilclient_common.h"
#include "video_mixer.h"
#endif
#include "audio_mixer.h"
#include "strmconn_ipc.h"

#define VIDMIXER_SECTION "vidmixer"
#define DISP_SECTION     "display"
#define ENCODE_SECTION   "encode"
#define DEBUG_SECTION  "debug"

#ifdef WIN32
char    *gIniFile = "C:/Projects/onyx/onyx_dm8168/onyx_core.conf";
#else
char    *gIniFile = "/etc/onyx_core.conf";
#endif

#define DBG_ASSERT(x)

#define MAX_PLAY_SESSIONS       6
#define ONYX_OMX_VERSION			 (0x00010004)
#define MAX_URL_SIZE              256

#define DEF_VIDMIXEROUT_WIDTH  (1280)
#define DEF_VIDMIXEROUT_HEIGHT (720)

#define DEF_DISPLAY_WIDTH     (800)
#define DEF_DISPLAY_HEIGHT    (480)
#define DEF_ENC_WIDTH         (1920)
#define DEF_ENC_HEIGHT        (1080)
#define DEF_LAYOUT_ID          1
#define DEF_ENC_FRAMERATE    (30)
#define DEF_ENC_BITRATE       (1500000)

int gMixerOutWidth = DEF_VIDMIXEROUT_WIDTH;
int gMixerOutHeight = DEF_VIDMIXEROUT_HEIGHT;
int gDispWidth = DEF_DISPLAY_WIDTH;
int gDispHeight = DEF_DISPLAY_HEIGHT;
int gEncWidth  = DEF_ENC_WIDTH;
int gEncHeight = DEF_ENC_HEIGHT;
int gLayoutId = DEF_LAYOUT_ID;
int gEncFramerate = DEF_ENC_FRAMERATE;
int gEncBitrate = DEF_ENC_BITRATE;

#define EN_IPC_STRM_CONN
#define IPC_PORT_NAME_SIZE  128

#ifndef WIN32
// A global instance of IL Client common
VIDEO_MIXER_T *gpVmix = NULL;
#endif
AUDIO_MIXER_T *gpAmix = NULL;
//#define IS_EQUAL_IID(x,y)         (x == y)          // TODO: Implement comparison of IID structure
int IS_EQUAL_IID(const XAInterfaceID X, const XAInterfaceID Y)
{
	int res = 1;
	int i;
	const char *pX = (const char *)X;
	const char *pY = (const char *)Y;
	for (i=0; i < 16; i++) {
		if(pX[i] != pY[i]) {
			res = 0;
			break;
		}
	}
	return res;
}

#ifndef WIN32
void handler(int sig) {
	void *array[10];
	size_t size;

	// get void*'s for all entries on the stack
	size = backtrace(array, 10);

	// print out all the frames to stderr
	fprintf(stderr, "Error: signal %d:\n", sig);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
	exit(1);
}
#endif


/**********************************************************************************************************************
 ** Engine structure wrappers
 **
 */

typedef struct ONYX_ENGINE_OBJECT_CTX_
{
	XAuint32  state;
	void      *pSession;
} ONYX_ENGINE_OBJECT_CTX;

typedef struct ONYX_ENGINE_OBJECT_ITF_
{
	struct XAObjectItf_     mObjItf;
	ONYX_ENGINE_OBJECT_CTX   mCtx;
} ONYX_ENGINE_OBJECT_ITF;

typedef struct ONYX_ENGINE_ENGINE_CTX_
{
	XAuint32  state;
    ConnCtxT  *mSinkConn;
	void      *pSession;
} ONYX_ENGINE_ENGINE_CTX;

typedef struct ONYX_ENGINE_ENGINE_ITF_
{
	struct XAEngineItf_     mEngItf;
	ONYX_ENGINE_ENGINE_CTX   mCtx;
} ONYX_ENGINE_ENGINE_ITF;

/**********************************************************************************************************************
 ** Player structure wrappers
 **
 */

typedef struct ONYX_PLAYER_OBJECT_CTX_
{
	XAuint32  state;
	void      *pSession;
} ONYX_PLAYER_OBJECT_CTX;


typedef struct ONYX_PLAYER_OBJECT_ITF_
{
	struct XAObjectItf_     mObjItf;
	ONYX_PLAYER_OBJECT_CTX    mCtx;
} ONYX_PLAYER_OBJECT_ITF;

typedef struct ONYX_PLAYER_PLAY_CTX_
{
	XAchar    url[MAX_URL_SIZE];
	XAuint32  state;
	XAuint32  event_flags;
	xaPlayCallback callback;
	void *         pCallbackContext;
	void      *pdecCtx;
	void      *pSession;
	int       nSrcType;
	void      *pConnSrcVideo;
	void      *pConnSrcAudio;
} ONYX_PLAYER_PLAY_CTX;

typedef struct ONYX_PLAYER_PLAY_ITF_
{
	struct XAPlayItf_               mPlayItf;
	ONYX_PLAYER_PLAY_CTX            mCtx;
} ONYX_PLAYER_PLAY_ITF;



/**********************************************************************************************************************
 ** Player extension wrappers
 **
 */

typedef struct ONYX_PLAYER_EXT_CTX_
{
	XAVideoStreamInformation vid_input_param;
	XAAudioStreamInformation aud_input_param;
	int                      nLatency;
    int                      nDeinterlace;
    int                      nFrameRate;
	int                      nDetectProgramPids;
	int                      nVidPidOrChan;
	int                      nAudPidOrChan;
	int                      nPcrPidOrProg;
	int                      nSessionId;
	void                    *pSession;
} ONYX_PLAYER_EXT_CTX;

typedef struct ONYX_PLAYER_EXT_ITF_
{
	struct XAConfigExtensionsItf_   mExtItf;
	ONYX_PLAYER_EXT_CTX              mCtx;
} ONYX_PLAYER_EXT_ITF;

typedef struct _OMX_PLAY_SESSION_T
{
	ONYX_PLAYER_OBJECT_ITF           PlayerObj;
	struct XAObjectItf_              *pPlayerObjItf;
	ONYX_PLAYER_PLAY_ITF             PlayerItf;
	struct XAPlayItf_               *pPlayerItf;
	ONYX_PLAYER_EXT_ITF              PlayerExtItf;
	struct XAConfigExtensionsItf_   *pPlayerExtItf;
} OMX_PLAY_SESSION_T;

//=============================================================================
typedef struct RECORDER_OBJECT_CTX_
{
	XAuint32  state;
	void      *pSession;
	ConnCtxT  *mDataSink;
	ConnCtxT  *mAudDataSink;
} RECORDER_OBJECT_CTX;

typedef struct ONYX_RECORDER_OBJECT_ITF_
{
	struct XAObjectItf_     mObjItf;
	RECORDER_OBJECT_CTX    mCtx;
} ONYX_RECORDER_OBJECT_ITF;


typedef struct _OMX_RECORD_SESSION_T
{
	ONYX_RECORDER_OBJECT_ITF           RecorderObj;
	struct XAObjectItf_              *pRecorderObjItf;
} OMX_RECORD_SESSION_T;

OMX_RECORD_SESSION_T *CreateRecorderInternal();

//=============================================================================

typedef struct ONYX_MIXER_OBJECT_CTX_
{
	XAuint32  state;
	void      *pSession;
	ConnCtxT  *mDataSink;
} ONYX_MIXER_OBJECT_CTX;

typedef struct ONYX_MIXER_OBJECT_ITF_
{
	struct XAObjectItf_     mObjItf;
	ONYX_MIXER_OBJECT_CTX    mCtx;
} ONYX_MIXER_OBJECT_ITF;


typedef struct _OMX_MIXER_SESSION_T
{
	ONYX_MIXER_OBJECT_ITF           MixerObj;
	struct XAObjectItf_              *pMixerObjItf;
} OMX_MIXER_SESSION_T;

OMX_MIXER_SESSION_T *CreateMixerInternal();
//=============================================================================
typedef struct _OMX_AL_SESSION_T
{
	ONYX_ENGINE_OBJECT_ITF           EngineObj;               // Main Entry point
	struct XAObjectItf_              *pEngineObjItf;          // pointer to engine object
	ONYX_ENGINE_ENGINE_ITF           EngineItf;               // engine interface v-tbale and cont
	struct XAEngineItf_              *pEngineItf;             // pointer to EngineItf

	//OMX_PLAY_SESSION_T               *mPlaySessionList[MAX_PLAY_SESSIONS];
	//int                              mPlaySessionCount;
} OMX_AL_SESSION_T;



/**********************************************************************************************************************
 ** Engine Object methods
 **
 */

XAresult EngineRealize (
    XAObjectItf self,
    XAboolean async
)
{
	DBG_ASSERT(async == XA_BOOLEAN_FALSE)

	ONYX_ENGINE_OBJECT_CTX *pCtx = &((ONYX_ENGINE_OBJECT_ITF *)(*self))->mCtx;
	pCtx->state = XA_OBJECT_STATE_REALIZED;
	return XA_RESULT_SUCCESS;
}


XAresult EngineResume (
    XAObjectItf self,
    XAboolean async
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}

XAresult EngineGetState (
    XAObjectItf self,
    XAuint32 * pState
)
{
	ONYX_ENGINE_OBJECT_CTX *pCtx = &((ONYX_ENGINE_OBJECT_ITF *)(*self))->mCtx;
	*pState = pCtx->state;
	return XA_RESULT_SUCCESS;
}

XAresult EngineGetInterface (
    XAObjectItf self,
    const XAInterfaceID iid,
    void * pInterface
)
{
	DBG_ASSERT(iid == XA_IID_ENGINE)
	DBG_ASSERT(self != NULL)

	ONYX_ENGINE_OBJECT_ITF *pEngObject = (ONYX_ENGINE_OBJECT_ITF *)(*self);
	OMX_AL_SESSION_T *pSession = (OMX_AL_SESSION_T *)pEngObject->mCtx.pSession;
	*(void **)pInterface = &pSession->pEngineItf;
	return XA_RESULT_SUCCESS;
}

XAresult EngineRegisterCallback (
    XAObjectItf self,
    xaObjectCallback callback,
    void * pContext
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
void EngineAbortAsyncOperation (
    XAObjectItf self
)
{
	
}
void EngineDestroy (
    XAObjectItf self
)
{
	ONYX_ENGINE_OBJECT_ITF *pEngObject = (ONYX_ENGINE_OBJECT_ITF *)(*self);
	OMX_AL_SESSION_T *pSession = (OMX_AL_SESSION_T *)pEngObject->mCtx.pSession;

	if(gpAmix) {
		amixDeinit(gpAmix);
		gpAmix = NULL;
	}
#ifndef WIN32
	if(gpVmix) {
		vmixDeinit(gpVmix);	
		gpVmix = NULL;
	}
#endif

	DBG_LOG(DBGLVL_SETUP, ("Free %p",pSession));
	//free(pSession);
}
XAresult EngineSetPriority (
    XAObjectItf self,
    XAuint32 priority
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
XAresult EngineGetPriority (
    XAObjectItf self,
    XAuint32 * pPriority
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
XAresult EngineSetLossOfControlInterfaces (
    XAObjectItf self,
    XAuint16 numInterfaces,
    const XAInterfaceID * pInterfaceIDs,
    XAboolean enabled
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}

ONYX_ENGINE_OBJECT_ITF gBaseEngineXAObjectItf = {
	{
		EngineRealize,
		EngineResume,
		EngineGetState,
		EngineGetInterface,
		EngineRegisterCallback,
		EngineAbortAsyncOperation,
		EngineDestroy,
		EngineSetPriority,
		EngineGetPriority,
		EngineSetLossOfControlInterfaces
	},
	{
		0
	}
};

/**********************************************************************************************************************
 ** Engine Interface methods
 **
 */

XAresult CreateCameraDevice (
    XAEngineItf self,
    XAObjectItf * pDevice,
    XAuint32 deviceID,
    XAuint32 numInterfaces,
    const XAInterfaceID * pInterfaceIds,
    const XAboolean * pInterfaceRequired
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}

XAresult CreateRadioDevice (
    XAEngineItf self,
    XAObjectItf * pDevice,
    XAuint32 numInterfaces,
    const XAInterfaceID * pInterfaceIds,
    const XAboolean * pInterfaceRequired
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
XAresult CreateLEDDevice (
    XAEngineItf self,
    XAObjectItf * pDevice,
    XAuint32 deviceID,
    XAuint32 numInterfaces,
    const XAInterfaceID * pInterfaceIds,
    const XAboolean * pInterfaceRequired
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
    XAresult CreateVibraDevice (
    XAEngineItf self,
    XAObjectItf * pDevice,
    XAuint32 deviceID,
    XAuint32 numInterfaces,
    const XAInterfaceID * pInterfaceIds,
    const XAboolean * pInterfaceRequired
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}


OMX_PLAY_SESSION_T *CreatePlayerInternal();

XAresult CreateMediaPlayer (
    XAEngineItf self,
    XAObjectItf *pPlayer,
    const XADataSource *pDataSrc,
    const XADataSource *pBankSrc,
    const XADataSink * pAudioSnk,
    const XADataSink * pImageVideoSnk,
    const XADataSink * pVibra,
    const XADataSink * pLEDArray,
    XAuint32 numInterfaces,
    const XAInterfaceID * pInterfaceIds,
    const XAboolean * pInterfaceRequired
)
{
	OMX_PLAY_SESSION_T *pSession = NULL;
	XADataLocator_Null *pUnKnown = NULL;
	ONYX_PLAYER_PLAY_CTX *pPlayerPlay = NULL;

	DBG_LOG(DBGLVL_SETUP, ("Enter"));
	DBG_ASSERT(pDataSrc != NULL)
	DBG_ASSERT(pDataSrc->pLocator != NULL)
	//ONYX_ENGINE_ENGINE_ITF *pEngItfObj = (ONYX_ENGINE_ENGINE_ITF *)(*self);
	//OMX_AL_SESSION_T *pSession = (OMX_AL_SESSION_T *)pEngItfObj->mCtx.pSession;
	pSession = CreatePlayerInternal();

	pUnKnown = (XADataLocator_Null *)pDataSrc->pLocator;
	pPlayerPlay = &pSession->PlayerItf.mCtx;

	if(pUnKnown->locatorType == XA_DATALOCATOR_URI) {
		XADataLocator_URI *locUri;
		DBG_LOG(DBGLVL_SETUP, ("Source is URI"));
		locUri = pDataSrc->pLocator;
		strncpy(pPlayerPlay->url, locUri->pURI, MAX_URL_SIZE - 1);
		pPlayerPlay->nSrcType = DEC_SRC_URI;
		DBG_LOG(DBGLVL_SETUP, ("Source is URI %s", locUri->pURI));
	} else if (pUnKnown->locatorType == XA_DATALOCATOR_ADDRESS) {
		XADataLocator_Address *pVidLocator;
		XADataLocator_Address *pAudLocator;

		DBG_LOG(DBGLVL_SETUP, ("Source is Stream Conn"));
		pVidLocator = (XADataLocator_Address *)pDataSrc->pLocator;
		pAudLocator = (XADataLocator_Address *)pBankSrc->pLocator;
		if(pVidLocator){
			pPlayerPlay->pConnSrcVideo = pVidLocator->pAddress;
		}
		if(pAudLocator) {
			pPlayerPlay->pConnSrcAudio = pAudLocator->pAddress;
		}
		DBG_LOG(DBGLVL_SETUP, ("Stream Conn vid=%p aud=%p", pPlayerPlay->pConnSrcVideo, pPlayerPlay->pConnSrcAudio));
		pPlayerPlay->nSrcType = DEC_SRC_STRM_CONN;
	}
	*pPlayer = &pSession->pPlayerObjItf;
	DBG_LOG(DBGLVL_SETUP, ("Leave"));
	return XA_RESULT_SUCCESS;
}


XAresult CreateMediaRecorderWithIpcConn (
	RECORDER_OBJECT_CTX *pCtx,
	const char *pszAudLocalPort,
	const char *pszAudPeerPort,
	const char *pszVidLocalPort,
	const char *pszVidPeerPort
)
{
	if(strlen(pszAudLocalPort) && strlen(pszAudPeerPort)) {
		DBG_LOG(DBGLVL_SETUP, ("pszAudLocalPort=%s pszAudPeerPort=%s",pszAudLocalPort,pszAudPeerPort));
		pCtx->mAudDataSink = CreateIpcStrmConn(pszAudLocalPort, pszAudPeerPort, 16*1024);
	}
	if(strlen(pszVidLocalPort) && strlen(pszVidPeerPort)) {
		DBG_LOG(DBGLVL_SETUP, ("pszVidLocalPort=%s pszVidPeerPort=%s",pszVidLocalPort,pszVidPeerPort));
		pCtx->mDataSink = CreateIpcStrmConn(pszVidLocalPort, pszVidPeerPort, 1024*1024);
	}

	return XA_RESULT_SUCCESS;
}

XAresult DeleteMediaRecorderWithIpcConn (RECORDER_OBJECT_CTX *pCtx)
{
	if(pCtx->mDataSink){
#ifndef WIN32
		if(gpVmix) {
			vmixSetStreamSink(gpVmix, NULL);
		}
		DeleteIpcStrmConn(pCtx->mDataSink );
#endif
	}
	if(pCtx->mAudDataSink)
		if(gpAmix){
			amixSetStreamSink(gpAmix, NULL);
		}
		DeleteIpcStrmConn(pCtx->mAudDataSink);
	return XA_RESULT_SUCCESS;
}

XAresult CreateMediaRecorder (
    XAEngineItf self,
    XAObjectItf * pRecorder,
    const XADataSource * pAudioSrc,
    const XADataSource * pImageVideoSrc,
    const XADataSink * pDataSnk,
    XAuint32 numInterfaces,
    const XAInterfaceID * pInterfaceIds,
    const XAboolean * pInterfaceRequired
)
{
	//ONYX_ENGINE_ENGINE_ITF *pEngItfObj = (ONYX_ENGINE_ENGINE_ITF *)(*self);
	//OMX_AL_SESSION_T *pSession = (OMX_AL_SESSION_T *)pEngItfObj->mCtx.pSession;
	int nEnableISrmConnIpc = 0;
	RECORD_SINK_T          *pRecSink = (RECORD_SINK_T *)pDataSnk;
	OMX_RECORD_SESSION_T *pSession = CreateRecorderInternal();
	RECORDER_OBJECT_CTX *pCtx = &pSession->RecorderObj.mCtx;
	//XADataLocator_URI *locUri = pDataSnk->pLocator;
	//strncpy(pPlayerPlay->url, locUri->pURI, MAX_URL_SIZE - 1);
	*pRecorder = &pSession->pRecorderObjItf;

	nEnableISrmConnIpc = ini_getl(ENCODE_SECTION, "strmconn_ipc", 0, gIniFile);

	if(nEnableISrmConnIpc)	{
		char szVidPortLocal[IPC_PORT_NAME_SIZE];
		char szVidPortPeer[IPC_PORT_NAME_SIZE];
		char szAudPortLocal[IPC_PORT_NAME_SIZE];
		char szAudPortPeer[IPC_PORT_NAME_SIZE];

		ini_gets(ENCODE_SECTION, "audio_port_local", "", szAudPortLocal, IPC_PORT_NAME_SIZE, gIniFile);
		ini_gets(ENCODE_SECTION, "audio_port_peer", "", szAudPortPeer, IPC_PORT_NAME_SIZE, gIniFile);
		ini_gets(ENCODE_SECTION, "video_port_local", "", szVidPortLocal, IPC_PORT_NAME_SIZE, gIniFile);
		ini_gets(ENCODE_SECTION, "video_port_peer", "", szVidPortPeer, IPC_PORT_NAME_SIZE, gIniFile);
		DBG_LOG(DBGLVL_SETUP, ("Starting IPC Stream Connection"));
		CreateMediaRecorderWithIpcConn (pCtx, szAudPortLocal,szAudPortPeer,szVidPortLocal,szVidPortPeer);
	} else {
		DBG_LOG(DBGLVL_SETUP, ("IPC Stream Connection not enabled."));
		pCtx->mDataSink = (ConnCtxT *)pRecSink->pVidSink;
		pCtx->mAudDataSink = pRecSink->pAudSink;
	}
#ifndef WIN32
	if(gpVmix && pCtx->mDataSink) {
		vmixSetStreamSink(gpVmix, pCtx->mDataSink);
	}
#endif
	if(gpAmix && pCtx->mAudDataSink) {
		amixSetStreamSink(gpAmix, pCtx->mAudDataSink);
	}
	return XA_RESULT_SUCCESS;
}

XAresult CreateOutputMix (
    XAEngineItf self,
    XAObjectItf * pMix,
    XAuint32 numInterfaces,
    const XAInterfaceID * pInterfaceIds,
    const XAboolean * pInterfaceRequired
)
{
	OMX_MIXER_SESSION_T *pSession = CreateMixerInternal();
	*pMix = &pSession->pMixerObjItf;
	return XA_RESULT_SUCCESS;
}

XAresult CreateMetadataExtractor (
    XAEngineItf self,
    XAObjectItf * pMetadataExtractor,
    const XADataSource * pDataSource,
    XAuint32 numInterfaces,
    const XAInterfaceID * pInterfaceIds,
    const XAboolean * pInterfaceRequired
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
XAresult CreateExtensionObject (
    XAEngineItf self,
    XAObjectItf * pObject,
    void * pParameters,
    XAuint32 objectID,
    XAuint32 numInterfaces,
    const XAInterfaceID * pInterfaceIds,
    const XAboolean * pInterfaceRequired
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
XAresult GetImplementationInfo (
    XAEngineItf self,
    XAuint32 * pMajor,
    XAuint32 * pMinor,
    XAuint32 * pStep,
            XAuint32 * pImplementationTextSize,
    XAchar * pImplementationText
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
XAresult QuerySupportedProfiles (
    XAEngineItf self,
    XAint16 * pProfilesSupported
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
XAresult QueryNumSupportedInterfaces (
    XAEngineItf self,
    XAuint32 objectID,
    XAuint32 * pNumSupportedInterfaces
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
XAresult QuerySupportedInterfaces (
    XAEngineItf self,
    XAuint32 objectID,
    XAuint32 index,
    XAInterfaceID * pInterfaceId
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
XAresult QueryNumSupportedExtensions (
    XAEngineItf self,
    XAuint32 * pNumExtensions
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
XAresult QuerySupportedExtension (
    XAEngineItf self,
    XAuint32 index,
    XAchar * pExtensionName,
    XAuint16 * pNameLength
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
XAresult IsExtensionSupported (
    XAEngineItf self,
    const XAchar * pExtensionName,
    XAboolean * pSupported
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
XAresult QueryLEDCapabilities (
    XAEngineItf self,
    XAuint32 *pIndex,
    XAuint32 * pLEDDeviceID,
    XALEDDescriptor * pDescriptor
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
XAresult QueryVibraCapabilities (
    XAEngineItf self,
    XAuint32 *pIndex,
    XAuint32 * pVibraDeviceID,
    XAVibraDescriptor * pDescriptor
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}

/*
** ONYX_ENGINE_ENGINE_ITF class
** OpenMAX AL engine v-table + engine context ONYX_ENGINE_ENGINE_CTX
*/
ONYX_ENGINE_ENGINE_ITF gBaseEngineXAEngineItf = {
	{
		CreateCameraDevice,
		CreateRadioDevice,
		CreateLEDDevice,
		CreateVibraDevice,
		CreateMediaPlayer,
		CreateMediaRecorder,
		CreateOutputMix,
		CreateMetadataExtractor,
		CreateExtensionObject,
		GetImplementationInfo,
		QuerySupportedProfiles,
		QueryNumSupportedInterfaces,
		QuerySupportedInterfaces,
		QueryNumSupportedExtensions,
		QuerySupportedExtension,
		IsExtensionSupported,
		QueryLEDCapabilities,
		QueryVibraCapabilities
	},
	{
		0
	}
};



/**********************************************************************************************************************
 ** Player Object methods
 **
 */

XAresult PlayRealize (
    XAObjectItf self,
    XAboolean async
)
{
	DBG_ASSERT(async == XA_BOOLEAN_FALSE)

	ONYX_PLAYER_OBJECT_CTX *pCtx = &((ONYX_PLAYER_OBJECT_ITF *)(*self))->mCtx;
	pCtx->state = XA_OBJECT_STATE_REALIZED;
	return XA_RESULT_SUCCESS;
}


XAresult PlayResume (
    XAObjectItf self,
    XAboolean async
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}

XAresult PlayGetState (
    XAObjectItf self,
    XAuint32 * pState
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
XAresult PlayGetInterface (
    XAObjectItf self,
    const XAInterfaceID iid,
    void * pInterface
)
{
	XAresult res = XA_RESULT_SUCCESS;
	ONYX_PLAYER_OBJECT_ITF *pPlayerObj = (ONYX_PLAYER_OBJECT_ITF *)(*self);
	OMX_PLAY_SESSION_T *pSession = (OMX_PLAY_SESSION_T *)pPlayerObj->mCtx.pSession;
	if(IS_EQUAL_IID(iid, XA_IID_PLAY)) {
		*(void **)pInterface = &pSession->pPlayerItf;
	} else if(IS_EQUAL_IID(iid, XA_IID_CONFIGEXTENSION)) {
		*(void **)pInterface = &pSession->pPlayerExtItf;
	} else {
		res = XA_RESULT_PARAMETER_INVALID;
	}
	return res;
}

XAresult PlayRegisterCallback (
    XAObjectItf self,
    xaObjectCallback callback,
    void * pContext
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
void PlayAbortAsyncOperation (
    XAObjectItf self
)
{
	
}
void PlayDestroy (
    XAObjectItf self
)
{
	ONYX_PLAYER_OBJECT_ITF *pPlayerObj = (ONYX_PLAYER_OBJECT_ITF *)(*self);
	OMX_PLAY_SESSION_T *pSession = (OMX_PLAY_SESSION_T *)pPlayerObj->mCtx.pSession;	
	DBG_LOG(DBGLVL_SETUP, ("Free %p",pSession));
	free(pSession);
}
XAresult PlaySetPriority (
    XAObjectItf self,
    XAuint32 priority
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
XAresult PlayGetPriority (
    XAObjectItf self,
    XAuint32 * pPriority
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
XAresult PlaySetLossOfControlInterfaces (
    XAObjectItf self,
    XAuint16 numInterfaces,
    const XAInterfaceID * pInterfaceIDs,
    XAboolean enabled
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}

ONYX_PLAYER_OBJECT_ITF   gBasePlayXAObjectItf = 
{
	{
		PlayRealize,
		PlayResume,
		PlayGetState,
		PlayGetInterface,
		PlayRegisterCallback,
		PlayAbortAsyncOperation,
		PlayDestroy,
		PlaySetPriority,
		PlayGetPriority,
		PlaySetLossOfControlInterfaces
	},
	{
		 0
	}
};


/**********************************************************************************************************************
 ** Player Play Interface methods
 **
 */

void stream_event_callback(void *pContext, int nStrmEvent)
{
	ONYX_PLAYER_PLAY_ITF *pPlayerPlay = (ONYX_PLAYER_PLAY_ITF *)pContext;
	ONYX_PLAYER_PLAY_CTX *pPlayCtx = &pPlayerPlay->mCtx;
	DBG_PRINT("omaxal:stream_event_callback:Enter\n");
	
	// TODO: Add mutex

	if(pPlayCtx->callback) {
		switch(nStrmEvent)
		{
			case STRM_EVENT_EOS:
				pPlayCtx->callback(pPlayerPlay, pPlayCtx->pCallbackContext, XA_PLAYEVENT_HEADATEND);
				break;
			case STRM_EVENT_OMX_ERROR:
				pPlayCtx->callback(pPlayerPlay, pPlayCtx->pCallbackContext, XA_PLAYEVENT_HEADSTALLED);
				break;
			case STRM_EVENT_DISCONTINUITY:
				pPlayCtx->callback(pPlayerPlay, pPlayCtx->pCallbackContext, XA_PLAYEVENT_HEADATNEWPOS);
				break;
		}
	}
	DBG_PRINT("omaxal:stream_event_callback:Leave\n");
}

XAresult SetPlayState (
    XAPlayItf self,
    XAuint32 state
)
{
	int res = 0;
	ONYX_PLAYER_PLAY_ITF *pPlayerPlay = (ONYX_PLAYER_PLAY_ITF *)(*self);
	OMX_PLAY_SESSION_T *pSession = (OMX_PLAY_SESSION_T *)pPlayerPlay->mCtx.pSession;
	ONYX_PLAYER_PLAY_CTX *pCtx = &pPlayerPlay->mCtx;
	ONYX_PLAYER_EXT_CTX *pPlayExtCfg = &pSession->PlayerExtItf.mCtx;
	ONYX_PLAYER_PLAY_CTX *pPlayCfg = &pPlayerPlay->mCtx;

	XAVideoStreamInformation *pVidCfg = &pPlayExtCfg->vid_input_param;
	XAAudioStreamInformation *pAudCfg = &pPlayExtCfg->aud_input_param;

	void *pVidInput = NULL; 
	void *pAudInput = NULL;

	XAresult xa_res = XA_RESULT_SUCCESS;

	pCtx->state = state;


	if(pPlayCfg->nSrcType == DEC_SRC_URI) {
		pVidInput = (void *)pPlayCfg->url; 
		pAudInput = (void *)pPlayCfg->url; 
	} else if(pPlayCfg->nSrcType == DEC_SRC_STRM_CONN) {
		pVidInput = pPlayCfg->pConnSrcVideo; 
		pAudInput = pPlayCfg->pConnSrcAudio;
	}

	if(state == XA_PLAYSTATE_PLAYING) {
			char szVidCodecName[32] = "h264"; // defualt
			char szAudCodecName[32] = "null";//"aaclc"; // defualt


			if(pVidCfg->codecId == XA_VIDEOCODEC_AVC) {
				strcpy(szVidCodecName,"h264");
			} else if(pVidCfg->codecId == XA_VIDEOCODEC_MPEG2) {
				strcpy(szVidCodecName,"mpeg2");
			}
			if(pAudCfg->codecId == XA_AUDIOCODEC_AAC) {
				strcpy(szAudCodecName,"aaclc");
			} else if(pAudCfg->codecId == XA_AUDIOCODEC_PCM) {
				strcpy(szAudCodecName,"g711u");
			}
#ifdef EN_DDPDEC
			else if(pAudCfg->codecId == 10/*XA_AUDIOCODEC_AC3*/) {
				strcpy(szAudCodecName,"ac3");
			}
#endif
			DBG_LOG(DBGLVL_SETUP, ("Streaming source=%s",pPlayCfg->url));
			DBG_LOG(DBGLVL_SETUP, ("Streaming vid_codec=%s aud_codec=%s w=%d h=%d interlace=%d",szVidCodecName, szAudCodecName, pVidCfg->width, pVidCfg->height, pPlayExtCfg->nDeinterlace));
			DBG_LOG(DBGLVL_SETUP, ("Streaming VidFrameRate=%d AudSampleRate=%d latency(avsync)=%d",pPlayExtCfg->nFrameRate, pAudCfg->sampleRate, pPlayExtCfg->nLatency));
			DBG_LOG(DBGLVL_SETUP, ("Streaming DetectProg=%d pcr_pid_or_prog=%d aud_pid_or_chan=%d vid_pid_or_chan=%d",pPlayExtCfg->nDetectProgramPids, pPlayExtCfg->nPcrPidOrProg, pPlayExtCfg->nAudPidOrChan, pPlayExtCfg->nVidPidOrChan));
			pCtx->pdecCtx = decmainCreateStream();
			res = decmainAcquireResource(
							pCtx->pdecCtx,
							pPlayExtCfg->nSessionId,
							pPlayCfg->nSrcType,
							pVidInput, 
							szVidCodecName,
							pPlayCfg->nSrcType,
							pAudInput,
							szAudCodecName,
							pVidCfg->width, pVidCfg->height, 
							gDispWidth, gDispHeight,
							pAudCfg->sampleRate,
							pPlayExtCfg->nLatency, pPlayExtCfg->nDeinterlace, pPlayExtCfg->nFrameRate, 
							pPlayExtCfg->nDetectProgramPids,
							pPlayExtCfg->nPcrPidOrProg, pPlayExtCfg->nAudPidOrChan, pPlayExtCfg->nVidPidOrChan, 
							stream_event_callback, 
							pPlayerPlay);
			if(res < 0)
				xa_res = XA_RESULT_UNKNOWN_ERROR;
			// TODO: Move it a common trigger place
			decmainStartStreaming(pCtx->pdecCtx);

	} else if(state == XA_PLAYSTATE_STOPPED) {
		decmainStopStreaming(pCtx->pdecCtx);
		decmainDeleteStream(pCtx->pdecCtx);
	}
	return xa_res;
}

XAresult GetPlayState (
    XAPlayItf self,
    XAuint32 *pState
)
{
	ONYX_PLAYER_PLAY_ITF *pPlayerPlay = (ONYX_PLAYER_PLAY_ITF *)(*self);
	ONYX_PLAYER_PLAY_CTX *pCtx = &pPlayerPlay->mCtx;

	*pState = pCtx->state;

	return XA_RESULT_SUCCESS;
}

XAresult GetDuration (
    XAPlayItf self,
    XAmillisecond * pMsec
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}

XAresult GetPosition (
    XAPlayItf self,
    XAmillisecond * pMsec
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}

XAresult playerRegisterCallback (
    XAPlayItf self,
    xaPlayCallback callback,
    void * pContext
)
{
	ONYX_PLAYER_PLAY_ITF *pPlayerPlay = (ONYX_PLAYER_PLAY_ITF *)(*self);
	ONYX_PLAYER_PLAY_CTX *pCtx = &pPlayerPlay->mCtx;
	pCtx->callback = callback;
	pCtx->pCallbackContext = pContext;
	return XA_RESULT_SUCCESS;

}

XAresult SetCallbackEventsMask (
    XAPlayItf self,
    XAuint32 eventFlags
)
{
	ONYX_PLAYER_PLAY_ITF *pPlayerPlay = (ONYX_PLAYER_PLAY_ITF *)(*self);
	ONYX_PLAYER_PLAY_CTX *pCtx = &pPlayerPlay->mCtx;
	pCtx->event_flags = eventFlags;

	return XA_RESULT_SUCCESS;
}

XAresult GetCallbackEventsMask (
    XAPlayItf self,
    XAuint32 * pEventFlags
)
{
	ONYX_PLAYER_PLAY_ITF *pPlayerPlay = (ONYX_PLAYER_PLAY_ITF *)(*self);
	ONYX_PLAYER_PLAY_CTX *pCtx = &pPlayerPlay->mCtx;
	*pEventFlags = pCtx->event_flags;

	return XA_RESULT_SUCCESS;
}

XAresult SetMarkerPosition(
    XAPlayItf self,
    XAmillisecond mSec
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}

XAresult ClearMarkerPosition (
    XAPlayItf self
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}

XAresult GetMarkerPosition (
    XAPlayItf self,
    XAmillisecond * pMsec
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}

XAresult SetPositionUpdatePeriod(
    XAPlayItf self,
    XAmillisecond mSec
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}

XAresult GetPositionUpdatePeriod
(
    XAPlayItf self,
    XAmillisecond * pMsec
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}

ONYX_PLAYER_PLAY_ITF   gBasePlayerXAPlayItf = 
{
	{
		SetPlayState,
		GetPlayState, 
		GetDuration, 
		GetPosition, 
		playerRegisterCallback,
		SetCallbackEventsMask, 
		GetCallbackEventsMask, 
		SetMarkerPosition,
		ClearMarkerPosition,
		GetMarkerPosition,
		SetPositionUpdatePeriod, 
		GetPositionUpdatePeriod
	},
	{
		 0
	}
};


/**********************************************************************************************************************
 ** Player Ext Interface methods
 **
 */

XAresult SetConfiguration (
    XAConfigExtensionsItf self,
    const XAchar * pConfigKey,
    XAuint32 valueSize,
    const void * pConfigValue
)
{
	ONYX_PLAYER_EXT_ITF *pPlayerExt = (ONYX_PLAYER_EXT_ITF *)(*self);
	ONYX_PLAYER_EXT_CTX *pCtx = &pPlayerExt->mCtx;
	if (strcmp(pConfigKey, VID_INPUT_PARAM) == 0) {
		XAVideoStreamInformation *pDescript = (XAVideoStreamInformation *)pConfigValue;
		XAVideoStreamInformation *pCrntDescript = &pCtx->vid_input_param;
		*pCrntDescript = *pDescript;
		DBG_LOG(DBGLVL_TRACE, ("vid_input_param w=%d h=%d codecId=%d",pCrntDescript->width, pCrntDescript->height, pCrntDescript->codecId));
	} else 	if (strcmp(pConfigKey, AUD_INPUT_PARAM) == 0) {
		XAAudioStreamInformation *pDescript = (XAAudioStreamInformation *)pConfigValue;
		XAAudioStreamInformation *pCrntDescript = &pCtx->aud_input_param;
		*pCrntDescript = *pDescript;
		DBG_LOG(DBGLVL_TRACE, ("aud_input_param sampleRate=%d codecId=%d",pCrntDescript->sampleRate, pCrntDescript->codecId));
	} else 	if (strcmp(pConfigKey, LATENCY) == 0) {
		pCtx->nLatency = *((int *)pConfigValue);
		DBG_LOG(DBGLVL_TRACE, ("latency=%d",pCtx->nLatency));
	} else 	if (strcmp(pConfigKey, DEINTERLACE) == 0) {
		pCtx->nDeinterlace = *((int *)pConfigValue);
		DBG_LOG(DBGLVL_TRACE, ("deinterlace=%d",pCtx->nDeinterlace));
	} else 	if (strcmp(pConfigKey, DEMUX_SELECT_PROG) == 0) {
		pCtx->nDetectProgramPids = *((int *)pConfigValue);
		pCtx->nPcrPidOrProg = *((int *)pConfigValue);
		DBG_LOG(DBGLVL_TRACE, ("demux_select_prog=%d",pCtx->nDetectProgramPids));
	} else 	if (strcmp(pConfigKey, FRAMERATE) == 0) {
		// TODO: Remove this
		pCtx->nFrameRate = *((int *)pConfigValue);
		DBG_LOG(DBGLVL_TRACE, ("framerate=%d",pCtx->nFrameRate));
	} else 	if (strcmp(pConfigKey, SESSION_ID) == 0) {
		// TODO: Remove this
		pCtx->nSessionId = *((int *)pConfigValue);
		DBG_LOG(DBGLVL_TRACE, ("framerate=%d",pCtx->nFrameRate));
	}  else if (strcmp(pConfigKey, A8_DEBUG_LEVEL) == 0) {
		gDbgLevel = *((int *)pConfigValue);
		DBG_LOG(DBGLVL_TRACE, ("a8_debug_level=%d",gDbgLevel));
	}

	
	return XA_RESULT_SUCCESS;
}

XAresult GetConfiguration (
        XAConfigExtensionsItf self,
        const XAchar * pConfigKey,
        XAuint32 * pValueSize,
        void * pConfigValue
    )
{
	ONYX_PLAYER_EXT_ITF *pPlayerExt = (ONYX_PLAYER_EXT_ITF *)(*self);
	ONYX_PLAYER_EXT_CTX *pCtx = &pPlayerExt->mCtx;
	if (strcmp(pConfigKey, VID_INPUT_PARAM) == 0) {
		XAVideoStreamInformation *pDescript = (XAVideoStreamInformation *)pConfigValue;
		XAVideoStreamInformation *pCrntDescript = &pCtx->vid_input_param;
		*pDescript = *pCrntDescript;
	} else if(strcmp(pConfigKey, AUD_INPUT_PARAM) == 0) {
		XAAudioStreamInformation *pDescript = (XAAudioStreamInformation *)pConfigValue;
		XAAudioStreamInformation *pCrntDescript = &pCtx->aud_input_param;
		*pDescript = *pCrntDescript;
	} else 	if (strcmp(pConfigKey, LATENCY) == 0) {
		*((int *)pConfigValue) = pCtx->nLatency;
	} else 	if (strcmp(pConfigKey, "latency") == 0) {
		*((int *)pConfigValue) = pCtx->nDeinterlace;
	} else 	if (strcmp(pConfigKey, VERSION) == 0) {
		*((int *)pConfigValue) = ONYX_OMX_VERSION;
	}
	
	return XA_RESULT_SUCCESS;
}


ONYX_PLAYER_EXT_ITF gBasePlayerXAExtItf =
{
	{
		SetConfiguration,
		GetConfiguration
	},
	{
		{0},       // vid_input_param
		{0},       // aud_input_param
		0,         // nLatency 
		0,         // nDeinterlace
		0,         // nFrameRate
		1,         // nDetectProgramPids
		1,         // PCR PID or program
		1,         // AUD PID or audio channel
		1          // VID PID or video channel
	}

};


OMX_AL_SESSION_T *CreateEngineInternal()
{
	OMX_AL_SESSION_T *pSession = (OMX_AL_SESSION_T *)malloc(sizeof(OMX_AL_SESSION_T));

	memcpy(&pSession->EngineObj, &gBaseEngineXAObjectItf, sizeof(ONYX_ENGINE_OBJECT_ITF));
	memcpy(&pSession->EngineItf, &gBaseEngineXAEngineItf,sizeof(ONYX_ENGINE_ENGINE_ITF));
	pSession->EngineObj.mCtx.pSession = pSession;
	pSession->EngineItf.mCtx.pSession = pSession;
	pSession->pEngineObjItf = &pSession->EngineObj.mObjItf;
	pSession->pEngineItf = &pSession->EngineItf.mEngItf;
	return pSession;
}

OMX_PLAY_SESSION_T *CreatePlayerInternal()
{
	OMX_PLAY_SESSION_T *pSession = (OMX_PLAY_SESSION_T *)malloc(sizeof(OMX_PLAY_SESSION_T));

	memcpy(&pSession->PlayerObj, &gBasePlayXAObjectItf,sizeof(ONYX_PLAYER_OBJECT_ITF));
	memcpy(&pSession->PlayerItf, &gBasePlayerXAPlayItf,sizeof(ONYX_PLAYER_PLAY_ITF));
	memcpy(&pSession->PlayerExtItf, &gBasePlayerXAExtItf, sizeof(ONYX_PLAYER_EXT_ITF));
	pSession->PlayerObj.mCtx.pSession = pSession;
	pSession->PlayerItf.mCtx.pSession = pSession;
	pSession->PlayerExtItf.mCtx.pSession = pSession;
	pSession->pPlayerObjItf = &pSession->PlayerObj.mObjItf;
	pSession->pPlayerItf = &pSession->PlayerItf.mPlayItf;
	pSession->pPlayerExtItf = &pSession->PlayerExtItf.mExtItf;
	return pSession;
}


//=============================================================================================
XAresult RecordRealize ( XAObjectItf self,  XAboolean async)
{
	return XA_RESULT_SUCCESS;
}


XAresult RecordResume ( XAObjectItf self,  XAboolean async)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}

XAresult RecordGetState (XAObjectItf self, XAuint32 * pState
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
XAresult RecordGetInterface (XAObjectItf self,  const XAInterfaceID iid,  void * pInterface)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}

XAresult RecordRegisterCallback (XAObjectItf self, xaObjectCallback callback, void * pContext)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
void RecordAbortAsyncOperation (XAObjectItf self)
{
	
}

void RecordDestroy ( XAObjectItf self)
{
	ONYX_RECORDER_OBJECT_ITF *pObj = (ONYX_RECORDER_OBJECT_ITF *)(*self);
	OMX_RECORD_SESSION_T *pSession = (OMX_RECORD_SESSION_T *)pObj->mCtx.pSession;	
	DBG_LOG(DBGLVL_SETUP, ("Free %p",pSession));
	free(pSession);
}

XAresult RecordSetPriority ( XAObjectItf self, XAuint32 priority)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
XAresult RecordGetPriority (XAObjectItf self, XAuint32 * pPriority)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
XAresult RecordSetLossOfControlInterfaces (XAObjectItf self, XAuint16 numInterfaces, const XAInterfaceID * pInterfaceIDs, XAboolean enabled)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}


ONYX_RECORDER_OBJECT_ITF   gBaseRecordXAObjectItf = 
{
	{
		RecordRealize,
		RecordResume,
		RecordGetState,
		RecordGetInterface,
		RecordRegisterCallback,
		RecordAbortAsyncOperation,
		RecordDestroy,
		RecordSetPriority,
		RecordGetPriority,
		RecordSetLossOfControlInterfaces
	},
	{
		 0
	}
};

OMX_RECORD_SESSION_T *CreateRecorderInternal()
{
	OMX_RECORD_SESSION_T *pSession = (OMX_RECORD_SESSION_T *)malloc(sizeof(OMX_RECORD_SESSION_T));
	memcpy(&pSession->RecorderObj, &gBaseRecordXAObjectItf,sizeof(ONYX_RECORDER_OBJECT_ITF));
	pSession->RecorderObj.mCtx.pSession = pSession;
	pSession->pRecorderObjItf = &pSession->RecorderObj.mObjItf;
	return pSession;
}


//=================================================================================================================
//======================================= MXER ====================================================================

XAresult MixerRealize ( XAObjectItf self,  XAboolean async)
{
	return XA_RESULT_SUCCESS;
}


XAresult MixerResume ( XAObjectItf self,  XAboolean async)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}

XAresult MixerGetState (XAObjectItf self, XAuint32 * pState
)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
XAresult MixerGetInterface (XAObjectItf self,  const XAInterfaceID iid,  void * pInterface)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}

XAresult MixerRegisterCallback (XAObjectItf self, xaObjectCallback callback, void * pContext)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
void MixerAbortAsyncOperation (XAObjectItf self)
{
	
}

void MixerDestroy ( XAObjectItf self)
{
	ONYX_MIXER_OBJECT_ITF *pObj = (ONYX_MIXER_OBJECT_ITF *)(*self);
	OMX_MIXER_SESSION_T *pSession = (OMX_MIXER_SESSION_T *)pObj->mCtx.pSession;	
	DBG_LOG(DBGLVL_SETUP, ("Free %p",pSession));
	free(pSession);
}

XAresult MixerSetPriority ( XAObjectItf self, XAuint32 priority)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
XAresult MixerGetPriority (XAObjectItf self, XAuint32 * pPriority)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}
XAresult MixerSetLossOfControlInterfaces (XAObjectItf self, XAuint16 numInterfaces, const XAInterfaceID * pInterfaceIDs, XAboolean enabled)
{
	return XA_RESULT_FEATURE_UNSUPPORTED;
}


ONYX_MIXER_OBJECT_ITF   gBaseMixerXAObjectItf = 
{
	{
		MixerRealize,
		MixerResume,
		MixerGetState,
		MixerGetInterface,
		MixerRegisterCallback,
		MixerAbortAsyncOperation,
		MixerDestroy,
		MixerSetPriority,
		MixerGetPriority,
		MixerSetLossOfControlInterfaces
	},
	{
		 0
	}
};


OMX_MIXER_SESSION_T *CreateMixerInternal()
{
	OMX_MIXER_SESSION_T *pSession = (OMX_MIXER_SESSION_T *)malloc(sizeof(OMX_MIXER_SESSION_T));
	memcpy(&pSession->MixerObj, &gBaseMixerXAObjectItf,sizeof(OMX_MIXER_SESSION_T));
	pSession->MixerObj.mCtx.pSession = pSession;
	pSession->pMixerObjItf = &pSession->MixerObj.mObjItf;
	return pSession;
}

WINDOW_PARAM_T *layoutGetUserDefined(
	DISP_WINDOW_LIST *pWndList,
	int            *pnNumWindows)
{
	int i;
	char szLayoutName[32];
	char szWndName[32];
	int nDefWidth = 720;
	int nDefHeight = 480;
	int nNumWindows = 0;
	WINDOW_PARAM_T *listWnd = NULL;
	WINDOW_PARAM_T *pWnd;

	DISP_WINDOW *pInWnd = pWndList->pWndList;
	nNumWindows = pWndList->nNumWnd; 

	if(nNumWindows > 0 && nNumWindows < 6) {
		listWnd = (WINDOW_PARAM_T *)malloc(nNumWindows * sizeof(WINDOW_PARAM_T));
		pWnd = listWnd;
		for (i=0; i < nNumWindows; i++) {
			pWnd->nStrmSrc = pInWnd->nStrmSrc;
			pWnd->nStartX = pInWnd->nStartX;
			pWnd->nStartY = pInWnd->nStartY;
			pWnd->nWidth = pInWnd->nWidth;
			pWnd->nHeight = pInWnd->nHeight;
			pWnd->nBufferCount = 6;
			pWnd->nStride = pWnd->nWidth * 2;
			pWnd++;
			pInWnd++;
		}
		*pnNumWindows = nNumWindows;
	}
	
	return listWnd;
}


ACHAN_PARAM_T *alayoutGetUserDefined(
	AUD_CHAN_LIST *pChanList,
	int            *pnNumChannels)
{
	int i;
	char szLayoutName[32];
	char szWndName[32];
	int nDefWidth = 720;
	int nDefHeight = 480;
	int nNumChannels = 0;
	ACHAN_PARAM_T *pChan = NULL;
	ACHAN_PARAM_T *pAChanParamList = NULL;

	AUD_CHAN_T *pInChan = pChanList->pAChanList;
	nNumChannels = pChanList->nNumChan; 

	if(nNumChannels > 0 && nNumChannels < 6) {
		pAChanParamList = (ACHAN_PARAM_T *)malloc(nNumChannels * sizeof(ACHAN_PARAM_T));
		pChan = pAChanParamList;
		for (i=0; i < nNumChannels; i++) {
			pChan->nFormat = pInChan->nFormat;
			pChan->nSampleRate = pInChan->nSampleRate;
			pChan++;
			pInChan++;
		}
		*pnNumChannels = nNumChannels;
	}
	
	return pAChanParamList;
}

/**********************************************************************************************************************
 ** Global Entry points
 **
 */
XA_API XAresult XAAPIENTRY xaCreateEngine(
    XAObjectItf          *pEngine,
    XAuint32              numOptions,
    const XAEngineOption *pEngineOptions,
    XAuint32              numInterfaces,
    const XAInterfaceID  *pInterfaceIds,
    const XAboolean      *pInterfaceRequired
)
{
#ifndef WIN32
	signal(SIGSEGV, handler);
#endif
	int fUseMosaic =  ini_getl(DISP_SECTION, "use_mosaic", 0, gIniFile); 
	int fEnableDisplay =  ini_getl(DISP_SECTION, "enable_display", 1, gIniFile); 
	int nDispId = ini_getl(DISP_SECTION, "display_id", 1, gIniFile); 
	int fEncode = ini_getl(DISP_SECTION, "encode", 0, gIniFile); 
	int nDefaultSwMosaicInputBuffers = 10;
	int nSwMosaicOutputBuffers = 6;
	int nDisplayInputBuffers = 6;
	XAEngineOption *pOption;
	WINDOW_PARAM_T *wndList = NULL;
	ACHAN_PARAM_T  *pachanList = NULL;
	int nNumWindows = 0;
	int nNumAChannels = 0;


	int nADestType = OUT_ACHAIN_MIX_ENC;
	int fAEncode = 1;
	int fEnableSpkr = 0;
	int nAEncBitrate = 64000;

	int i;
	OMX_AL_SESSION_T *pSession = NULL;
	DBG_ASSERT(pEngine != NULL)
	DBG_ASSERT(numOptions == 0)
	DBG_ASSERT(pEngineOptions == NULL)
	DBG_ASSERT(numInterfaces == 0)
	DBG_ASSERT(pInterfaceIds == NULL)
	DBG_ASSERT(pInterfaceRequired == NULL)

	gDbgLevel = ini_getl(DEBUG_SECTION, "A8_DEBUG_LEVEL", 2, gIniFile);



	gMixerOutWidth  =  ini_getl(VIDMIXER_SECTION, "width",   DEF_VIDMIXEROUT_WIDTH, gIniFile);
    gMixerOutHeight =  ini_getl(VIDMIXER_SECTION, "height",  DEF_VIDMIXEROUT_HEIGHT, gIniFile);

    gDispWidth      =  ini_getl(DISP_SECTION, "width",   DEF_DISPLAY_WIDTH, gIniFile);
    gDispHeight     =  ini_getl(DISP_SECTION, "height",  DEF_DISPLAY_HEIGHT, gIniFile);
    gEncWidth       =  ini_getl(ENCODE_SECTION, "width",   DEF_DISPLAY_WIDTH, gIniFile);
    gEncHeight      =  ini_getl(ENCODE_SECTION, "height",  DEF_DISPLAY_HEIGHT, gIniFile);
    gEncFramerate   =  ini_getl(ENCODE_SECTION, "framerate",  DEF_ENC_FRAMERATE, gIniFile);
	gEncBitrate     =  ini_getl(ENCODE_SECTION, "bitrate",  DEF_ENC_BITRATE, gIniFile);
	DBG_LOG(DBGLVL_SETUP, ("Option numOptions=%d listp=%p",numOptions,pEngineOptions));

	pOption = pEngineOptions;
	for (i=0; i < numOptions; i++) {
		DBG_LOG(DBGLVL_SETUP, ("Option Feature=%d data=%d",pOption->feature, pEngineOptions));
		switch(pOption->feature){
			case XAEXT_ENGINEOPTION_SETLAYOUT:
				wndList = layoutGetUserDefined((DISP_WINDOW_LIST *)pOption->data, &nNumWindows);
				break;
			case XAEXT_ENGINEOPTION_SET_AUD_LAYOUT:
				pachanList = alayoutGetUserDefined((AUD_CHAN_LIST *)pOption->data, &nNumAChannels);
				break;
		}
		pOption++;
	}

	DBG_PRINT("OMXAL Library Version:%08x Compiled on:%s %s\n", ONYX_OMX_VERSION, __DATE__, __TIME__);
	pSession = CreateEngineInternal();

	{

		DBG_LOG(DBGLVL_SETUP, ("LayoutId=%d nNumWindows=%d",gLayoutId, nNumWindows));
#ifndef WIN32
		gpVmix = vmixInit(
			fUseMosaic ? DEST_TYPE_SWMOSAIC : DEST_TYPE_DISP, 
			fEncode,
			fEnableDisplay,
			nNumWindows, wndList,
			gMixerOutWidth, gMixerOutHeight,
			gDispWidth, gDispHeight, gDispWidth * 2,
			gEncWidth, gEncHeight,
			gEncFramerate, gEncBitrate,
			15, 0x6131,
			nSwMosaicOutputBuffers,
			nDisplayInputBuffers,
			nDispId);
#endif
		if(pachanList) {		
			gpAmix = amixInit( nADestType, fAEncode,
					fEnableSpkr, nNumAChannels, 48000, 2, 1024, pachanList,	nAEncBitrate);
		}
		if((wndList))
			free(wndList);
		if(pachanList)
			free(pachanList);
	}

	*pEngine = &pSession->pEngineObjItf; 

	return XA_RESULT_SUCCESS;
}
