@ECHO OFF
IF NOT EXIST build.user.bat (
    ECHO Build.user.bat is missing! Compilation aborted!
    PAUSE
    EXIT
)
CALL build.user.bat
CALL "%_QT_DIR%\bin\qtenv2.bat"
CALL "%_VC_BUILD_DIR%\vcvars64.bat"
SET "PATH=%~dp03rdparty\jom;%PATH%"
CD /D "%~dp0"
IF EXIST bin64 RD /S /Q bin64
qmake "sugoi-player.pro" -spec win32-msvc "CONFIG+=release"
jom qmake_all
jom && jom install
CALL build-artifacts.bat
jom distclean
IF EXIST bin64 RD /S /Q bin64
PAUSE
EXIT
