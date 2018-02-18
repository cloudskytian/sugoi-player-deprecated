TEMPLATE = app
QT = core
CONFIG += qt console
LIBS += -lUser32
contains(QT_ARCH, x86_64) {
    CONFIG(debug, debug|release) {
        DESTDIR = $$PWD/../../bin64/Debug
        TARGET = SPlayerService64d
    } else {
        DESTDIR = $$PWD/../../bin64/Release
        TARGET = SPlayerService64
    }
} else {
    CONFIG(debug, debug|release) {
        DESTDIR = $$PWD/../../bin/Debug
        TARGET = SPlayerServiced
    } else {
        DESTDIR = $$PWD/../../bin/Release
        TARGET = SPlayerService
    }
}
exists($$PWD/../../ci_version.h) {
    DEFINES += CI
}
include($$PWD/../../version.pri)
QMAKE_TARGET_COMPANY = wangwenx190
QMAKE_TARGET_DESCRIPTION = SPlayer Service
QMAKE_TARGET_COPYRIGHT = GPLv3
QMAKE_TARGET_PRODUCT = SPlayer Service
RC_ICONS = ../splayer/resources/splayer.ico
QMAKE_POST_LINK += $$quote(windeployqt \"$${DESTDIR}\\$${TARGET}.exe\"$$escape_expand(\\n\\t))
HEADERS += \
    qtservice.h \
    qtservice_p.h

SOURCES += \
    qtservice.cpp \
    qtservice_win.cpp \
    main.cpp
