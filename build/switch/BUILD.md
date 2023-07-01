## Requirements
- Visual Studio 2022 with the Nintendo SDK extension
- [Leaked Switch SDK from February 2023 BreachForums leak](https://archive.org/details/nintendo-switch-sdk-2023) at `C:\NintendoDev\NintendoSDK` (link may eventually be updated, especially if this one goes down)
- The tools used in [this guide](https://gbatemp.net/threads/how-to-create-nintendo-switch-games-with-unity.625751/) (hacPack and `repack.exe`) in your PATH and `prod.keys` in `%USERPROFILE%\.switch`

## Instructions
- Open Game.sln

## Notes
- Sometimes the NSP build process is finicky, so delete the NSP files in the output directory if your changes aren't reflected when you run it
- Only the repacked NSP boots in emulators, and I don't have a hacked Switch or a devkit (obviously) currently
- In the event that Nintendo takes this down, I will be sad. However, I'm going to eventually put the console stuff on a separate branch so it's the only loss in such an event.
