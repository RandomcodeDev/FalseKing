@echo off

setlocal enabledelayedexpansion enableextensions

if "%2" == "Gaming.Scarlett.x64" (
	set "ORIGCD=%CD%"
	call "%GameDK%Command Prompts\GamingScarlettVars.cmd" GamingScarlett2022
	cd /d %ORIGCD%
	set extraparams=
) else (
	set "ORIGCD=%CD%"
	call "%GameDK%Command Prompts\GamingDesktopVars.cmd" GamingDesktop2022
	cd /d %ORIGCD%
	set extraparams=/pc
)

rmdir /s/q "%1"
rmdir /s/q "%4"

call "%~dp0copyfiles" "%1" "%2" "%3"

copy "%~dp0..\build\windows\%2\%3\Game.%2.exe" "%1"
copy "%~dp0..\build\windows\%2\%3\Game.%2.pdb" "%1"

copy "%~dp0..\build\windows\%2\%3\MicrosoftGame.Config" "%1"
copy "%~dp0..\build\windows\%2\%3\"*.dll "%1"
xcopy /e/i/y "%~dp0..\build\windows\GdkAssets" "%1\GdkAssets"
xcopy /e/i/y "%~dp0..\deps-public\licenses" "%1\licenses"
copy "%~dp0..\LICENSE.txt" "%1\licenses\falseking.txt"

mkdir "%4"
makepkg genmap /f "layout.xml" /d "%1"
makepkg pack /f "layout.xml" /lt /d "%1" /nogameos %extraparams% /pd "%4"
copy "%4\*.msixvc" "%4.msixvc"
del "layout.xml"
rmdir /s/q "%1"
