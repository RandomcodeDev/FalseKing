::@echo off

if "%3" == "" echo %0 ^<output^> ^<architecture^> ^<configuration^> & exit /b

if "%2" == "Gaming.Desktop.x64" (
    set gdk=1
    set arch=x86_64
    set "gdkpath=%GRDKLatest%"
) else if "%2" == "Gaming.Xbox.Scarlett.x64" (
    set gdk=1
    set arch=x86_64
    set "gdkpath=%GXDKLatest%"
) else (
    set arch=%2
)

mkdir "%1"
copy "%~dp0..\deps-public\lib\%arch%\%3\"*.dll "%1"
copy "%~dp0..\deps-public\lib\%arch%\%3\"*.pdb "%1"
copy "%~dp0..\deps-public\lib\%arch%\"*.dll "%1"
copy "%~dp0..\deps-public\lib\%arch%\"*.pdb "%1"

if "%gdk%" == "1" (
    echo "%gdkpath%"
    copy "%gdkpath%GameKit\Lib\amd64\"XCurl.dll "%1"
)

del "%1"\*-winrt.*
copy "%~dp0..\assets_"*.vpk "%1"
