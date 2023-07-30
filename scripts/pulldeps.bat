@echo off

if "%1" == "" exit /b

if not exist "%~dp0..\deps-%1" (
    git clone --depth=1 "https://git.randomcode.dev/mobslicer152/FalseKing-deps-%1" "%~dp0..\deps-%1"
) else (
    pushd "%~dp0..\deps-%1"
    git pull
    popd
)

