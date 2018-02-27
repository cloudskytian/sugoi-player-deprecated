QMAKE_POST_LINK += $$quote(copy /y \"$${PWD}\\..\\..\\3rdparty\\youtube-dl.exe\" \"$${DESTDIR}\\youtube-dl.exe\"$$escape_expand(\\n\\t))

contains(QT_ARCH, x86_64) {
    QMAKE_POST_LINK += $$quote(copy /y \"$${PWD}\\..\\..\\3rdparty\\bin64\\*\" \"$${DESTDIR}\"$$escape_expand(\\n\\t))
} else {
    QMAKE_POST_LINK += $$quote(copy /y \"$${PWD}\\..\\..\\3rdparty\\bin\\*\" \"$${DESTDIR}\"$$escape_expand(\\n\\t))
}

QMAKE_POST_LINK += $$quote(if exist \"$${DESTDIR}\\translations\" rd /s /q \"$${DESTDIR}\\translations\"$$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote(md \"$${DESTDIR}\\translations\"$$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote(copy /y \"$${PWD}\\translations\\*.qm\" \"$${DESTDIR}\\translations\"$$escape_expand(\\n\\t))

QMAKE_POST_LINK += $$quote(if exist \"$${DESTDIR}\\stylesheets\" rd /s /q \"$${DESTDIR}\\stylesheets\"$$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote(md \"$${DESTDIR}\\stylesheets\"$$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote(copy /y \"$${PWD}\\stylesheets\\*.css\" \"$${DESTDIR}\\stylesheets\"$$escape_expand(\\n\\t))

QMAKE_POST_LINK += $$quote(if exist \"$${DESTDIR}\\images\" rd /s /q \"$${DESTDIR}\\images\"$$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote(md \"$${DESTDIR}\\images\"$$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote(copy /y \"$${PWD}\\resources\\logo.png\" \"$${DESTDIR}\\images\\logo.png\"$$escape_expand(\\n\\t))

CONFIG(release, debug|release) {
    QMAKE_POST_LINK += $$quote(copy /y \"$${PWD}\\..\\..\\doc\\ReadMe.txt.deploy\" \"$${DESTDIR}\\ReadMe.txt\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(copy /y \"$${PWD}\\..\\..\\doc\\Changelog.txt.deploy\" \"$${DESTDIR}\\Changelog.txt\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(copy /y \"$${PWD}\\..\\..\\doc\\License.txt.deploy\" \"$${DESTDIR}\\License.txt\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(copy /y \"$${PWD}\\..\\..\\doc\\Contributors.txt.deploy\" \"$${DESTDIR}\\Contributors.txt\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(if exist \"$${DESTDIR}\\licenses\" rd /s /q \"$${DESTDIR}\\licenses\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(xcopy /e /i /r /y \"$${PWD}\\..\\..\\doc\\licenses\" \"$${DESTDIR}\\licenses\"$$escape_expand(\\n\\t))
}
