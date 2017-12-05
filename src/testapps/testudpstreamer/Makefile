EZAVMIXER_ROOT=../../..

PLUGIN_JDUDP=$(EZAVMIXER_ROOT)/src/plugins/jdudp
PLUGIN_OSAL=$(EZAVMIXER_ROOT)/src/plugins/osal
PLUGIN_STREAMIF=$(EZAVMIXER_ROOT)/src/plugins/streamif
PLUGIN_PARSEUTIL=$(EZAVMIXER_ROOT)/src/plugins/parseutil
PLUGIN_TSDEMUX=$(EZAVMIXER_ROOT)/src/plugins/tsdemux
PLUGIN_TSDEMUX=$(EZAVMIXER_ROOT)/src/plugins/tsdemux
LOGUTIL=$(EZAVMIXER_ROOT)/src/logutil
CORE=$(EZAVMIXER_ROOT)/src/core

INCLUDES= -I $(CORE)/include -I $(PLUGIN_JDUDP)/include -I $(PLUGIN_OSAL)/include -I $(LOGUTIL)/include -I $(PLUGIN_STREMIF)/include -I $(PLUGIN_PARSEUTIL)/include -I $(PLUGIN_TSDEMUX)/include
SOURCES= main.cpp \
         $(PLUGIN_JDUDP)/src/filesrc.cpp \
         $(PLUGIN_JDUDP)/src/xport.cpp  \
         $(PLUGIN_JDUDP)/src/strmclock.cpp \
         $(PLUGIN_JDUDP)/src/udprx.cpp \
          $(PLUGIN_JDUDP)/src/udptx.cpp \
         $(PLUGIN_OSAL)/src/JdOsal.cpp \
         $(PLUGIN_STREAMIF)/src/strmconn.c \
         $(PLUGIN_PARSEUTIL)/src/h264parser.cpp \
         $(PLUGIN_TSDEMUX)/src/TsPsi.cpp \
         $(CORE)/src/UdpClntBridge.cpp
all:
	gcc -g  -DDEMUX_DUMP_OUTPUT -DUDP_STREAMER $(SOURCES) $(INCLUDES) -lstdc++ -std=gnu++11 -lpthread -o testudpstreamer
