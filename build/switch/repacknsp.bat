@echo off
setlocal EnableExtensions EnableDelayedExpansion

mklink /h "%~dpnx2" "%~dpnx1"
set orig=%~dpnx2
set repacked=!orig:.nsp=_repacked.nsp!
del %repacked%
echo Repacking %orig% -^> %repacked%
for /f "usebackq tokens=*" %%x in (`where repack.exe`) do (
    "%%x" "%orig%" 2>&1 | findstr /v WARN
    set repack=true
    :: only do the first one found
    goto :eof
)
if "%repack%" neq "true" echo Could not find repack.exe ^(is it in the PATH?^)
:end