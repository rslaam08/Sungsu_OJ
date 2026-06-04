@echo off
cd /d "%~dp0"
if exist build\soj.exe del build\soj.exe
if exist workspace\sources\*.c del /q workspace\sources\*.c
if exist workspace\executables\*.exe del /q workspace\executables\*.exe
if exist workspace\outputs\*.txt del /q workspace\outputs\*.txt
if exist workspace\errors\*.txt del /q workspace\errors\*.txt
echo [OK] Cleaned build and workspace files.
pause
