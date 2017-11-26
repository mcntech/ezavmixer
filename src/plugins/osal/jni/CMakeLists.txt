set(LOCAL_PATH .)
set(JD_PLUGIN_FOLDER ${LOCAL_PATH}/../..)

include_directories(
     ${LOCAL_PATH}/../include
     ${LOCAL_PATH}/..
     ${LOCAL_PATH}/../../../logutil/include
     )
add_library(jdosal STATIC
 	../src/JdOsal.cpp
    )
