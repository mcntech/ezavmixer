set(LOCAL_PATH .)
set(JD_PLUGIN_FOLDER ${LOCAL_PATH}/../..)

include_directories(
     ${LOCAL_PATH}/../include
     ${LOCAL_PATH}/..
     ${JD_PLUGIN_FOLDER}/osal/include
     ${LOCAL_PATH}/../codecdm81xx/include
     ${LOCAL_PATH}/../../../logutil/include
     ${JD_PLUGIN_FOLDER}/streamif/include
     )
add_library(jdudp STATIC
 	../src/filesrc.cpp
 	../src/udprx.cpp
    ../src/xport.cpp
    ../src/strmclock.cpp
    )
