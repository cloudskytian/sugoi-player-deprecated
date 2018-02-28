@ECHO OFF
IF NOT EXIST build.user.bat (
    ECHO Build.user.bat is missing! Compilation aborted!
    PAUSE
    EXIT
)
IF NOT EXIST bin64 (
    ECHO bin[64] directory not found!
    PAUSE
    EXIT
)
CALL build.user.bat
SET "PATH=%~dp03rdparty;%_INNO_SETUP_DIR%;%PATH%"
IF EXIST artifacts RD /S /Q artifacts
MD artifacts
7za a -tzip .\artifacts\Sugoi_x64_Portable.zip .\bin64\Release\*
IF EXIST ci_version.h (
    ISCC /D_WIN64 /DCI /O"%~dp0artifacts" /F"Sugoi_x64_Setup" "%~dp0src\installer\installer.iss"
) ELSE (
    ISCC /D_WIN64 /O"%~dp0artifacts" /F"Sugoi_x64_Setup" "%~dp0src\installer\installer.iss"
)
PAUSE
