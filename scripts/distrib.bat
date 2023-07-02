@echo off

setlocal enabledelayedexpansion enableextensions

rmdir /s/q %1
del %4.zip

call %~dp0copyfiles %1 %2 %3

copy %~dp0..\build\winxp\%2\%3\Game\*.exe %1
copy %~dp0..\build\winxp\%2\%3\Game\*.pdb %1
copy %~dp0..\build\winxp\%2\%3\test\*.exe %1
copy %~dp0..\build\winxp\%2\%3\test\*.pdb %1

set OUTDIR=%CD%
pushd %1
7z a -tzip %OUTDIR%\%4.zip *
popd
rmdir /s/q %1

