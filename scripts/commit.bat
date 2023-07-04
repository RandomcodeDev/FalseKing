@echo off

if "%1" == "" exit /b

set COMMIT=^<unknown^>
for /f "usebackq tokens=2" %%x in (`git log`) do (
    set COMMIT="%%x"
    goto :done1
)

:done1
if not exist "%1\commit.txt" goto :done2
for /f "usebackq tokens=1" %%x in (`type "%1\commit.txt"`) do (
    set LAST=%%x
    goto :done2
)

:done2
if "%COMMIT%" NEQ "%LAST%" (
    echo Updating commit hash (%COMMIT% vs %LAST%^)
    echo %COMMIT% > "%1\commit.txt"
)

