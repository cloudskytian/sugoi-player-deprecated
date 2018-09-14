TEMPLATE = lib
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11
win32: CONFIG += dll
HEADERS += iconlib.h
SOURCES += iconlib.cpp

win32: RC_FILE = iconlib.rc

TARGET = iconlib

BIN_DIR = $$PWD/../../bin

contains(QT_ARCH, x86_64) {
    TARGET = $$join(TARGET,,,64)
    BIN_DIR = $$join(BIN_DIR,,,64)
}

CONFIG(debug, debug|release) {
    win32: TARGET = $$join(TARGET,,,d)
}

DESTDIR = $$BIN_DIR

target.path = $$BIN_DIR
INSTALLS += target
