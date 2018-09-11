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
IF "%_ARCH%" == "" SET "_ARCH=86"
IF "%_ARCH%" == "86" (
    CALL "%_QT_DIR_32%\bin\qtenv2.bat"
) ELSE (
    CALL "%_QT_DIR_64%\bin\qtenv2.bat"
)
CALL "%_VC_BAT_PATH%" x%_ARCH%
SET "PATH=%_JOM_DIR%;%PATH%"
CD /D "%~dp0"
IF EXIST Makefile jom clean
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
IF "%_ARCH%" == "86" (
    IF EXIST "%_QT_DIR_32%\bin\libeay32.dll" COPY /Y "%_QT_DIR_32%\bin\libeay32.dll" "%CD%\libeay32.dll"
    IF EXIST "%_QT_DIR_32%\bin\ssleay32.dll" COPY /Y "%_QT_DIR_32%\bin\ssleay32.dll" "%CD%\ssleay32.dll"
    IF EXIST "%_QT_DIR_32%\bin\libcrypto-1_1.dll" COPY /Y "%_QT_DIR_32%\bin\libcrypto-1_1.dll" "%CD%\libcrypto-1_1.dll"
    IF EXIST "%_QT_DIR_32%\bin\libssl-1_1.dll" COPY /Y "%_QT_DIR_32%\bin\libssl-1_1.dll" "%CD%\libssl-1_1.dll"
) ELSE (
    IF EXIST "%_QT_DIR_64%\bin\libeay32.dll" COPY /Y "%_QT_DIR_64%\bin\libeay32.dll" "%CD%\libeay32.dll"
    IF EXIST "%_QT_DIR_64%\bin\ssleay32.dll" COPY /Y "%_QT_DIR_64%\bin\ssleay32.dll" "%CD%\ssleay32.dll"
    IF EXIST "%_QT_DIR_64%\bin\libcrypto-1_1.dll" COPY /Y "%_QT_DIR_64%\bin\libcrypto-1_1.dll" "%CD%\libcrypto-1_1.dll"
    IF EXIST "%_QT_DIR_64%\bin\libssl-1_1.dll" COPY /Y "%_QT_DIR_64%\bin\libssl-1_1.dll" "%CD%\libssl-1_1.dll"
)
CD ..
jom clean
IF "%_ARCH%" == "86" CALL "%0" 64
ENDLOCAL
EXIT /B
