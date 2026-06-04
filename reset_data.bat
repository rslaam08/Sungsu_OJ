@echo off
cd /d "%~dp0"
if exist data\users.dat del data\users.dat
if exist data\problems.dat del data\problems.dat
if exist data\submissions.dat del data\submissions.dat
if exist data\promotions.dat del data\promotions.dat
if exist workspace\sources\*.c del /q workspace\sources\*.c
if exist workspace\executables\*.exe del /q workspace\executables\*.exe
if exist workspace\outputs\*.txt del /q workspace\outputs\*.txt
if exist workspace\errors\*.txt del /q workspace\errors\*.txt
echo [OK] Data reset. The program will recreate admin and sample problem on next run.
pause
