@echo off

if not exist "%1\%2.pfx" echo y | powershell -command %~dp0makecert.ps1 "%1\AppX\AppXManifest.xml" "%1\%2.pfx" password
makeappx pack /d "%1\AppX" /p "%1\%2.appx"
signtool sign /fd SHA256 /p password /f "%1\%2.pfx" "%1\%2.appx"
