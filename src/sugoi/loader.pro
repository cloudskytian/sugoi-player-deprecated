TEMPLATE = app

CONFIG -= app_bundle
CONFIG -= qt

RC_FILE += player.rc

SOURCES += loader.cpp

exists($$PWD/../../ci_version.h) {
    DEFINES += CI
}

contains(QT_ARCH, x86_64) {
    CONFIG(debug, debug|release) {
        TARGET = Sugoi64d
        DESTDIR = $$PWD/../../bin64/Debug
    } else {
        TARGET = Sugoi64
        DESTDIR = $$PWD/../../bin64/Release
    }
} else {
    CONFIG(debug, debug|release) {
        TARGET = Sugoid
        DESTDIR = $$PWD/../../bin/Debug
    } else {
        TARGET = Sugoi
        DESTDIR = $$PWD/../../bin/Release
    }
}

LIBS += -lKernel32 -lUser32

include(deploy.pri)

QMAKE_POST_LINK += $$quote(copy /y \"$$[QT_INSTALL_BINS]\\d3dcompiler_47.dll\" \"$${DESTDIR}\\d3dcompiler_47.dll\"$$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote(copy /y \"$$[QT_INSTALL_BINS]\\libEGL.dll\" \"$${DESTDIR}\\libEGL.dll\"$$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote(copy /y \"$$[QT_INSTALL_BINS]\\libGLESv2.dll\" \"$${DESTDIR}\\libGLESv2.dll\"$$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote(copy /y \"$$[QT_INSTALL_BINS]\\opengl32sw.dll\" \"$${DESTDIR}\\opengl32sw.dll\"$$escape_expand(\\n\\t))
