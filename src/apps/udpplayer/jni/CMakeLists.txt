cmake_minimum_required(VERSION 3.4.1)


set(LOCAL_PATH .)
set(JD_PLUGIN_FOLDER ${LOCAL_PATH}/../../../plugins)
set(FFMPEG_DIST_DIR /home/ramp/exp_proj/ezavmixer/src/ffmpeg_dist/android )

include_directories(
     ${LOCAL_PATH}
     ${LOCAL_PATH}/..
     ${LOCAL_PATH}/../include
     ${LOCAL_PATH}/../../../logutil/include
     ${JD_PLUGIN_FOLDER}/jdudp/include
     ${JD_PLUGIN_FOLDER}/jdnet/include
     ${JD_PLUGIN_FOLDER}/parseutil/include
     ${JD_PLUGIN_FOLDER}/tsdemux/include
     ${JD_PLUGIN_FOLDER}/tinyxml/include
     ${JD_PLUGIN_FOLDER}/osal/include
     ${JD_PLUGIN_FOLDER}/configdb/include
     ${JD_PLUGIN_FOLDER}/osal/include
     ${JD_PLUGIN_FOLDER}/streamif/include
     ${LOCAL_PATH}/../../../core/include
     ${FFMPEG_DIST_DIR}/${ANDROID_ABI}/include
     )
add_subdirectory(${LOCAL_PATH}/../../../core/jni subproject/onyxcore)
add_subdirectory(${LOCAL_PATH}/../../../plugins/jdudp/jni subproject/jdudp)
add_subdirectory(${LOCAL_PATH}/../../../plugins/streamif/jni subproject/streamif)
add_subdirectory(${LOCAL_PATH}/../../../plugins/osal/jni subproject/osal)
add_subdirectory(${LOCAL_PATH}/../../../plugins/parseutil/jni subproject/parseutil)
add_subdirectory(${LOCAL_PATH}/../../../plugins/auddec/jni subproject/auddec)

add_library(UdpPlayerApi SHARED
 	jUdpPlayerApi.cpp
	jUdpPlayerEvents.cpp 
	ServerNodeBase.cpp  
	ServerNodeUdp.cpp  
	UdpMultiPlayer.cpp) 

add_library( libavcodec STATIC IMPORTED )
set_target_properties( libavcodec PROPERTIES IMPORTED_LOCATION
                       ${FFMPEG_DIST_DIR}/${ANDROID_ABI}/lib/libavcodec.a )

target_link_libraries(
    UdpPlayerApi
    onyxcore
    jdudp
    jdosal
    streamif
    parseutil
    android
    log
    )

add_library(AudDecApi SHARED
 	jAudDecApi.cpp
    )

target_link_libraries(
    UdpPlayerApi
    libavcodec
    )