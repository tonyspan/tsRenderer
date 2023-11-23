@echo off
set /p vs=vs2019 or vs2022: 
@echo
cd ..
@echo %cd%
call scripts\premake\bin\premake5.exe %vs%
PAUSE