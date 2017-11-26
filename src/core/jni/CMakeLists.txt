cmake_minimum_required(VERSION 3.4.1)


set(LOCAL_PATH .)
set(JD_PLUGIN_FOLDER ${LOCAL_PATH}/../../plugins)

include_directories(
     ${LOCAL_PATH}/../include
     ${LOCAL_PATH}/..
     ${JD_PLUGIN_FOLDER}/osal/include
     ${LOCAL_PATH}/../codecdm81xx/include
     ${LOCAL_PATH}/../../logutil/include
     ${JD_PLUGIN_FOLDER}/configdb/include
     ${JD_PLUGIN_FOLDER}/jdrtsp/include
     ${JD_PLUGIN_FOLDER}/mpegdash/include
     ${JD_PLUGIN_FOLDER}/tinyxml/include
     ${JD_PLUGIN_FOLDER}/jdhttp/include
     ${JD_PLUGIN_FOLDER}/httplive/include
     ${JD_PLUGIN_FOLDER}/tsmux/include
     ${JD_PLUGIN_FOLDER}/tsdemux/include
     ${JD_PLUGIN_FOLDER}/mp4mux/include
     ${JD_PLUGIN_FOLDER}/parseutil/include
     ${JD_PLUGIN_FOLDER}/jdaws/include
     ${JD_PLUGIN_FOLDER}/jdudp/include
     )
add_library(onyxcore STATIC
 	../src/UdpClntBridge.cpp
 	../src/MediaSwitch.cpp
    ../src/RtspPublishBridge.cpp
    ../src/StrmInBridgeBase.cpp
    ../src/RtspClntBridge.cpp
    ../src/UdpClntBridge.cpp
    ../src/StrmOutBridgeBase.cpp
    ../src/RtspConfigure.cpp
    ../src/MpdSrvBridge.cpp
    ../src/StreamUtil.cpp
    ../src/StrmConnWrapper.cpp
    ../src/InprocStrmConn.cpp
    ../src/InputStrmBase.cpp
    )
