@ECHO OFF

REM E.g. 'hello_world'
SET project_name=%1

REM Absolute path to project's root.
SET project_loc=%2

REM E.g. 'Debug'
SET config_name=%3

ECHO.
ECHO Project name: %project_name%
ECHO Project loc: %project_loc%
ECHO Config name: %config_name%
SET bin_src=%project_loc%\%config_name%\%project_name%.bin
ECHO.
:DoUntil
ECHO Searching for ARM PRO MINI
CALL :GetVolumeID "CRP DISABLD"
IF NOT exist %VOLUMEID%\ GOTO DoUntil
:EndDoUntil
SET "bin_dst=%VOLUMEID%\firmware.bin"
ECHO Will copy %bin_src% to %bin_dst%
COPY %bin_src% %bin_dst% /Y
ECHO.
ECHO DONE OK.
GOTO :eof

:GetVolumeID
SET VOLUMEID=$$$
FOR /F "tokens=1,* delims==" %%A IN ('"wmic volume where label='%~1' get deviceid /value 2>nul"') DO  IF NOT "%%~B"=="" SET "VOLUMEID=%%~B"
SET "VOLUMEID=%VOLUMEID:~0,-2%"
GOTO :eof