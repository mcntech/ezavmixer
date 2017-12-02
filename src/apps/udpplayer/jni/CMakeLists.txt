cmake_minimum_required(VERSION 3.4.1)


set(LOCAL_PATH .)
set(JD_PLUGIN_FOLDER ${LOCAL_PATH}/../../../plugins)

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
     )
add_subdirectory(${LOCAL_PATH}/../../../core/jni subproject/onyxcore)
add_subdirectory(${LOCAL_PATH}/../../../plugins/jdudp/jni subproject/jdudp)
add_subdirectory(${LOCAL_PATH}/../../../plugins/streamif/jni subproject/streamif)
add_subdirectory(${LOCAL_PATH}/../../../plugins/osal/jni subproject/osal)
add_subdirectory(${LOCAL_PATH}/../../../plugins/parseutil/jni subproject/parseutil)

add_library(UdpPlayerApi SHARED
 	jUdpPlayerApi.cpp
	jUdpPlayerEvents.cpp 
	ServerNodeBase.cpp  
	ServerNodeUdp.cpp  
	UdpMultiPlayer.cpp) 

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
