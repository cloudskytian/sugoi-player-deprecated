@ECHO OFF
IF NOT EXIST bin64 (
    ECHO bin[64] directory not found!
    PAUSE
    EXIT
)
SET "PATH=%~dp03rdparty;%PATH%"
CD /D "%~dp0bin64\release"
IF EXIST "libGLESV2.dll" upx --best --force "libGLESV2.dll"
IF EXIST "libmpv.dll" upx --best --force "libmpv.dll"
IF EXIST "opengl32sw.dll" upx --best --force "opengl32sw.dll"
IF EXIST "Sugoi*.dll" upx --best --force "Sugoi*.dll"
IF EXIST "Sugoi*.exe" upx --best --force "Sugoi*.exe"
IF EXIST "WinSparkle.dll" upx --best --force "WinSparkle.dll"
IF EXIST "youtube-dl.exe" upx --best --force "youtube-dl.exe"
IF EXIST "Qt*.dll" upx --best --force "Qt*.dll"
PAUSE
