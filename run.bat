@echo off
cd /d "%~dp0"
chcp 65001 >nul

if not exist build\soj.exe (
    echo [ERROR] build\soj.exe was not found.
    echo Run build.bat first.
    pause
    exit /b 1
)

build\soj.exe
pause
