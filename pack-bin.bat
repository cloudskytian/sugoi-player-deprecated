@ECHO OFF
SETLOCAL EnableDelayedExpansion
IF NOT EXIST bin64 ECHO bin[64] directory not found! && GOTO Fin
CD /D bin64
IF NOT EXIST release ECHO release directory not found! && GOTO Fin
CD /D release
SET "_UPX_EXE=%~dp03rdparty\upx.exe"
IF EXIST "libGLESV2.dll" "%_UPX_EXE%" --best --force "libGLESV2.dll"
IF EXIST "libmpv.dll" "%_UPX_EXE%" --best --force "libmpv.dll"
IF EXIST "opengl32sw.dll" "%_UPX_EXE%" --best --force "opengl32sw.dll"
IF EXIST "Sugoi*.dll" "%_UPX_EXE%" --best --force "Sugoi*.dll"
IF EXIST "Sugoi*.exe" "%_UPX_EXE%" --best --force "Sugoi*.exe"
IF EXIST "WinSparkle.dll" "%_UPX_EXE%" --best --force "WinSparkle.dll"
IF EXIST "youtube-dl.exe" "%_UPX_EXE%" --best --force "youtube-dl.exe"
IF EXIST "Qt*.dll" "%_UPX_EXE%" --best --force "Qt*.dll"
GOTO Fin

:Fin
ENDLOCAL
PAUSE
EXIT /B
