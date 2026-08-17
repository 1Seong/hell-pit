@echo off
setlocal

if not exist build mkdir build

powershell -NoProfile -ExecutionPolicy Bypass -File scripts\make_icon.ps1 -OutPath build\hellpit.ico
if errorlevel 1 exit /b 1

rc /nologo /fo build\resources.res src\resources.rc
if errorlevel 1 exit /b 1

cl /nologo /O1 /GS- /GR- /std:c++17 ^
  src\main.cpp build\resources.res ^
  /link /SUBSYSTEM:WINDOWS "/OUT:build\HELL PIT.exe" user32.lib gdi32.lib winmm.lib

endlocal
