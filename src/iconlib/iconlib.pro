TEMPLATE = lib
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += dll c++11
RC_FILE += iconlib.rc
HEADERS += iconlib.h
SOURCES += iconlib.cpp

exists($$PWD/../../ci_version.h) {
    DEFINES += CI
}

TARGET = iconlib

BIN_DIR = $$PWD/../../bin

contains(QT_ARCH, x86_64) {
    TARGET = $$join(TARGET,,,64)
    BIN_DIR = $$join(BIN_DIR,,,64)
}

CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,d)
}

target.path = $$BIN_DIR
INSTALLS += target
