lessThan(QT_MAJOR_VERSION, 5) {
    message("Cannot build Sugoi Player with Qt version $${QT_VERSION}")
    error("Use at least Qt 5.6.3")
}

!versionAtLeast(QT_VERSION, 5.6.3) {
    message("Cannot build Sugoi Player with Qt version $${QT_VERSION}")
    error("Use at least Qt 5.6.3")
}

TEMPLATE = app

isEmpty(ROOT): ROOT = $$PWD/../..

include($$ROOT/version.pri)

win32: RC_ICONS = resources/player.ico

DEFINES += QT_DEPRECATED_WARNINGS QT_DISABLE_DEPRECATED_BEFORE=0x050000

QT += gui widgets
qtHaveModule(svg): QT += svg
qtHaveModule(concurrent) {
    QT += concurrent
    DEFINES += QT_HAS_CONCURRENT
}
win32:qtHaveModule(winextras) {
    QT += winextras
    DEFINES += QT_HAS_WINEXTRAS
}
qtHaveModule(network) {
    QT += network
    include($$PWD/../3rdparty/singleapplication/singleapplication.pri)
    DEFINES += QT_HAS_NETWORK QAPPLICATION_CLASS=QApplication
}

CONFIG *= c++11
CONFIG -= app_bundle

TARGET = Sugoi

BIN_DIR = $$ROOT/build/bin
win32: LIB_DIR = $$ROOT/lib

contains(QT_ARCH, x86_64) {
    TARGET = $$join(TARGET,,,64)
    BIN_DIR = $$join(BIN_DIR,,,64)
    win32: LIB_DIR = $$join(LIB_DIR,,,64)
}

CONFIG(debug, debug|release) {
    win32: TARGET = $$join(TARGET,,,d)
    unix: TARGET = $$join(TARGET,,,_debug)
}

DESTDIR = $$BIN_DIR
OBJECTS_DIR = $$DESTDIR/../obj
MOC_DIR = $$DESTDIR/../moc
RCC_DIR = $$DESTDIR/../rcc
UI_DIR = $$DESTDIR/../ui

target.path = $$BIN_DIR

win32 {
    qtlibfiles.path = $$BIN_DIR
    qtlibfiles.commands += $$quote(\"$$[QT_INSTALL_BINS]\\windeployqt.exe\" --force --compiler-runtime --plugindir \"$${BIN_DIR}\\plugins\" \"$${BIN_DIR}\\$${TARGET}.exe\")
    #qtlibfiles.commands += $$quote(echo [Paths] > \"$${BIN_DIR}\\qt.conf\")
    #qtlibfiles.commands += $$quote(echo Prefix=. >> \"$${BIN_DIR}\\qt.conf\")
    #qtlibfiles.commands += $$quote(echo Binaries=. >> \"$${BIN_DIR}\\qt.conf\")
    #qtlibfiles.commands += $$quote(echo Libraries=. >> \"$${BIN_DIR}\\qt.conf\")
    #qtlibfiles.commands += $$quote(echo Plugins=plugins >> \"$${BIN_DIR}\\qt.conf\")
    #qtlibfiles.commands += $$quote(echo Imports=imports >> \"$${BIN_DIR}\\qt.conf\")
    #qtlibfiles.commands += $$quote(echo Qml2Imports=qml >> \"$${BIN_DIR}\\qt.conf\")
    #qtlibfiles.commands += $$quote(echo Translations=translations >> \"$${BIN_DIR}\\qt.conf\")
    qtlibfiles.commands  = $$join(qtlibfiles.commands,$$escape_expand(\\n\\t))
}

qmfiles.files = $$[QT_INSTALL_TRANSLATIONS]/qt_zh_CN.qm $$PWD/translations/*.qm
qmfiles.path = $$BIN_DIR/translations
win32: qmfiles.depends = qtlibfiles

skinfiles.files = $$PWD/stylesheets/*.css
skinfiles.path = $$BIN_DIR/stylesheets

3rdpartylicenses.files = $$ROOT/doc/licenses/*
3rdpartylicenses.path = $$BIN_DIR/licenses

sugoilicense.file = $$ROOT/LICENSE.md
sugoilicense.path = $$BIN_DIR

INSTALLS += target qmfiles skinfiles 3rdpartylicenses sugoilicense
win32: INSTALLS += qtlibfiles

win32 {
    LIBS += -lUser32 -lShell32 -lKernel32 -lDwmapi -L$${LIB_DIR} -lmpv
    INCLUDEPATH += $$ROOT/include
    DEPENDPATH += $$ROOT/include
}

macx {
    LIBS += -framework Cocoa
    OBJECTIVE_SOURCES += ui/cframelesswindow.mm
}

unix {
    QT_CONFIG -= no-pkg-config
    CONFIG += link_pkgconfig
    PKGCONFIG += mpv
}

#unix:!macx {
    #qtHaveModule(x11extras): QT += x11extras
#}

CONFIG(update_translations) {
    isEmpty(lupdate): lupdate = lupdate
    system("$${lupdate} -no-obsolete $${_PRO_FILE_}")
}

CONFIG(release_translations) {
    isEmpty(lrelease): lrelease = lrelease
    system("$${lrelease} -nounfinished -removeidentical $${_PRO_FILE_}")
}

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
    widgets/progressindicatorbar.cpp \
    skinmanager.cpp \
    mpvwidget.cpp \
    ui/cframelesswindow.cpp

win32 {
    HEADERS += fileassoc.h
    SOURCES += fileassoc.cpp
}

FORMS += \
    ui/aboutdialog.ui \
    ui/inputdialog.ui \
    ui/jumpdialog.ui \
    ui/locationdialog.ui \
    ui/mainwindow.ui \
    ui/preferencesdialog.ui \
    ui/screenshotdialog.ui \
    ui/keydialog.ui
