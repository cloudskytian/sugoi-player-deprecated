TEMPLATE = lib

CONFIG += dll

DEFINES += _STATIC_BUILD

RC_FILE += player.rc

include(src.pri)

HEADERS += sugoilib_global.h sugoilib.h

SOURCES += guardmain.cpp
