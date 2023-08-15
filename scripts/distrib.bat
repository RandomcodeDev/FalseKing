@echo off

setlocal enabledelayedexpansion enableextensions

if "%4" == "" exit /b

rmdir /s/q "%1"
del "%4.zip"

call "%~dp0copyfiles" "%1" "%2" "%3"

copy "%~dp0..\build\%2\%3\Game.%2.exe" "%1"
copy "%~dp0..\build\%2\%3\Game.%2.pdb" "%1"

set "OUTDIR=%CD%"
pushd "%1"
7z a -tzip "%OUTDIR%\%4.zip" *
popd
rmdir /s/q "%1"

