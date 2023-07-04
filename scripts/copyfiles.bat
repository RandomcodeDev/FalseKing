@echo off

mkdir "%1"
copy "%~dp0..\deps\lib\%2\%3\"*.dll "%1"
copy "%~dp0..\deps\lib\%2\%3\"*.pdb "%1"
copy "%~dp0..\deps\lib\%2\"*.dll "%1"
copy "%~dp0..\deps\lib\%2\"*.pdb "%1"
xcopy /e/i/y "%~dp0..\assets" "%1\assets"
