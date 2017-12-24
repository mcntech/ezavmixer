set(LOCAL_PATH .)
set(JD_PLUGIN_FOLDER ${LOCAL_PATH}/../..)

include_directories(
     ${LOCAL_PATH}/../include
     ${LOCAL_PATH}/..
     ${LOCAL_PATH}/../../../logutil/include
     )
add_library(parseutil STATIC
 	../src/h264parser.cpp
 	../src/mpeg2parser.cpp
    )
