@echo off

if not exist "%~dp0..\deps" (
    git clone "https://git.randomcode.dev/mobslicer152/FalseKing-deps-%1" "%~dp0..\deps-%1"
)

