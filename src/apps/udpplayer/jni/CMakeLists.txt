cmake_minimum_required(VERSION 3.4.1)

cmake_policy(SET CMP0015 NEW)
set(JD_PLUGIN_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../../../plugins)
set(FFMPEG_DIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../ffmpeg_dist/android )

include_directories(
     ${CMAKE_CURRENT_SOURCE_DIR}
     ${CMAKE_CURRENT_SOURCE_DIR}/..
     ${CMAKE_CURRENT_SOURCE_DIR}/../include
     ${CMAKE_CURRENT_SOURCE_DIR}/../../../logutil/include
     ${JD_PLUGIN_FOLDER}/jdudp/include
     ${JD_PLUGIN_FOLDER}/jdnet/include
     ${JD_PLUGIN_FOLDER}/parseutil/include
     ${JD_PLUGIN_FOLDER}/tsdemux/include
     ${JD_PLUGIN_FOLDER}/tinyxml/include
     ${JD_PLUGIN_FOLDER}/osal/include
     ${JD_PLUGIN_FOLDER}/configdb/include
     ${JD_PLUGIN_FOLDER}/osal/include
     ${JD_PLUGIN_FOLDER}/streamif/include
     ${CMAKE_CURRENT_SOURCE_DIR}/../../../core/include
     ${JD_PLUGIN_FOLDER}/auddec/include
     ${FFMPEG_DIST_DIR}/${ANDROID_ABI}/include
     )
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../core/jni subproject/onyxcore)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../plugins/jdudp/jni subproject/jdudp)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../plugins/streamif/jni subproject/streamif)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../plugins/osal/jni subproject/osal)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../plugins/parseutil/jni subproject/parseutil)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../plugins/auddec/jni subproject/auddec)

add_library(UdpPlayerApi SHARED
 	jUdpPlayerApi.cpp
	jUdpPlayerEvents.cpp 
	ServerNodeBase.cpp  
	ServerNodeUdp.cpp  
	UdpMultiPlayer.cpp) 

add_library( libavcodec STATIC IMPORTED )
add_library( libavutil STATIC IMPORTED )
set_target_properties( libavcodec PROPERTIES IMPORTED_LOCATION
                       ${FFMPEG_DIST_DIR}/${ANDROID_ABI}/lib/libavcodec.a )
set_target_properties( libavutil PROPERTIES IMPORTED_LOCATION
                       ${FFMPEG_DIST_DIR}/${ANDROID_ABI}/lib/libavutil.a )

add_library(AudDecApi SHARED
 	jAudDecApi.cpp
    )

target_link_libraries(
    AudDecApi
    streamif
    auddec
    libavcodec
    libavutil
    android
    log
    )

target_link_libraries(
    UdpPlayerApi
    AudDecApi
    onyxcore
    jdudp
    jdosal
    streamif
    parseutil
    android
    log
    )



