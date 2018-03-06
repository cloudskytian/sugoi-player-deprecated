QT += core gui widgets svg winextras network concurrent opengl
TEMPLATE = app
CONFIG += qt c++11
CONFIG -= app_bundle

contains(QT_ARCH, x86_64) {
    LIBS += -L$$PWD/../../lib64 -llibmpv -lWinSparkle
    CONFIG(debug, debug|release) {
        TARGET = Sugoi64d
        DESTDIR = $$PWD/../../bin64/Debug
    } else {
        TARGET = Sugoi64
        DESTDIR = $$PWD/../../bin64/Release
    }
} else {
    LIBS += -L$$PWD/../../lib -llibmpv -lWinSparkle
    CONFIG(debug, debug|release) {
        TARGET = Sugoid
        DESTDIR = $$PWD/../../bin/Debug
    } else {
        TARGET = Sugoi
        DESTDIR = $$PWD/../../bin/Release
    }
}

LIBS += -lUser32 -lShell32 -lKernel32 -lDwmapi

exists($$PWD/../../ci_version.h) {
    DEFINES += CI
}

RC_FILE += player.rc

#QMAKE_PRE_LINK += $$quote(taskkill /F /IM \"$${TARGET}.exe\" /T$$escape_expand(\\n\\t))
#QMAKE_POST_LINK += $$quote(taskkill /F /IM \"youtube-dl.exe\" /T$$escape_expand(\\n\\t))

QMAKE_POST_LINK += $$quote(windeployqt \"$${DESTDIR}\\$${TARGET}.exe\"$$escape_expand(\\n\\t))

include(deploy.pri)

#QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
#QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(src.pri)
