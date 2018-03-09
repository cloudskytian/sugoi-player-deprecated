# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QT += core gui widgets svg winextras network concurrent opengl

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

INCLUDEPATH += $$PWD/../../include
DEPENDPATH += $$PWD/../../include

RESOURCES += resources.qrc

TRANSLATIONS += translations/sugoi_zh_CN.ts

HEADERS += \
    sugoiengine.h \
    mpvtypes.h \
    overlayhandler.h \
    overlay.h \
    util.h \
    widgets/customlabel.h \
    widgets/customlineedit.h \
    widgets/customslider.h \
    widgets/customsplitter.h \
    widgets/dimdialog.h \
    widgets/indexbutton.h \
    widgets/openbutton.h \
    widgets/playlistwidget.h \
    widgets/seekbar.h \
    ui/aboutdialog.h \
    ui/inputdialog.h \
    ui/jumpdialog.h \
    ui/locationdialog.h \
    ui/mainwindow.h \
    ui/preferencesdialog.h \
    ui/screenshotdialog.h \
    ui/keydialog.h \
    recent.h \
    fileassoc.h \
    widgets/progressindicatorbar.h \
    qtlocalpeer.h \
    qtsingleapplication.h \
    qtlockedfile.h \
    ui/sysinfodialog.h \
    skinmanager.h \
    mpvwidget.h \
    ui/cframelesswindow.h

SOURCES += \
    playermain.cpp \
    sugoiengine.cpp \
    sugoicommands.cpp \
    overlayhandler.cpp \
    util.cpp \
    widgets/customlabel.cpp \
    widgets/customlineedit.cpp \
    widgets/customslider.cpp \
    widgets/customsplitter.cpp \
    widgets/dimdialog.cpp \
    widgets/indexbutton.cpp \
    widgets/openbutton.cpp \
    widgets/playlistwidget.cpp \
    widgets/seekbar.cpp \
    ui/aboutdialog.cpp \
    ui/inputdialog.cpp \
    ui/jumpdialog.cpp \
    ui/locationdialog.cpp \
    ui/mainwindow.cpp \
    ui/preferencesdialog.cpp \
    ui/screenshotdialog.cpp \
    ui/keydialog.cpp \
    overlay.cpp \
    configmanager.cpp \
    fileassoc.cpp \
    widgets/progressindicatorbar.cpp \
    qtlocalpeer.cpp \
    qtsingleapplication.cpp \
    qtlockedfile_win.cpp \
    qtlockedfile.cpp \
    ui/sysinfodialog.cpp \
    skinmanager.cpp \
    mpvwidget.cpp \
    ui/cframelesswindow.cpp

FORMS += \
    ui/aboutdialog.ui \
    ui/inputdialog.ui \
    ui/jumpdialog.ui \
    ui/locationdialog.ui \
    ui/mainwindow.ui \
    ui/preferencesdialog.ui \
    ui/screenshotdialog.ui \
    ui/keydialog.ui \
    ui/sysinfodialog.ui
