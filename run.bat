@echo off
echo Starting EduERP with Qt 6.8.2 Environment...

:: Temporarily add Qt DLLs to the system PATH for this session only
set PATH=C:\Qt\6.8.2\msvc2022_64\bin;%PATH%

:: Launch the executable from the current directory
start "" "%~dp0build\Release\EduERP.exe"

echo Launched successfully! You can close this window.
exit
