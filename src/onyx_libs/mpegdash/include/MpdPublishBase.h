#ifndef __MPD_PUBLISH_BASE_H__
#define __MPD_PUBLISH_BASE_H__


typedef struct _MPD_PUBLISH_STATS
{
	int               nState;
	int               nSegmentTime;
	int               nLostBufferTime;
	int               nStreamInTime;
	int               nStreamOutTime;
} MPD_PUBLISH_STATS;

#endif // __MPD_PUBLISH_BASE_H__
