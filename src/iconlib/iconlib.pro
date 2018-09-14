TEMPLATE = lib
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11
HEADERS += iconlib.h
SOURCES += iconlib.cpp

include($$PWD/../../version.pri)

win32: RC_ICONS = $$PWD/../player/resources/player.ico

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
