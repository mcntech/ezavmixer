#ifndef __ILCLIENT_COMMON_H__
#define __ILCLIENT_COMMON_H__

#include <stdint.h>
#include <stdint.h>
#include "vdec_es_parser.h"
#include "semp.h"
#include "OMX_Types.h"
#include "OMX_Core.h"
#include "OMX_Component.h"
#include "dec_xport.h"
#include "dec_filesrc.h"
#include "dec_omx_chains_common.h"
#include "strmcomp.h"
#include "dec_clock.h"
#include "layout.h"

#ifdef __cplusplus              /* required for headers that might */
extern "C"
{                               /* be compiled by a C++ compiler */
#endif

/*--------------------------- defines ----------------------------------------*/
/* Align address "a" at "b" boundary */
#define UTIL_ALIGN(a,b)  ((((uint32_t)(a)) + (b)-1) & (~((uint32_t)((b)-1))))

#define IL_CLIENT_MAX_NUM_IN_BUFS         32
#define IL_CLIENT_MAX_NUM_OUT_BUFS        32

#define H264_PADX    (32)
#define H264_PADY    (24)


#define OMX_INIT_PARAM(param)                                                  \
        {                                                                      \
          memset ((param), 0, sizeof (*(param)));                              \
          (param)->nSize = sizeof (*(param));                                  \
          (param)->nVersion.s.nVersionMajor = 1;                               \
          (param)->nVersion.s.nVersionMinor = 1;                               \
        }

typedef struct IL_CLIENT_COMP_PRIVATE_T    IL_CLIENT_COMP_PRIVATE;
typedef struct IL_CLIENT_GFX_PRIVATE_T     IL_CLIENT_GFX_PRIVATE;
typedef struct IL_CLIENT_OUTPORT_PARAMS_T  IL_CLIENT_OUTPORT_PARAMS;
typedef struct IL_CLIENT_INPORT_PARAMS_T   IL_CLIENT_INPORT_PARAMS;

typedef struct IL_CLIENT_SNT_CONNECT_T 
{
	OMX_U32 remotePort;
	OMX_S32 remotePipe[2];      /* for making ETB / FTB calls to connected comp 
								*/
	IL_CLIENT_COMP_PRIVATE *remoteClient;       /* For allocate / use buffer */
} IL_CLIENT_SNT_CONNECT;


typedef struct IL_CLIENT_OUTPORT_PARAMS_T
{
	IL_CLIENT_SNT_CONNECT connInfo;
	OMX_S32 opBufPipe[2];       /* output pipe */
	OMX_U32 nBufferCountActual;
	OMX_U32 nBufferSize;

	OMX_BOOL fInterlace;
	OMX_S32 nFieldCount;
	OMX_U8  *pFirstFieldBuf;
	OMX_S32 nFramesToM3;
	OMX_S32 nFramesFromM3;

	OMX_BUFFERHEADERTYPE *pOutBuff[IL_CLIENT_MAX_NUM_OUT_BUFS];
	int                  fUsePeerBuffer;    // Default allocate
} IL_CLIENT_OUTPORT_PARAMS_T;

typedef struct IL_CLIENT_INPORT_PARAMS_T
{
	IL_CLIENT_SNT_CONNECT connInfo;
	OMX_S32 ipBufPipe[2];       /* input pipe */
	OMX_U32 nBufferCountActual;
	OMX_U32 nBufferSize;
	
	OMX_BOOL fInterlace;
	OMX_S32 nFieldCount;
	OMX_S32 nSecondFieldOffset;
	OMX_U8  *pFirstFieldBuf;
	OMX_S32 nFramesToM3;
	OMX_S32 nFramesFromM3;

	OMX_BUFFERHEADERTYPE *pInBuff[IL_CLIENT_MAX_NUM_IN_BUFS];
	// The following field used for caching parameters of connected port
	int                      fUsePeerBuffer;      // Default use buffer
} IL_CLIENT_INPORT_PARAMS_T;


#define MAX_COMP_NAME  128
typedef struct IL_CLIENT_COMP_PRIVATE_T
{
	IL_CLIENT_INPORT_PARAMS *inPortParams;
	IL_CLIENT_OUTPORT_PARAMS *outPortParams;    /* Common o/p port params */
	OMX_U32 numInport;
	OMX_U32 numOutport;
	OMX_U32 startOutportIndex;
	OMX_S32 localPipe[2];						// Used as tunnel communication for A-8 components and for IL assisted
											// communication for M3 componets

	OMX_HANDLETYPE handle;
	OMX_U32 numFrames;

	pthread_t inDataStrmThrdId;
	pthread_t outDataStrmThrdId;
	pthread_t connDataStrmThrdId;
	semp_t    *done_sem;
	semp_t *eos;
	semp_t *port_sem;
	pthread_attr_t ThreadAttr;
	pthread_mutex_t ebd_mutex;
	pthread_mutex_t fbd_mutex;

	int     nSessionId;
	int     nUiCmd;
	int     frameCounter;
	int     frameCounterDrop;
	int     prevframeCounter;
	int     nDropFraction;
	CLOCK_T statPrevTime;
	int     fSync;
	void    *pClk;
	CLOCK_T crnt_pts;
	int                nJitterLatency;
	int                nSyncMaxLateness;
	CLOCK_T            nSyncMaxWaitRunning;
	CLOCK_T            nSyncMaxWaitStartup;


	char    comp_name[MAX_COMP_NAME];
	void    *pParent;
  /* 
 IL_CLIENT_FILEIO_PARAMS sFileIOBuff[IL_CLIENT_MAX_NUM_FILE_OUT_BUFS]; *//* file Input/Output buffers */
} IL_CLIENT_COMP_PRIVATE_T;

typedef struct _IL_CLIENT_COMP_HOST_T
{
	OMX_HANDLETYPE handle;
	char    comp_name[MAX_COMP_NAME];
	void    *pParent;
} IL_CLIENT_COMP_HOST_T;

typedef struct IL_CLIENT_GFX_PRIVATE_T
{
	pthread_t ThrdId;
	pthread_attr_t ThreadAttr;
	uint32_t terminateGfx;
	uint32_t terminateDone;
	uint32_t gfxId;
} IL_CLIENT_GFX_PRIVATE_T;

typedef enum
{
	IL_CLIENT_PIPE_CMD_ETB,
	IL_CLIENT_PIPE_CMD_FTB,
	IL_CLIENT_PIPE_CMD_EBD,
	IL_CLIENT_PIPE_CMD_FBD,
	IL_CLIENT_PIPE_CMD_EXIT,
	IL_CommandMax = 0X7FFFFFFF
} IL_CLIENT_PIPE_CMD_TYPE;

typedef enum
{
	ILCLIENT_INPUT_PORT,
	ILCLIENT_OUTPUT_PORT,
	ILCLIENT_PortMax = 0X7FFFFFFF
} ILCLIENT_PORT_TYPE;

typedef struct IL_CLIENT_PIPE_MSG_T
{
	IL_CLIENT_PIPE_CMD_TYPE cmd;
	OMX_BUFFERHEADERTYPE *pbufHeader;   /* used for EBD/FBD */
	OMX_BUFFERHEADERTYPE bufHeader;     /* used for ETB/FTB */
} IL_CLIENT_PIPE_MSG;


OMX_STRING IL_ClientErrorToStr (OMX_ERRORTYPE error);

int scalerInit(
	IL_CLIENT_COMP_PRIVATE_T *scILComp,
	int                       deinterlace,
	int                       useDeiScalerAlways,
	int  nInputWidth,
	int  nInputHeight,
	int  nInputStride,
	int  nAlinedInputBufferSize,
	int  nInputBuffers,
	int  nInputFormat, //OMX_COLOR_FormatYUV420SemiPlanar

	int  nOutput1Width,
	int  nOutput1Height,
	int  nOutput1Stride,
	int  nAlinedouptu1BufferSize,
	int  nOutput1Buffers,
	int  nOutput1Format, //OMX_COLOR_FormatYUV420SemiPlanar

	int  nOutput2Width,
	int  nOutput2Height,
	int  nOutput2Stride,
	int  nAlinedouptu2BufferSize,
	int  nOutput2Buffers,
	int  nOutput2Format, //OMX_COLOR_FormatYUV420SemiPlanar

	OMX_CALLBACKTYPE          *pCb);

int scalerAllocateResource(
	IL_CLIENT_COMP_PRIVATE_T *scILComp,
	int deinterlace
	);

OMX_ERRORTYPE ConfigureDisplay(
	int            displayId,
	OMX_HANDLETYPE pDisHandle,
	OMX_HANDLETYPE pctrlHandle,
	int            nWidth,
	int            nHeight,
	int            nBufferCOunt
	);

typedef void *(*ILC_StartFcnPtr) (void *);
void IL_ClientSinkTask (void *threadsArg);
void IL_DisplayTask (void *threadsArg);
OMX_ERRORTYPE IL_ClientProcessPipeCmdEBD (IL_CLIENT_COMP_PRIVATE *thisComp,
                                          IL_CLIENT_PIPE_MSG *pipeMsg);
OMX_ERRORTYPE IL_ClientProcessPipeCmdFBD (IL_CLIENT_COMP_PRIVATE *thisComp,
                                          IL_CLIENT_PIPE_MSG *pipeMsg);
OMX_ERRORTYPE
  IL_ClientUseInitialOutputResources (IL_CLIENT_COMP_PRIVATE *thisComp);
void IL_ClientConnInConnOutTask (void *threadsArg);

OMX_ERRORTYPE IL_ClientUtilGetSelfBufHeader (IL_CLIENT_COMP_PRIVATE *thisComp,
                                             OMX_U8 *pBuffer,
                                             ILCLIENT_PORT_TYPE type,
                                             OMX_U32 portIndex,
                                             OMX_BUFFERHEADERTYPE **pBufferOut);
OMX_ERRORTYPE IL_ClientProcessPipeCmdETB (
	IL_CLIENT_COMP_PRIVATE *thisComp,
	IL_CLIENT_PIPE_MSG *pipeMsg);

OMX_ERRORTYPE IL_ClientConnectComponents (IL_CLIENT_COMP_PRIVATE
                                            *handleCompPrivA,
                                          unsigned int compAPortOut,
                                          IL_CLIENT_COMP_PRIVATE
                                            *handleCompPrivB,
                                          unsigned int compBPortIn);

IL_CLIENT_INPORT_PARAMS *compGetConnectedCompInPortParams (
	IL_CLIENT_COMP_PRIVATE	*pComp,
	int             nOutPort);


IL_CLIENT_OUTPORT_PARAMS *compGetConnectedCompOutPortParams(
	IL_CLIENT_COMP_PRIVATE   *pComp,
	int             nInPort);

OMX_ERRORTYPE IL_ClientProcessPipeCmdFTB (IL_CLIENT_COMP_PRIVATE *thisComp,
                                          IL_CLIENT_PIPE_MSG *pipeMsg);

OMX_ERRORTYPE IL_ClientCbFillBufferDone (OMX_HANDLETYPE hComponent,
                                         OMX_PTR ptrAppData,
                                         OMX_BUFFERHEADERTYPE *pBuffer);

OMX_ERRORTYPE IL_ClientCbEmptyBufferDone (OMX_HANDLETYPE hComponent,
                                          OMX_PTR ptrAppData,
                                          OMX_BUFFERHEADERTYPE *pBuffer);


OMX_ERRORTYPE IL_ClientInitSwMosaic( 
	IL_CLIENT_COMP_PRIVATE *vswmosaicILComp,
	int                    nNumWindows,
	WINDOW_PARAM_T         *pWindInParam,
	WINDOW_PARAM_T         *pWindOutParam,
	OMX_CALLBACKTYPE       *pvswmosaicCb
	);

OMX_ERRORTYPE IL_ClientCbEventHandler (
	OMX_HANDLETYPE     hComponent,
	OMX_PTR            ptrCompCtx,
	OMX_EVENTTYPE      eEvent,
	OMX_U32            nData1, 
	OMX_U32            nData2,
	OMX_PTR            pEventData);

IL_CLIENT_COMP_PRIVATE *CreateILCompWrapper(
	int numInPort, 
	int numOutPort, 
	int nOutPortIndex, 
	int nInPortBuffCount, 
	int nInPortBuffSize,
	int nOutPortBuffCount, 
	int nOutPortBuffSize);

IL_CLIENT_COMP_PRIVATE *CreateILMultiInputCompWrapper(
	int numInPort, 
	int numOutPort, 
	int nOutPortIndex, 
	WINDOW_PARAM_T *listInputWindows,
	int nOutPortBuffCount, 
	int nOutPortBuffSize);

void DeleteILCompWrapper(IL_CLIENT_COMP_PRIVATE *pComp);

int displayInit(
	int                       displayId,
	void                      *pParent,
	IL_CLIENT_COMP_PRIVATE    *pComp,
	OMX_CALLBACKTYPE          *pCb,
	int                       nWidth,
	int                       nHeight,
	int                       nBufferCount,
    OMX_HANDLETYPE            *pCtrlHandle
	);

int encoderInit(
	void                      *pParent,
	IL_CLIENT_COMP_PRIVATE    *pComp,
	OMX_CALLBACKTYPE          *pCb,
	int  nWidth,
	int  nHeight,
	int  nInputBuffers,
	int  nOutBuffers,
	int  nEncodedFrms,
	int  nFrameRate,
	int  nBitRate
	);

int decoderInit(
		void                      *pParent,
		IL_CLIENT_COMP_PRIVATE    *pComp,
		OMX_CALLBACKTYPE          *pCb
	);

int compSetOutPortParam(IL_CLIENT_COMP_PRIVATE *pComp, int nOutPortOffset, int nBuffers, int nBufferSize);
int compSetStateExec(IL_CLIENT_COMP_PRIVATE_T *pComp, OMX_HANDLETYPE ctrlHandle);
int compStartStream(IL_CLIENT_COMP_PRIVATE *pComp, ILC_StartFcnPtr pfnTask);
int compStopStream(IL_CLIENT_COMP_PRIVATE *pComp);
int semp_timedpend (semp_t *semp, int nTimeOutMsec);
int compSetInportAllocationType(IL_CLIENT_COMP_PRIVATE *pComp, int nInputPort, int fUsePeerBuffer);
int compSetOutPortAllocationType(IL_CLIENT_COMP_PRIVATE *pComp, int nOutputPort, int fUsePeerBuffer);
int compInitResource(IL_CLIENT_COMP_PRIVATE *pComp, OMX_HANDLETYPE ctrlHandle);
int compDeinitResource(IL_CLIENT_COMP_PRIVATE *pComp, OMX_HANDLETYPE ctrlHandle);

void close_pipe(int *pipedscrpt);

#ifdef __cplusplus              /* matches __cplusplus construct above */
}
#endif

#endif
