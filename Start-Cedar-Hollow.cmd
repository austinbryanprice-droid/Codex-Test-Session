@echo off
cd /d "%~dp0"
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Scripts\Setup-Windows.ps1" -Mode Setup
if errorlevel 1 goto failed
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Scripts\Setup-Windows.ps1" -Mode Open
if errorlevel 1 goto failed
exit /b 0
:failed
 echo Setup failed. Copy the first build error and Saved\Logs for diagnosis.
 pause
 exit /b 1
