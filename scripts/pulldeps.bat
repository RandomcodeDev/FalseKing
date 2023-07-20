@echo off

if not exist "%~dp0..\deps" (
    git clone "http://99.225.158.138:3000/mobslicer152/FalseKing-deps-%1" "%~dp0..\deps-%1"
)

