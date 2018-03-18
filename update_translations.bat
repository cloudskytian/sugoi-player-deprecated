@ECHO OFF
SETLOCAL EnableDelayedExpansion
IF NOT EXIST build.user.bat ECHO Build.user.bat is missing! Compilation aborted! && GOTO Fin
CALL build.user.bat
CALL "%_QT_DIR%\bin\qtenv2.bat"
CALL "%_VC_BUILD_DIR%\vcvars64.bat"
CD /D "%~dp0src\sugoi"
lupdate player.pro
lrelease player.pro
GOTO Fin

:Fin
ENDLOCAL
PAUSE
EXIT /B
