@ECHO OFF
SETLOCAL
CLS
IF NOT EXIST build.user.bat (
    ECHO Cannot find build.user.bat
    PAUSE
    EXIT /B
)
CALL build.user.bat
IF EXIST bin64 RD /S /Q bin64
CALL "%_QT_DIR_64%\bin\qtenv2.bat"
CALL "%_VC_BAT_PATH%" x64
SET "PATH=%_JOM_DIR%;%PATH%"
CD /D "%~dp0"
IF EXIST Makefile jom clean
qmake "sugoi-player.pro" -spec win32-msvc "CONFIG+=release"
jom qmake_all
jom && jom install
CD bin64
windeployqt Sugoi64.exe
DEL /F iconlib.lib
IF EXIST "%_QT_DIR_64%\bin\libeay32.dll" COPY /Y "%_QT_DIR_64%\bin\libeay32.dll" "%CD%\libeay32.dll"
IF EXIST "%_QT_DIR_64%\bin\ssleay32.dll" COPY /Y "%_QT_DIR_64%\bin\ssleay32.dll" "%CD%\ssleay32.dll"
IF EXIST "%_QT_DIR_64%\bin\libcrypto-1_1-x64.dll" COPY /Y "%_QT_DIR_64%\bin\libcrypto-1_1-x64.dll" "%CD%\libcrypto-1_1-x64.dll"
IF EXIST "%_QT_DIR_64%\bin\libssl-1_1-x64.dll" COPY /Y "%_QT_DIR_64%\bin\libssl-1_1-x64.dll" "%CD%\libssl-1_1-x64.dll"
CD ..
jom clean
ENDLOCAL
EXIT /B
