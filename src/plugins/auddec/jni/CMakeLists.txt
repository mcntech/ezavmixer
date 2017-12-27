set(LOCAL_PATH .)
set(JD_PLUGIN_FOLDER ${LOCAL_PATH}/../..)
set(FFMPEG_DIST_DIR /opt/mcnt/ezavmixer/src/ffmpeg_dist/android )

include_directories(
     ${LOCAL_PATH}/../include
     ${LOCAL_PATH}/..
     ${LOCAL_PATH}/../../../logutil/include
     ${FFMPEG_DIST_DIR}/${ANDROID_ABI}/include
     )
add_library(auddec STATIC
 	../src/decac3.cpp
 	../src/decaac.cpp
 	../src/decmp2.cpp
    )
