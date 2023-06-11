@echo off

if not exist "%1\Game.pfx" echo y | powershell -command %~dp0makecert.ps1 "%1\AppX\AppXManifest.xml" "%1\Game.pfx" password
makeappx pack /d "%1\AppX" /p "%1\Game.appx"
signtool sign /fd SHA256 /p password /f "%1\Game.pfx" "%1\Game.appx"
