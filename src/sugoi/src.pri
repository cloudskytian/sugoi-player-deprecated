INCLUDEPATH += $$PWD/../../include
DEPENDPATH += $$PWD/../../include

RESOURCES += resources.qrc

TRANSLATIONS += translations/sugoi_zh_CN.ts

HEADERS += \
    sugoiengine.h \
    mpvhandler.h \
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
    widgets/logowidget.h \
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
    skinmanager.h

SOURCES += \
    playermain.cpp \
    sugoiengine.cpp \
    sugoicommands.cpp \
    mpvhandler.cpp \
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
    widgets/logowidget.cpp \
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
    skinmanager.cpp

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
