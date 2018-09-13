TEMPLATE = app

RC_FILE += player.rc

DEFINES += QT_DEPRECATED_WARNINGS QT_DISABLE_DEPRECATED_BEFORE=0x050000

QT += gui widgets
qtHaveModule(svg): QT += svg
qtHaveModule(concurrent) {
    QT += concurrent
    DEFINES += QT_HAS_CONCURRENT
}
qtHaveModule(winextras) {
    QT += winextras
    DEFINES += QT_HAS_WINEXTRAS
}
qtHaveModule(network) {
    QT += network
    include($$PWD/../3rdparty/singleapplication/singleapplication.pri)
    DEFINES += QT_HAS_NETWORK QAPPLICATION_CLASS=QApplication
}

CONFIG += c++11
CONFIG -= app_bundle

TARGET = Sugoi

exists($$PWD/../../ci_version.h) {
    DEFINES += CI
}

BIN_DIR = $$PWD/../../bin
LIB_DIR = $$PWD/../../lib

contains(QT_ARCH, x86_64) {
    TARGET = $$join(TARGET,,,64)
    BIN_DIR = $$join(BIN_DIR,,,64)
    LIB_DIR = $$join(LIB_DIR,,,64)
}

CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,d)
}

DESTDIR = $$BIN_DIR

target.path = $$BIN_DIR

qmfiles.files = $$[QT_INSTALL_TRANSLATIONS]/qt_zh_CN.qm $$PWD/translations/*.qm
qmfiles.path = $$BIN_DIR/translations

skinfiles.files = $$PWD/stylesheets/*.css
skinfiles.path = $$BIN_DIR/stylesheets

licensefiles.files = $$PWD/../../doc/licenses/*
licensefiles.path = $$BIN_DIR/licenses

INSTALLS += target qmfiles skinfiles licensefiles

LIBS += \
    -lUser32 -lShell32 -lKernel32 -lDwmapi \
    -L$${LIB_DIR} -lmpv

INCLUDEPATH += $$PWD/../../include
DEPENDPATH += $$PWD/../../include

RESOURCES += $$PWD/resources.qrc

TRANSLATIONS += $$PWD/translations/sugoi_zh_CN.ts

HEADERS += \
    $$PWD/sugoiengine.h \
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
    $$PWD/skinmanager.h \
    $$PWD/mpvwidget.h \
    $$PWD/ui/cframelesswindow.h

SOURCES += \
    $$PWD/main.cpp \
    $$PWD/sugoiengine.cpp \
    $$PWD/sugoicommands.cpp \
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
    $$PWD/skinmanager.cpp \
    $$PWD/mpvwidget.cpp \
    $$PWD/ui/cframelesswindow.cpp

FORMS += \
    $$PWD/ui/aboutdialog.ui \
    $$PWD/ui/inputdialog.ui \
    $$PWD/ui/jumpdialog.ui \
    $$PWD/ui/locationdialog.ui \
    $$PWD/ui/mainwindow.ui \
    $$PWD/ui/preferencesdialog.ui \
    $$PWD/ui/screenshotdialog.ui \
    $$PWD/ui/keydialog.ui
