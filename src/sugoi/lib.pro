QT += core gui widgets svg winextras network concurrent opengl

TEMPLATE = lib

CONFIG -= app_bundle
CONFIG += qt dll c++11

DEFINES += _STATIC_BUILD

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

RC_FILE += player.rc

exists($$PWD/../../ci_version.h) {
    DEFINES += CI
}

contains(QT_ARCH, x86_64) {
    LIBS += -L$$PWD/../../lib64 -llibmpv -lWinSparkle
    CONFIG(debug, debug|release) {
        DLLDESTDIR = $$PWD/../../bin64/Debug
        TARGET = Sugoi64d
    } else {
        DLLDESTDIR = $$PWD/../../bin64/Release
        TARGET = Sugoi64
    }
} else {
    LIBS += -L$$PWD/../../lib -llibmpv -lWinSparkle
    CONFIG(debug, debug|release) {
        DLLDESTDIR = $$PWD/../../bin/Debug
        TARGET = Sugoid
    } else {
        DLLDESTDIR = $$PWD/../../bin/Release
        TARGET = Sugoi
    }
}

LIBS += -lUser32 -lShell32 -lKernel32 -lDwmapi

HEADERS += sugoilib_global.h sugoilib.h
SOURCES += guardmain.cpp

include(src.pri)
