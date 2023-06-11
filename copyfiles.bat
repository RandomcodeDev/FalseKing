@echo off

mkdir %1\AppX
copy %~dp0deps\lib\%2\%3\*.dll %1\AppX
copy %~dp0deps\lib\%2\*.dll %1\AppX
xcopy /e/i/y %~dp0assets %1\AppX\assets
