QT += webkit \
    network
win32 {
	DEFINES += WIN32
        DEFINES += CONFIG_CTRL_IFACE
        DEFINES += _LLONG_AVAILABLE
        DEFINES += CONFIG_NATIVE_WINDOWS
        DEFINES += CONFIG_CTRL_IFACE_NAMED_PIPE
        DEFINES += MININI_ANSI
        LIBS += ws2_32.lib
}

unix {
        DEFINES += CONFIG_CTRL_IFACE
        DEFINES += CONFIG_CTRL_IFACE_UNIX
        SOURCES += hostap/src/utils/os_unix.c
	LIBS += -lpng -lfreetype -lz -lgthread-2.0
}

INCLUDEPATH	+= . .. hostap/src hostap/src hostap/src/common hostap/src/utils

HEADERS = mainwindow.h \
    buttonfactory.h \
    buttonlaunch.h \
    customstyle.h \
    menupage.h \
    onyxcontrol.h \
    osdmanager.h \
    minini.h \
    Onvif.h \
    rtsppublish.h \
    keyboard/ui_keyboard.h \
    keyboard/keyboard.h \
    advertisement.h \
    selectcamera.h \
    hlsserver.h \
    hlspublish.h \
    rtmppublish.h \
    rtspserver.h \
    activeserverlist.h \
    settingsgeneral.h \
    wifisetup.h \

TARGET = onyx_gui
SOURCES += main.cpp \
    mainwindow.cpp \
    buttonfactory.cpp \
    buttonlaunch.cpp \
    customstyle.cpp \
    menupage.cpp \
    onyxcontrol.cpp \
    osdmanager.cpp \
    minini.c \
    Onvif.c \
    rtsppublish.cpp \
    keyboard/keyboard.cpp \
    advertisement.cpp \
    selectcamera.cpp \
    hlsserver.cpp \
    hlspublish.cpp \
    rtmppublish.cpp \
    rtspserver.cpp \
    activeserverlist.cpp \
    settingsgeneral.cpp \
    wifisetup.cpp \
    hostap/src/common/wpa_ctrl.c


# install
target.path = .
INSTALLS += target

FORMS += \
    rtsppublish.ui \
    keyboard/keyboard.ui \
    advertisement.ui \
    selectcamera.ui \
    hlsserver.ui \
    hlspublish.ui \
    rtmppublish.ui \
    rtspserver.ui \
    activeserverlist.ui \
    settingsgeneral.ui \
    wifisetup.ui

