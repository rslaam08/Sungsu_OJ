@echo off
cd /d "%~dp0"
setlocal

where gcc >nul 2>nul
if errorlevel 1 (
    echo [ERROR] gcc was not found in PATH.
    echo Install MinGW-w64 or MSYS2 MinGW, then make sure gcc --version works in CMD.
    pause
    exit /b 1
)

if not exist build mkdir build

set SRC=src\main.c src\state.c src\storage.c src\utils.c src\menu.c src\user.c src\problem.c src\judge.c src\runner.c src\compare.c src\score.c src\ranking.c src\promotion.c

echo [BUILD] gcc -Iinclude -std=c11 -Wall -Wextra -O2 ... -o build\soj.exe
gcc -Iinclude -std=c11 -Wall -Wextra -O2 %SRC% -o build\soj.exe

if errorlevel 1 (
    echo [FAILED] Build failed.
    pause
    exit /b 1
)

echo [OK] build\soj.exe created.
pause
