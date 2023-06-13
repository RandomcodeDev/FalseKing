@echo off

mkdir %1
copy %~dp0deps\lib\%2\%3\*.dll %1
copy %~dp0deps\lib\%2\*.dll %1
xcopy /e/i/y %~dp0assets %1\assets
