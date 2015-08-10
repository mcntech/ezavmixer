# Makefile for decode app

MODULE_NAME = codecdm81xx

ifeq ($(PLATFORM),ti816x-evm)
PLATDIR = dm816x
else
PLATDIR = dm814x
endif

OMXALDIR = OpenMAXAL
G711DIR = codecs/g711
RESAMPLEDIR = codecs/resample

SRCDIR = src   src/$(PLATDIR)  ../common src/$(OMXALDIR) src/$(G711DIR) src/$(RESAMPLEDIR)
INCDIR = . src src/$(PLATDIR)  ../common src/$(OMXALDIR) src/$(G711DIR) src/$(RESAMPLEDIR) $(lindevkit_PATH)/include

# List all the external components/interfaces, whose interface header files 
#  need to be included for this component
INCLUDE_EXERNAL_INTERFACES = bios xdc omx timmosal ipc fc xdais osal syslink \
                             ce h264dec mp3dec aaclcdec aaclcenc uia slog kernel

# List of all external libraries that need to be linked
EXTLIBS_m3video =
EXTLIBS_m3vpss =
EXTLIBS_a8host = $(omx_LIBPATH)/../lib/omxcore.av5T \
                 $(omx_LIBPATH)/../lib/memcfg.av5T                     \
                 $(omx_LIBPATH)/../lib/domx.av5T           \
                 $(omx_LIBPATH)/../lib/domx_delegates_shmem.av5T       \
                 $(omx_LIBPATH)/../lib/timmosal.av5T \
                 $(omx_LIBPATH)/../lib/omxcfg.av5T         \
                 $(osal_PATH)/packages/linuxdist/build/lib/osal.a \
                 $(osal_PATH)/packages/linuxdist/cstubs/lib/cstubs.a \
                 $(fc_PATH)/packages/ti/sdo/rcm/lib/debug/rcm_syslink.av5T \
                 $(fc_PATH)/packages/ti/sdo/fc/memutils/lib/release/memutils.av5T \
                 $(osal_PATH)/packages/ti/sdo/xdcruntime/linux/lib/debug/osal_linux_470.av5T \
                 $(fc_PATH)/packages/ti/sdo/fc/global/lib/debug/fcsettings.av5T \
                 $(syslink_PATH)/packages/ti/syslink/lib/syslink.a_debug \
                 $(linuxutils_PATH)/packages/ti/sdo/linuxutils/cmem/lib/cmem.a470MV \
                 $(uia_PATH)/packages/ti/uia/linux/lib/servicemgr.a \
                 $(lindevkit_PATH)/lib/libasound.so
#                 $(omx_LIBPATH)/../lib/ldrmemcfg.av5T
#                 $(slog_PATH)/lib/slog.a \
#                 $(slog_PATH)/lib/slog_modtbl.a

# List all the components required by the application
COMP_LIST_m3video = omxbase vdec server scheduler domx omxcore 
COMP_LIST_m3vpss = omxbase vfcc vfpc vfdc ctrl domx omxcore hdvpss
COMP_LIST_c6xdsp = memcfg omxbase domx omxcore adec
COMP_LIST_a8host = ldrmemseg


# Common source files and CFLAGS across all platforms and cores
SRCS_COMMON =
CFLAGS_LOCAL_COMMON = -DGlobal_appTerminate=0 -D_VIDEO_M3_DYNAMIC_CONFIG 



SRCS_a8host = dec_main.c  dec_platform_utils.c \
              vdec_ilclient.c  vdec_ilclient_utils.c vdec_es_parser.c dec_xport.c dec_filesrc.c dec_clock.c vdec_fakedec.c \
			  aac_dec_chain.c adec_ilclient_utils.c \
			  g711_dec_chain.c \
			  aenc_ilclient.c  aenc_ilclient_utils.c \
			  msgq.c semp.c  OpenMAXAL_IID.c omxal.c udprx.c strmconn.c strmconn_ipc.c minini.c dbglog.c \
			  vcap_ilclient.c cap_main.c acap_ilclient.c ilclient_common.c aud_src_alsa.c oal.c \
			  video_mixer.c audio_mixer.c venc_omx_chain.c \
			  g711_interface.c g711.c resample_48khz.c resample_by_2_internal.c

CFLAGS_LOCAL_a8host = -DGlobal_GrpxDssMsgHeapId=6 -DGlobal_TilerHeapId=7

ifeq ($(CORE),a8host)
  CFLAGS_LOCAL_a8host += -D_LOCAL_CORE_a8host_
endif

 #CFLAGS_LOCAL_a8host += -DEXCLUDE_OMX
 #CFLAGS_LOCAL_a8host += -DEN_OMX_AL

# Include common make files
include $(ROOTDIR)/makerules/common.mk


# Nothing beyond this point
