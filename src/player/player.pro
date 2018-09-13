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

LIBS += -lUser32 -lShell32 -lKernel32 -lDwmapi -L$${LIB_DIR} -lmpv

INCLUDEPATH += $$PWD/../../include
DEPENDPATH += $$PWD/../../include

RESOURCES += $$PWD/resources.qrc

TRANSLATIONS += $$PWD/translations/sugoi_zh_CN.ts

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
    skinmanager.h \
    mpvwidget.h \
    ui/cframelesswindow.h

SOURCES += \
    main.cpp \
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
    ui/keydialog.ui
