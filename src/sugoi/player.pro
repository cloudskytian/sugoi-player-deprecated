TEMPLATE = app

RC_FILE += player.rc

include(src.pri)

QMAKE_POST_LINK += $$quote(windeployqt \"$${DESTDIR}\\$${TARGET}.exe\"$$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote(copy /y \"$$[QT_INSTALL_PLUGINS]\\platforms\\qdirect2d.dll\" \"$${DESTDIR}\\platforms\\qdirect2d.dll\"$$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote(copy /y \"$$[QT_INSTALL_PLUGINS]\\platforms\\qminimal.dll\" \"$${DESTDIR}\\platforms\\qminimal.dll\"$$escape_expand(\\n\\t))
QMAKE_POST_LINK += $$quote(copy /y \"$$[QT_INSTALL_PLUGINS]\\platforms\\qoffscreen.dll\" \"$${DESTDIR}\\platforms\\qoffscreen.dll\"$$escape_expand(\\n\\t))

include(deploy.pri)
