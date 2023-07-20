@echo off

mkdir "%1"
copy "%~dp0..\deps-public\lib\%2\%3\"*.dll "%1"
copy "%~dp0..\deps-public\lib\%2\%3\"*.pdb "%1"
copy "%~dp0..\deps-public\lib\%2\"*.dll "%1"
copy "%~dp0..\deps-public\lib\%2\"*.pdb "%1"
del "%1"\*-winrt.*
xcopy /e/i/y "%~dp0..\assets" "%1\assets"
