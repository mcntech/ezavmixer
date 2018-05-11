set(LOCAL_PATH .)
set(JD_PLUGIN_FOLDER ${LOCAL_PATH}/../..)
set(JD_EXTLIBS_FOLDER ${LOCAL_PATH}/../../../extlibs)

include_directories(
     ${LOCAL_PATH}/../../../logutil/include
     ${LOCAL_PATH}/../include
     ${LOCAL_PATH}/..
     ${LOCAL_PATH}/../../../logutil/include
     ${JD_EXTLIBS_FOLDER}/Simple-WebSocket-Server
     ${JD_EXTLIBS_FOLDER}/openssl_dist/android/${ANDROID_ABI}/include
     ${JD_EXTLIBS_FOLDER}/asio/asio/include
     )
add_definitions( -DUSE_STANDALONE_ASIO -DSTANDALONE_ASIO)
add_library(jdwebsocket STATIC
 	../src/JdWebsocket.cpp
    )
