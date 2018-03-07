QMAKE_POST_LINK += $$quote(cd /d \"$${DESTDIR}\"$$escape_expand(\\n\\t))

QMAKE_POST_LINK += $$quote(copy /y \"$${PWD}\\..\\..\\3rdparty\\youtube-dl.exe\" \"$${DESTDIR}\\youtube-dl.exe\"$$escape_expand(\\n\\t))

contains(QT_ARCH, x86_64) {
    QMAKE_POST_LINK += $$quote(copy /y \"$${PWD}\\..\\..\\3rdparty\\bin64\\*\" \"$${DESTDIR}\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(if exist \"vcredist_msvc2017_x64.exe\" del /f /q \"vcredist_msvc2017_x64.exe\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(if exist \"vc_redist_msvc2017.x64.exe\" del /f /q \"vc_redist_msvc2017.x64.exe\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(if exist \"vcredist_x64.exe\" ren \"vcredist_x64.exe\" \"vcredist_msvc2017_x64.exe\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(if exist \"vc_redist.x64.exe\" ren \"vc_redist.x64.exe\" \"vc_redist_msvc2017.x64.exe\"$$escape_expand(\\n\\t))
} else {
    QMAKE_POST_LINK += $$quote(copy /y \"$${PWD}\\..\\..\\3rdparty\\bin\\*\" \"$${DESTDIR}\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(if exist \"vcredist_msvc2017_x86.exe\" del /f /q \"vcredist_msvc2017_x86.exe\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(if exist \"vc_redist_msvc2017.x86.exe\" del /f /q \"vc_redist_msvc2017.x86.exe\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(if exist \"vcredist_x86.exe\" ren \"vcredist_x86.exe\" \"vcredist_msvc2017_x86.exe\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(if exist \"vc_redist.x86.exe\" ren \"vc_redist.x86.exe\" \"vc_redist_msvc2017.x86.exe\"$$escape_expand(\\n\\t))
}

QMAKE_POST_LINK += $$quote(copy /y \"$$[QT_INSTALL_TRANSLATIONS]\\qt_zh_CN.qm\" \"$${DESTDIR}\\translations\\qt_zh_CN.qm\"$$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote(copy /y \"$$[QT_INSTALL_TRANSLATIONS]\\qt_zh_TW.qm\" \"$${DESTDIR}\\translations\\qt_zh_TW.qm\"$$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote(copy /y \"$${PWD}\\translations\\*.qm\" \"$${DESTDIR}\\translations\"$$escape_expand(\\n\\t))

QMAKE_POST_LINK += $$quote(if exist \"$${DESTDIR}\\stylesheets\" rd /s /q \"$${DESTDIR}\\stylesheets\"$$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote(md \"$${DESTDIR}\\stylesheets\"$$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote(copy /y \"$${PWD}\\stylesheets\\*.css\" \"$${DESTDIR}\\stylesheets\"$$escape_expand(\\n\\t))

CONFIG(release, debug|release) {
    QMAKE_POST_LINK += $$quote(copy /y \"$${PWD}\\..\\..\\3rdparty\\vcredist_msvc2010_x86.exe\" \"$${DESTDIR}\\vcredist_msvc2010_x86.exe\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(copy /y \"$${PWD}\\..\\..\\doc\\ReadMe.txt.deploy\" \"$${DESTDIR}\\ReadMe.txt\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(copy /y \"$${PWD}\\..\\..\\doc\\Changelog.txt.deploy\" \"$${DESTDIR}\\Changelog.txt\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(copy /y \"$${PWD}\\..\\..\\doc\\License.txt.deploy\" \"$${DESTDIR}\\License.txt\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(copy /y \"$${PWD}\\..\\..\\doc\\Contributors.txt.deploy\" \"$${DESTDIR}\\Contributors.txt\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(if exist \"$${DESTDIR}\\licenses\" rd /s /q \"$${DESTDIR}\\licenses\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(xcopy /e /i /r /y \"$${PWD}\\..\\..\\doc\\licenses\" \"$${DESTDIR}\\licenses\"$$escape_expand(\\n\\t))
}
