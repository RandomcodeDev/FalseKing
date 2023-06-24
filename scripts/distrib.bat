@echo off

del %4.tar.xz

call %~dp0copyfiles %1 %2 %3

copy %~dp0..\%2\%3\Game\*.exe %1
copy %~dp0..\%2\%3\Game\*.pdb %1
copy %~dp0..\%2\%3\test\*.exe %1
copy %~dp0..\%2\%3\test\*.pdb %1

set "FILES="
for /f "usebackq tokens=*" %%x in (`dir /b %1`) do set "FILES=%FILES% %%x"

tar cvJf %4.tar.xz -C %1 %FILES%

