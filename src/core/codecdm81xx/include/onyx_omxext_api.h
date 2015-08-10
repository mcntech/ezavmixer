#ifndef __ONYX_OMXALEXT_API__
#define __ONYX_OMXALEXT_API__

#define VERSION            "version"
#define SESSION_ID         "session_id"
#define VID_INPUT_PARAM    "vid_input_param"
#define AUD_INPUT_PARAM    "aud_input_param"
#define LATENCY            "latency"
#define DEINTERLACE         "deinterlace"
#define DEMUX_SELECT_PROG   "demux_select_prog"
#define FRAMERATE           "framerate"
#define A8_DEBUG_LEVEL      "a8_debug_level"

// XAEXT_ENGINEOPTION_xx follows the XA_ENGINEOPTION_STEPVERSION
//#define XA_ENGINEOPTION_STEPVERSION     ((XAuint32) 0x00000005)
#define XAEXT_ENGINEOPTION_LAYOUTID       8
#define XAEXT_ENGINEOPTION_SETLAYOUT      9
#define XAEXT_ENGINEOPTION_SET_AUD_LAYOUT 10

typedef struct _DISP_WINDOW_T
{
	int    nStrmSrc;
	int    nStartX;
	int    nStartY;
	int    nWidth;
	int    nHeight;
} DISP_WINDOW;

typedef struct _DISP_WINDOW_LIST_T
{
	int          nNumWnd;
	DISP_WINDOW *pWndList;
} DISP_WINDOW_LIST;

typedef struct _AUD_CHAN_T
{
	int    nStrmSrc;
	int    nFormat;
	int    nBitrate;
	int    nSampleRate;
	int    nVolPercent;
} AUD_CHAN_T;

typedef struct _AUD_CHAN_LIST
{
	int          nNumChan;
	AUD_CHAN_T *pAChanList;
} AUD_CHAN_LIST;

typedef struct _RECORD_SINK_T
{
	void       *pVidSink;
	void       *pAudSink;
} RECORD_SINK_T;

#define VID_ENC_TX "/dev/vid_enc_tx"
#define AUD_ENC_TX "/dev/aud_enc_tx"

#define VID_ENC_RX "/dev/vid_enc_rx"
#define AUD_ENC_RX "/dev/aud_enc_rx"

#endif