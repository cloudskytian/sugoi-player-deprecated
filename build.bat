@ECHO OFF
SETLOCAL
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
CD /D "%~dp0"
qmake "sugoi-player.pro" -spec win32-msvc "CONFIG+=release"
jom qmake_all
jom && jom install
IF "%_ARCH%" == "86" (
    CD bin
    windeployqt Sugoi.exe
) ELSE (
    CD bin64
    windeployqt Sugoi64.exe
)
DEL /F /S /Q *.lib
IF EXIST "%_QT_DIR%\bin\libeay32.dll" COPY /Y "%_QT_DIR%\bin\libeay32.dll" "%CD%\libeay32.dll"
IF EXIST "%_QT_DIR%\bin\ssleay32.dll" COPY /Y "%_QT_DIR%\bin\ssleay32.dll" "%CD%\ssleay32.dll"
IF EXIST "%_QT_DIR%\bin\libcrypto-1_1.dll" COPY /Y "%_QT_DIR%\bin\libcrypto-1_1.dll" "%CD%\libcrypto-1_1.dll"
IF EXIST "%_QT_DIR%\bin\libssl-1_1.dll" COPY /Y "%_QT_DIR%\bin\libssl-1_1.dll" "%CD%\libssl-1_1.dll"
CD ..
jom clean
ENDLOCAL
IF "%_ARCH%" == "86" CALL "%~dp0build.bat" 64
EXIT /B
