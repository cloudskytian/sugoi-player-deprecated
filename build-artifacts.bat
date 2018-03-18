@ECHO OFF
SETLOCAL EnableDelayedExpansion
IF NOT EXIST bin64 ECHO bin[64] directory not found! && GOTO Fin
CD /D bin64
IF NOT EXIST release ECHO release directory not found! && GOTO Fin
CD ..
SET "_ISCC_EXE=C:\Program Files (x86)\Inno Setup 5\ISCC.exe"
FOR /F "tokens=5*" %%A IN (
  'REG QUERY "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Inno Setup 5_is1" /v "Inno Setup: App Path" 2^>NUL ^| FIND "REG_SZ" ^|^|
   REG QUERY "HKLM\SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall\Inno Setup 5_is1" /v "Inno Setup: App Path" 2^>NUL ^| FIND "REG_SZ"') DO SET "_ISCC_EXE=%%B\ISCC.exe"
IF EXIST artifacts RD /S /Q artifacts
MD artifacts
"%~dp03rdparty\7za.exe" a -tzip ".\artifacts\Sugoi_x64_Portable.zip" ".\bin64\Release\*"
IF NOT EXIST "%_ISCC_EXE%" ECHO Inno Setup not found! && GOTO Fin
SET _CI_FLAG=
IF EXIST ci_version.h SET "_CI_FLAG=/DCI"
"%_ISCC_EXE%" /D_WIN64 %_CI_FLAG% /O"%~dp0artifacts" /F"Sugoi_x64_Setup" "%~dp0src\installer\installer.iss"
GOTO Fin

:Fin
ENDLOCAL
PAUSE
EXIT /B
