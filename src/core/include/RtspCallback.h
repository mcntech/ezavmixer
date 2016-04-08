#ifndef __RTSP_CALLBACK_H__
#define __RTSP_CALLBACK_H__

typedef enum RTSP_SERVER_STATE
{
	RTSP_SERVER_UNINIT,
	RTSP_SERVER_SETUP,
	RTSP_SERVER_PLAY,
	RTSP_SERVER_ERROR,
};

typedef struct _RTSP_SERVER_DESCRIPTION
{
	int nAudCodecType;
	int nVidCodecType;
} RTSP_SERVER_DESCRIPTION;

typedef struct _RTSP_SERVER_STATS
{
	int nAudBitrate;
	int nVidBitrate;
	int nAudPktLoss;
	int nVidPktLoss;
	int nClockJitter;
	int nClockJitterMax;
	int nClockJitterMin;
} RTSP_SERVER_STATS;

typedef struct _RTSP_SERVER_STATUS
{
	int nState;
} RTSP_SERVER_STATUS;

class CRtspServerCallback
{
public:
	virtual void NotifyStateChange(const char *url, int nState) = 0;
	virtual void UpdateStats(const char *url, RTSP_SERVER_STATS *) = 0;
};

#endif //__RTSP_CALLBACK_H__
