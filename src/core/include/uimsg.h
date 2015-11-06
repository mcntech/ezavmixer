#ifndef __UI_MSG_H__
#define __UI_MSG_H__


#define ONYX_CMD_PORT          56323

#ifdef WIN32
#define ONYX_PUBLISH_FILE_NAME "C:/Projects/onyx/onyx_em/conf/onyx_publish.conf"
#else
#define ONYX_PUBLISH_FILE_NAME "/etc/onyx_publish.conf"
#endif
#ifdef WIN32
#define ONYX_CORE_FILE_NAME "C:/Projects/onyx/onyx_em/conf/onyx_core.conf"
#else
#define ONYX_CORE_FILE_NAME "/etc/onyx_core.conf"
#endif
#ifdef WIN32
#define ONYX_USER_FILE_NAME "C:/Projects/onyx/onyx_em/conf/onyx_inputs.conf"
#else
#define ONYX_USER_FILE_NAME "/etc/onyx_inputs.conf"
#endif
#ifdef WIN32
#define ONYX_MPD_FILE_NAME "C:/Projects/onyx/onyx_em/conf/onyx_mpd.conf"
#else
#define ONYX_MPD_FILE_NAME "/etc/onyx_mpd.conf"
#endif

#define SECTION_GLOBAL                 "global"
#define KEY_MW_AVMIXERS                "avmixers"

#define SECTION_AVMIXER                "avmixer"

#define AVMIXER_PREFIX                 "avmixer"
#define AVMIXER_INPUT_COUNT            "inputs"
#define AVMIXER_INPUT_PREFIX           "input"

#define KEY_MW_SWITCHES                "switches"
#define SWITCH_PREFIX                  "switch"
#define SWITCH_INPUT_COUNT             "inputs"
#define SWITCH_INPUT_PREFIX            "input"
#define SECTION_INPUT_PREFIX           "input"
#define SECTION_MPD_SERVER             "mpdserver"
#define SECTION_MPD_PUBLISH            "mpdpublish"
#define SECTION_HLS_SERVER             "hlsserver"
#define SECTION_HLS_PUBLISH            "hlspublish"
#define SECTION_RTMP_PUBLISH           "rtmppublish"

#define SECTION_RTSP_COMMON            "rtspcommon"
#define SECTION_RTSP_SERVER            "rtspserver"
#define SECTION_RTSP_PUBLISH           "rtsppublish"

#define SECTION_SWITCH_INPUT_ID_PREFIX "input"
#define SECTION_SWICTH_INPUT_PREFIX    "switch_input"

#define KEY_RTMP_PUBLISH_PRIMARY_ENABLE "primary_enable"
#define KEY_RTMP_PUBLISH_PRIMARY_HOST   "primary_host"
#define KEY_RTMP_PUBLISH_PRIMARY_PORT   "primary_rtmp_port"
#define KEY_RTMP_PUBLISH_PRIMARY_APP    "primary_application"
#define KEY_RTMP_PUBLISH_PRIMARY_STRM   "primary_stream"
#define KEY_RTMP_PUBLISH_PRIMARY_USER   "primary_user"
#define KEY_RTMP_PUBLISH_PRIMARY_PASSWD "primary_password"

#define KEY_RTMP_PUBLISH_SECONDARY_ENABLE "secondary_enable"
#define KEY_RTMP_PUBLISH_SECONDARY_HOST   "secondary_host"
#define KEY_RTMP_PUBLISH_SECONDARY_PORT   "secondary_rtmp_port"
#define KEY_RTMP_PUBLISH_SECONDARY_APP    "secondary_application"
#define KEY_RTMP_PUBLISH_SECONDARY_STRM   "secondary_stream"
#define KEY_RTMP_PUBLISH_SECONDARY_USER   "secondary_user"
#define KEY_RTMP_PUBLISH_SECONDARY_PASSWD "secondary_password"

#define KEY_RTSP_PUBLISH_HOST           "host"
#define KEY_RTSP_PUBLISH_RTSP_PORT      "rtsp_port"
#define KEY_RTSP_PUBLISH_APPLICATION    "application"
#define KEY_RTSP_PUBLISH_USER           "user"
#define KEY_RTSP_PUBLISH_PASSWD         "password"

#define KEY_RTSP_SERVER_STREAM         "stream"
#define KEY_RTSP_SERVER_RTSP_PORT     "rtsp_port"


#define KEY_RTSP_COMMON_ENABLE_MUX     "mux"
#define KEY_RTSP_COMMON_ENABLE_VID     "vid"
#define KEY_RTSP_COMMON_ENABLE_AUD     "aud"
#define KEY_RTSP_COMMON_INPUT0         "input0"

#define KEY_MPD_PROTOCOL               "protocol"
#define KEY_MPD_FOLDER                 "folder"
#define KEY_MPD_STREAM                 "stream"
#define KEY_MPD_SEGMENT_DURATION       "segment_duration"
#define KEY_MPD_LIVE_ONLY              "live_only"
#define KEY_MPD_HTTP_SERVER            "http_server"
#define KEY_MPD_S3_BUCKET              "s3_bucket"
#define KEY_MPD_ACCESS_ID              "access_id"
#define KEY_MPD_SECURITY_KEY           "security_key"
#define KEY_MPD_SRVR_PORT              "port"
#define KEY_MPD_SRVR_ROOT              "srvr_root"
#define KEY_MPD_SRVR_INTERNAL          "internal_server"
#define KEY_MPD_INPUT_COUNT            "inputs"
#define KEY_MPD_INPUT_PREFIX            "input"

#define KEY_HLS_PROTOCOL               "protocol"
#define KEY_HLS_FOLDER                 "folder"
#define KEY_HLS_STREAM                 "stream"
#define KEY_HLS_SEGMENT_DURATION       "segment_duration"
#define KEY_HLS_LIVE_ONLY              "live_only"
#define KEY_HLS_HTTP_SERVER            "http_server"
#define KEY_HLS_S3_BUCKET              "s3_bucket"
#define KEY_HLS_ACCESS_ID              "access_id"
#define KEY_HLS_SECURITY_KEY           "security_key"
#define KEY_HLS_INPUT0                  "input0"

#define HLS_PROTOCOL_HTTP              "http://"
#define HLS_PROTOCOL_FILE              "file:///"
#define KEY_HLS_SRVR_PORT              "port"
#define KEY_HLS_SRVR_ROOT              "srvr_root"
#define KEY_HLS_SRVR_INTERNAL          "internal_server"


#define SECTION_SERVERS                "servers"
#define EN_RTSP_SERVER                 "rtspserver"
#define EN_RTSP_PUBLISH                "rtsppublish"
#define EN_HLS_SERVER                  "hlsserver"
#define EN_HLS_PUBLISH                 "hlspublish"
#define EN_MPD_SERVER                  "mpdserver"
#define EN_MPD_PUBLISH                 "mpdpublish"

#define EN_RTMP_SERVER                 "rtmpserver"
#define EN_RTMP_PUBLISH                "rtmppublish"
#define EN_RTP_SERVER                  "rtpserver"
#define EN_UDP_SERVER                  "udpserver"

#define EN_PUBLISH_SWITCH              "publishswitch"

#define ENABLE_AUDIO                   "enable_audio"

#define KEY_INPUT_TYPE                "type"
#define KEY_INPUT_FILE_LOCATION       "location"

#define KEY_INPUT_HOST                 "host"
#define KEY_INPUT_PORT                 "port"
#define KEY_INPUT_APPLICATION          "application"
#define KEY_INPUT_STREAM               "stream"
#define KEY_INPUT_STREAM_AUD_CODEC     "aud_codec"
#define KEY_INPUT_DEVICE               "device"

#define KEY_INPUT_AUD_PORT_LOCAL       "audio_port_local"
#define KEY_INPUT_AUD_PORT_PEER        "audio_port_peer"
#define KEY_INPUT_VID_PORT_LOCAL       "video_port_local"
#define KEY_INPUT_VID_PORT_PEER        "video_port_peer"

#define INPUT_STREAM_TYPE_FILE         "file"
#define INPUT_STREAM_TYPE_HLS          "hls"
#define INPUT_STREAM_TYPE_RTSP         "rtsp"
#define INPUT_STREAM_TYPE_CAPTURE      "capture"
#define INPUT_STREAM_TYPE_AVMIXURE     "avmixer"
#define INPUT_STREAM_TYPE_STRMCONN     "strmconn"
#define INPUT_STREAM_TYPE_INPROC        "inproc"
#define INPUT_STREAM_TYPE_STRMCONN_IPC "strmconn_ipc"
#define INPUT_STREAM_TYPE_STRMCONN_ZMQ "strmconn_zmq"
#define INPUT_STREAM_TYPE_AVMIXER      "avmixer"

#define AUDIO_CODEC_G711U              "g711u"
#define AUDIO_CODEC_NONE               "none"

#define GLOBAL_SECTION                 "global"
#define KEY_ENABLE_VIDEO_MIXER         "video_mixer"
#define KEY_LIBRARY_NAME               "library_name"

#define UI_MSG_HELLOW               0
#define UI_MSG_EXIT					1
#define UI_MSG_SELECT_LAYOUT		2
#define UI_MSG_SELECT_SWITCH_SRC	3

#define UI_MSG_SET_MOD_DBG_LVL      9

#define UI_MSG_HLS_PUBLISH_STATS	64
#define UI_MSG_SWITCHES_STATS	    65

#define UI_STATUS_OK				1
#define UI_STATUS_ERR				0x8000

typedef struct _ONYX_MSG_HDR_T
{
    unsigned long ulCmdId;
    unsigned long ulSize;
} ONYX_MSG_HDR_T;

typedef struct _uiLayoutSelection_t
{
	int		nLayoutId;
} uiSelectLayout_t;

typedef struct _SwitchSrcSelect_t
{
	int		nSrcId;
} SwitchSrcSelect_t;

typedef struct _SetModDbgLvl_t
{
	int		nModuleId;
	int		nDbgLvl;
} SetModDbgLvl_t;

#define HLS_PUBLISH_STATE_UNKNOWN		0
#define HLS_PUBLISH_STOPPED				1
#define HLS_PUBLISH_RUNNING				2

#define HLS_PUBLISH_ERROR_CONNECT_FAIL	0x80000001
#define HLS_PUBLISH_ERROR_XFR_FAIL		0x80000002
#define HLS_PUBLISH_ERROR_INPUT_FAIL	0x80000003

#define SWITCH_STAT_MAX_NUM             8
#define MAX_SWITCH_OUTPUTS              4
#define MAX_SWITCH_OUT_ID_SIZE          8

#define SWITCH_OUT_STATE_STOP          0
#define SWITCH_OUT_STATE_RUN           1
#define SWITCH_OUT_STATE_ERROR         2

typedef struct _hlspublisgStats_t
{
	int		nState;
	int		nStreamInTime;
	int		nStreamOutTime;
	int		nTotalSegmentTime;
	int		nLostBufferTime;
} hlspublisgStats_t;

typedef struct _swicthPortStats_t
{
	char szId[MAX_SWITCH_OUT_ID_SIZE];
	int nState;
	int nVidStrmTime;
	int nAudStrmTime;
	int nErrors;
} swicthPortStats_t;

typedef struct _switchStats_t
{
	int nNumOutputs;
	swicthPortStats_t  inputStats;
	swicthPortStats_t  outputStats[MAX_SWITCH_OUTPUTS];
} switchStats_t; 

typedef struct _switchesStats_t
{
	int            nStartTime;
	int		       nNumSwitches;
	switchStats_t  switchesStats[SWITCH_STAT_MAX_NUM];
} switchesStats_t;

typedef struct _uiExit_t
{
	int		nExitCode;
} uiExit_t;

typedef struct UI_MSG_T {
	int		    nMsgId;
	union {
		uiSelectLayout_t     Layout;
		SwitchSrcSelect_t    SwitchSrc;
		SetModDbgLvl_t       DbgLvl;
	} Msg;
} UI_MSG_T;


#endif //__UI_MSG_H__
