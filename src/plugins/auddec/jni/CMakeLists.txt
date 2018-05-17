set(LOCAL_PATH .)
set(JD_PLUGIN_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(FFMPEG_DIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../ffmpeg_dist/android )

include_directories(
     ${CMAKE_CURRENT_SOURCE_DIR}/../include
     ${CMAKE_CURRENT_SOURCE_DIR}/..
     ${CMAKE_CURRENT_SOURCE_DIR}/../../../logutil/include
     ${FFMPEG_DIST_DIR}/${ANDROID_ABI}/include
     )
add_library(auddec STATIC
 	../src/decac3.cpp
 	../src/decaac.cpp
 	../src/decmp2.cpp
 	 ../src/audfft.cpp
    )
