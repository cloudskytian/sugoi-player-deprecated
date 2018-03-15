TEMPLATE = app

RC_FILE += player.rc

include(src.pri)

QMAKE_POST_LINK += $$quote(windeployqt \"$${DESTDIR}\\$${TARGET}.exe\"$$escape_expand(\\n\\t))

CONFIG(release, debug|release) {
    QMAKE_POST_LINK += $$quote(copy /y \"$$[QT_INSTALL_PLUGINS]\\Qt5Concurrent.dll\" \"$${DESTDIR}\\Qt5Concurrent.dll\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(copy /y \"$$[QT_INSTALL_PLUGINS]\\Qt5OpenGL.dll\" \"$${DESTDIR}\\Qt5OpenGL.dll\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(copy /y \"$$[QT_INSTALL_PLUGINS]\\platforms\\qdirect2d.dll\" \"$${DESTDIR}\\platforms\\qdirect2d.dll\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(copy /y \"$$[QT_INSTALL_PLUGINS]\\platforms\\qminimal.dll\" \"$${DESTDIR}\\platforms\\qminimal.dll\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(copy /y \"$$[QT_INSTALL_PLUGINS]\\platforms\\qoffscreen.dll\" \"$${DESTDIR}\\platforms\\qoffscreen.dll\"$$escape_expand(\\n\\t))
} else {
    QMAKE_POST_LINK += $$quote(copy /y \"$$[QT_INSTALL_PLUGINS]\\Qt5Concurrentd.dll\" \"$${DESTDIR}\\Qt5Concurrentd.dll\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(copy /y \"$$[QT_INSTALL_PLUGINS]\\Qt5OpenGLd.dll\" \"$${DESTDIR}\\Qt5OpenGLd.dll\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(copy /y \"$$[QT_INSTALL_PLUGINS]\\platforms\\qdirect2dd.dll\" \"$${DESTDIR}\\platforms\\qdirect2dd.dll\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(copy /y \"$$[QT_INSTALL_PLUGINS]\\platforms\\qminimald.dll\" \"$${DESTDIR}\\platforms\\qminimald.dll\"$$escape_expand(\\n\\t))
    QMAKE_POST_LINK += $$quote(copy /y \"$$[QT_INSTALL_PLUGINS]\\platforms\\qoffscreend.dll\" \"$${DESTDIR}\\platforms\\qoffscreend.dll\"$$escape_expand(\\n\\t))
}

include(deploy.pri)
