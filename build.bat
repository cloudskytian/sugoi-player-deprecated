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
qmake "splayer.pro" -spec win32-msvc "CONFIG+=release"
jom qmake_all
jom && jom install
"%~dp03rdparty\7za.exe" a -tzip SPlayer_x64_Portable.zip .\bin64\Release\*
"%_INNO_SETUP_DIR%\ISCC.exe" /Dx64 /O"%~dp0" /F"SPlayer_x64_Setup" "%~dp0src\installer\splayer.iss"
jom distclean
IF EXIST bin64 RD /S /Q bin64
PAUSE
EXIT
