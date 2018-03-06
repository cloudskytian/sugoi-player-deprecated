@ECHO OFF
IF NOT EXIST bin64 (
    ECHO bin[64] directory not found!
    PAUSE
    EXIT
)
SET "PATH=%~dp03rdparty;%PATH%"
upx --best --force "%~dp0bin64\release\libmpv.dll"
upx --best --force "%~dp0bin64\release\opengl32sw.dll"
upx --best --force "%~dp0bin64\release\*.exe"
PAUSE
