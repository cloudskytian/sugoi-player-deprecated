@ECHO OFF
SET "_ARCH=%1"
CLS
IF NOT EXIST build.user.bat (
    ECHO Cannot find build.user.bat
    PAUSE
    EXIT /B
)
CALL build.user.bat
IF EXIST bin%_ARCH% RD /S /Q bin%_ARCH%
CALL "%_QT_DIR%%_ARCH%\bin\qtenv2.bat"
IF "%_ARCH%" == "" SET "_ARCH=86"
CALL "%_VC_BAT_PATH%" x%_ARCH%
SET "PATH=%_JOM_DIR%;%PATH%"
qmake "sugoi-player.pro" -spec win32-msvc "CONFIG+=release"
jom qmake_all
jom && jom install
jom clean
IF "%_ARCH%" == "86" CALL "%0" 64
EXIT /B
