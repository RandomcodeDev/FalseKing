@echo off

echo %1
mkdir "%1"
copy %~dp0communism.txt "%1"

