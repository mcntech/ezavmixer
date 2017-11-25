#ifndef __UDP_CALLBACK_H__
#define __UDP_CALLBACK_H__

typedef enum UDP_SERVER_STATE
{
	UDP_SERVER_UNINIT,
	UDP_SERVER_SETUP,
	UDP_SERVER_PLAY,
	UDP_SERVER_ERROR,
};

typedef struct _UDP_SERVER_DESCRIPTION
{
	int nAudCodecType;
	int nVidCodecType;
} UDP_SERVER_DESCRIPTION;

typedef struct _UDP_SERVER_STATS
{
	int nAudBitrate;
	int nVidBitrate;
	int nAudPktLoss;
	int nVidPktLoss;
	int nClockJitter;
	int nClockJitterMax;
	int nClockJitterMin;
} UDP_SERVER_STATS;

typedef struct _UDP_SERVER_STATUS
{
	int nState;
} UDP_SERVER_STATUS;

class CUdpServerCallback
{
public:
	virtual void UpdateStats(const char *url, UDP_SERVER_STATS *) = 0;
	virtual void NotifyPsiChange(const char *url, const char *pPsiData) = 0;
};

#endif //__RTSP_CALLBACK_H__
