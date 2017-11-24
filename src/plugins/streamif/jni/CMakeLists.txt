set(LOCAL_PATH .)
set(JD_PLUGIN_FOLDER ${LOCAL_PATH}/../..)

include_directories(
     ${LOCAL_PATH}/../include
     ${LOCAL_PATH}/..
     ${JD_PLUGIN_FOLDER}/osal/include
     ${LOCAL_PATH}/../../../logutil/include
     )
add_library(streamif STATIC
 	../src/strmconn.c
    )
