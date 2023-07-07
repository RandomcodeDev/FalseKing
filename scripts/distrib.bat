@echo off

setlocal enabledelayedexpansion enableextensions

rmdir /s/q "%1"
del "%4.zip"

call "%~dp0copyfiles" "%1" "%2" "%3"

copy "%~dp0..\build\windows\%2\%3\Game.%2.exe" "%1"
copy "%~dp0..\build\windows\%2\%3\Game.%2.pdb" "%1"

if "%2" == "Gaming.Desktop.x64" (
	copy "%~dp0..\build\windows\%2\%3\MicrosoftGameConfig.mgc" "%1"
	copy "%~dp0..\build\windows\%2\%3\"*.dll "%1"
	xcopy /e/i/y "%~dp0..\build\windows\GdkAssets" "%1\GdkAssets"
)

set "OUTDIR=%CD%"
pushd "%1"
7z a -tzip "%OUTDIR%\%4.zip" *
popd
rmdir /s/q "%1"

