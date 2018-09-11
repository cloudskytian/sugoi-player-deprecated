QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS QT_DISABLE_DEPRECATED_BEFORE=0x050000

SOURCES += guardmain.cpp

exists($$PWD/../../ci_version.h) {
    DEFINES += CI
}

RC_FILE += guard.rc

TARGET = SugoiGuard

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
