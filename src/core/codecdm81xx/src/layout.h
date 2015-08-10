#ifndef __LAYOUT_H__
#define __LAYOUT_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _WINDOW_PARAM_T
{
	int    nStrmSrc;
	int    nStartX;
	int    nStartY;
	int    nWidth;
	int    nHeight;
	int    nStride;
	int    nBufferCount;
} WINDOW_PARAM_T;

typedef struct _ACHAN_PARAM_T
{
	int    nStrmSrc;
	int    nVolPercent;
	int    nSampleRate;
	int    nFormat;
	int    nChannels;
	int    nBufferCount;
} ACHAN_PARAM_T;

#ifdef __cplusplus
}
#endif

#endif //__LAYOUT_H__
