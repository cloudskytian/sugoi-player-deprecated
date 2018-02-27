TEMPLATE = app

CONFIG -= app_bundle
CONFIG -= qt

RC_FILE += guard.rc

SOURCES += guardloader.cpp

exists($$PWD/../../ci_version.h) {
    DEFINES += CI
}

contains(QT_ARCH, x86_64) {
    CONFIG(debug, debug|release) {
        TARGET = SugoiGuard64d
        DESTDIR = $$PWD/../../bin64/Debug
    } else {
        TARGET = SugoiGuard64
        DESTDIR = $$PWD/../../bin64/Release
    }
} else {
    CONFIG(debug, debug|release) {
        TARGET = SugoiGuardd
        DESTDIR = $$PWD/../../bin/Debug
    } else {
        TARGET = SugoiGuard
        DESTDIR = $$PWD/../../bin/Release
    }
}

LIBS += -lKernel32 -lUser32

#QMAKE_CFLAGS_RELEASE += /MT
#QMAKE_CXXFLAGS_RELEASE += /MT
#QMAKE_CFLAGS_DEBUG += /MTd
#QMAKE_CXXFLAGS_DEBUG += /MTd
#CONFIG(debug,debug|release) {
#    QMAKE_LFLAGS += /NODEFAULTLIB:\"LIBCMTD.lib\"
#} else {
#    QMAKE_LFLAGS += /NODEFAULTLIB:\"LIBCMT.lib\"
#}
