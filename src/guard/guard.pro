QT -= gui

CONFIG += qt c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp

exists($$PWD/../../ci_version.h) {
    DEFINES += CI
}

contains(QT_ARCH, x86_64) {
    CONFIG(debug, debug|release) {
        DESTDIR = $$PWD/../../bin64/Debug
        TARGET = SugoiGuard64d
    } else {
        DESTDIR = $$PWD/../../bin64/Release
        TARGET = SugoiGuard64
    }
} else {
    CONFIG(debug, debug|release) {
        DESTDIR = $$PWD/../../bin/Debug
        TARGET = SugoiGuardd
    } else {
        DESTDIR = $$PWD/../../bin/Release
        TARGET = SugoiGuard
    }
}

include($$PWD/../../version.pri)

QMAKE_TARGET_COMPANY = wangwenx190
QMAKE_TARGET_DESCRIPTION = Sugoi Player Guard
QMAKE_TARGET_COPYRIGHT = GPLv3
QMAKE_TARGET_PRODUCT = Sugoi Player Guard
RC_ICONS = ../player/resources/player.ico

QMAKE_POST_LINK += $$quote(windeployqt \"$${DESTDIR}\\$${TARGET}.exe\"$$escape_expand(\\n\\t))
