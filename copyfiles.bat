@echo off

mkdir %1\AppX
copy %~dp0deps\lib\%2\%3\*.dll %1\AppX
copy %~dp0deps\lib\%2\*.dll %1\AppX
