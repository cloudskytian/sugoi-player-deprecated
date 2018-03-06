@ECHO OFF
IF NOT EXIST bin64 (
    ECHO bin[64] directory not found!
    PAUSE
    EXIT
)
SET "PATH=%~dp03rdparty;%PATH%"
CD /D "%~dp0bin64\release"
IF EXIST "libmpv.dll" upx --best --force "libmpv.dll"
IF EXIST "opengl32sw.dll" upx --best --force "opengl32sw.dll"
IF EXIST "Sugoi.dll" upx --best --force "Sugoi.dll"
IF EXIST "Sugoi64.dll" upx --best --force "Sugoi64.dll"
IF EXIST "SugoiGuard.exe" upx --best --force "SugoiGuard.exe"
IF EXIST "SugoiGuard64.exe" upx --best --force "SugoiGuard64.exe"
IF EXIST "Sugoi.exe" upx --best --force "Sugoi.exe"
IF EXIST "Sugoi64.exe" upx --best --force "Sugoi64.exe"
PAUSE
