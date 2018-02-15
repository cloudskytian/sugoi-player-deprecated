TEMPLATE = lib
TARGET = iconlib
CONFIG -= qt
CONFIG += dll
RC_FILE += iconlib.rc
HEADERS += iconlib.h
SOURCES += iconlib.cpp
exists($$PWD/../../version.h) {
    DEFINES += CI
}
contains(QT_ARCH, x86_64) {
    CONFIG(debug, debug|release) {
        DLLDESTDIR = $$PWD/../../bin64/Debug
    } else {
        DLLDESTDIR = $$PWD/../../bin64/Release
    }
} else {
    CONFIG(debug, debug|release) {
        DLLDESTDIR = $$PWD/../../bin/Debug
    } else {
        DLLDESTDIR = $$PWD/../../bin/Release
    }
}
