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

RESOURCES += $$PWD/resources.qrc

TRANSLATIONS += $$PWD/translations/sugoi_zh_CN.ts

HEADERS += \
    $$PWD/mpvtypes.h \
    $$PWD/overlayhandler.h \
    $$PWD/overlay.h \
    $$PWD/util.h \
    $$PWD/widgets/customlabel.h \
    $$PWD/widgets/customlineedit.h \
    $$PWD/widgets/customslider.h \
    $$PWD/widgets/customsplitter.h \
    $$PWD/widgets/dimdialog.h \
    $$PWD/widgets/indexbutton.h \
    $$PWD/widgets/openbutton.h \
    $$PWD/widgets/playlistwidget.h \
    $$PWD/widgets/seekbar.h \
    $$PWD/ui/aboutdialog.h \
    $$PWD/ui/inputdialog.h \
    $$PWD/ui/jumpdialog.h \
    $$PWD/ui/locationdialog.h \
    $$PWD/ui/mainwindow.h \
    $$PWD/ui/preferencesdialog.h \
    $$PWD/ui/screenshotdialog.h \
    $$PWD/ui/keydialog.h \
    $$PWD/recent.h \
    $$PWD/fileassoc.h \
    $$PWD/widgets/progressindicatorbar.h \
    $$PWD/qtlocalpeer.h \
    $$PWD/qtsingleapplication.h \
    $$PWD/qtlockedfile.h \
    $$PWD/ui/sysinfodialog.h \
    $$PWD/skinmanager.h \
    $$PWD/widgets/mpvwidget.h \
    $$PWD/widgets/cframelesswindow.h \
    $$PWD/playbackmanager.h \
    $$PWD/widgets/logowidget.h \
    $$PWD/ui/propertieswindow.h

SOURCES += \
    $$PWD/playermain.cpp \
    $$PWD/overlayhandler.cpp \
    $$PWD/util.cpp \
    $$PWD/widgets/customlabel.cpp \
    $$PWD/widgets/customlineedit.cpp \
    $$PWD/widgets/customslider.cpp \
    $$PWD/widgets/customsplitter.cpp \
    $$PWD/widgets/dimdialog.cpp \
    $$PWD/widgets/indexbutton.cpp \
    $$PWD/widgets/openbutton.cpp \
    $$PWD/widgets/playlistwidget.cpp \
    $$PWD/widgets/seekbar.cpp \
    $$PWD/ui/aboutdialog.cpp \
    $$PWD/ui/inputdialog.cpp \
    $$PWD/ui/jumpdialog.cpp \
    $$PWD/ui/locationdialog.cpp \
    $$PWD/ui/mainwindow.cpp \
    $$PWD/ui/preferencesdialog.cpp \
    $$PWD/ui/screenshotdialog.cpp \
    $$PWD/ui/keydialog.cpp \
    $$PWD/overlay.cpp \
    $$PWD/configmanager.cpp \
    $$PWD/fileassoc.cpp \
    $$PWD/widgets/progressindicatorbar.cpp \
    $$PWD/qtlocalpeer.cpp \
    $$PWD/qtsingleapplication.cpp \
    $$PWD/qtlockedfile_win.cpp \
    $$PWD/qtlockedfile.cpp \
    $$PWD/ui/sysinfodialog.cpp \
    $$PWD/skinmanager.cpp \
    $$PWD/widgets/mpvwidget.cpp \
    $$PWD/widgets/cframelesswindow.cpp \
    $$PWD/playbackmanager.cpp \
    $$PWD/widgets/logowidget.cpp \
    $$PWD/ui/propertieswindow.cpp

FORMS += \
    $$PWD/ui/aboutdialog.ui \
    $$PWD/ui/inputdialog.ui \
    $$PWD/ui/jumpdialog.ui \
    $$PWD/ui/locationdialog.ui \
    $$PWD/ui/mainwindow.ui \
    $$PWD/ui/preferencesdialog.ui \
    $$PWD/ui/screenshotdialog.ui \
    $$PWD/ui/keydialog.ui \
    $$PWD/ui/sysinfodialog.ui \
    $$PWD/ui/propertieswindow.ui
