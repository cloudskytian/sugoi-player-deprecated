TEMPLATE = lib
CONFIG -= app_bundle
CONFIG -= qt
CONFIG *= c++11
win32: CONFIG *= dll
HEADERS += iconlib.h
SOURCES += iconlib.cpp

isEmpty(ROOT): ROOT = $$PWD/../..

include($$ROOT/optimization.pri)

win32: RC_FILE = iconlib.rc

TARGET = iconlib

BIN_DIR = $$ROOT/build/bin

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
