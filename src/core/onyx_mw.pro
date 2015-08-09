#QT += webkit \
#    network
#LIBS += -lgthread-2.0
QT -= gui
QT -= core
QT -= network

ONYX_LIBS = $(EZSDK)/onyx_em/onyx_libs
JDNET = $$ONYX_LIBS/jdnet
JDAWS = $$ONYX_LIBS/jdaws
TS_DEMUX = $$ONYX_LIBS/tsdemux
TS_MUX = $$ONYX_LIBS/tsmux
MP4_MUX = $$ONYX_LIBS/mp4mux
HTTPLIVE = $$ONYX_LIBS/httplive
MPEGDASH = $$ONYX_LIBS/mpegdash
MISC = $$ONYX_LIBS/misc
LIBRTMP = $$ONYX_LIBS/librtmp
TINYXML = $$ONYX_LIBS/tinyxml

#DEFINES += ENABLE_OPENSSL
#DEFINES += ENABLE_RTMP
#DEFINES += ENABLE_AWS_PUBLISH

#LIBS += -lssl -lcrypto 
LIBS += -ldl

CONFIG+=debug

CODEC_DM81XX = $(EZSDK)/component-sources/omx_05_02_00_48/examples/ti/omx/demos/codecdm81xx/src

INCLUDEPATH += $$JDNET/include $$JDAWS/include $$CODEC_DM81XX ./OpenMAXAL $$TS_DEMUX $$TS_MUX $$LIBRTMP $$MP4_MUX $$MISC $$MPEGDASH $$TINYXML $$HTTPLIVE
HEADERS = $$JDNET/include/JdRtspClntRec.h

TARGET = onyx_mw
SOURCES += $$JDNET/src/JdRtspClntRec.cpp $$JDNET/src/JdRtspSrv.cpp $$JDNET/src/JdRtpSnd.cpp $$JDNET/src/JdRtspClnt.cpp $$JDNET/src/JdAuthRfc2617.cpp $$JDNET/src/JdNetUtil.cpp \
           $$JDNET/src/JdMediaResMgr.cpp $$JDNET/src/JdHttpSrv.cpp $$JDNET/src/JdHttpClnt.cpp \
           $$JDNET/src/JdRfc2250.cpp $$JDNET/src/JdRfc3984.cpp $$JDNET/src/JdRtp.cpp $$JDNET/src/JdRfc5391.cpp $$JDNET/src/JdRfc3640.cpp $$JDNET/src/JdOsal.cpp \
		   onyx_mw_main.cpp onyx_mw_util.cpp sock_ipc.cpp minini.c OmxIf.cpp OpenMAXAL/OpenMAXAL_IID.c \
		   RtspPublishBridge.cpp HlsSrvBridge.cpp MediaSwitch.cpp \
		   StrmInBridgeBase.cpp StrmOutBridgeBase.cpp RtspClntBridge.cpp  HlsClntBridge.cpp MpdSrvBridge.cpp \
		   $$CODEC_DM81XX/strmconn.c $$CODEC_DM81XX/strmconn_ipc.c $$CODEC_DM81XX/dbglog.c \
		   $$MISC/h264parser.cpp \
		   $$TS_DEMUX/TsDemux.cpp $$TS_DEMUX/TsPsi.cpp $$TS_DEMUX/tsfilter.cpp \
  		   $$TS_MUX/SimpleTsMux.cpp \
		   $$HTTPLIVE/JdHttpLiveSgmt.cpp $$HTTPLIVE/JdHttpLiveClnt.cpp \
		   $$MPEGDASH/MpegDashSgmt.cpp $$MPEGDASH/Mpd.cpp \
		   $$MP4_MUX/Mp4Mux.cpp \
		   $$TINYXML/tinyxml.cpp $$TINYXML/tinyxmlparser.cpp $$TINYXML/tinyxmlerror.cpp $$TINYXML/tinystr.cpp \

#		   RtmpPublishBridge.cpp $$LIBRTMP/amf.c $$LIBRTMP/hashswf.c $$LIBRTMP/log.c $$LIBRTMP/parseurl.c  $$LIBRTMP/rtmp.c \
#           $$JDAWS/src/JdAwsConfig.cpp $$JDAWS/src/JdAwsContext.cpp $$JDAWS/src/JdAwsS3.cpp $$JDAWS/src/JdAwsS3UpnpHttpConnection.cpp \

# install
target.path = .
INSTALLS += target
