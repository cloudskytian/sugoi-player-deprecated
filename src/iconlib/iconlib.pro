TEMPLATE = lib
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11
win32: CONFIG += dll
HEADERS += iconlib.h
SOURCES += iconlib.cpp

# Enable Whole Program Optimization and Link Time Code Generation
win32:!win32-g++* {
    QMAKE_CFLAGS_RELEASE   += -GL
    QMAKE_CXXFLAGS_RELEASE += -GL
    QMAKE_LFLAGS_RELEASE   += /LTCG
}

win32-icc {
    QMAKE_CFLAGS_RELEASE = $$replace(QMAKE_CFLAGS_RELEASE, O2, O3)
    QMAKE_CXXFLAGS_RELEASE = $$replace(QMAKE_CXXFLAGS_RELEASE, O2, O3)
    QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO = $$replace(QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO, O2, O3)
    QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO = $$replace(QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO, O2, O3)
}

win32: RC_FILE = iconlib.rc

TARGET = iconlib

BIN_DIR = $$PWD/../../build/bin

contains(QT_ARCH, x86_64) {
    TARGET = $$join(TARGET,,,64)
    BIN_DIR = $$join(BIN_DIR,,,64)
}

CONFIG(debug, debug|release) {
    win32: TARGET = $$join(TARGET,,,d)
    unix: TARGET = $$join(TARGET,,,_debug)
}

DESTDIR = $$BIN_DIR

target.path = $$BIN_DIR
INSTALLS += target
