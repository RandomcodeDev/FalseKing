[![Build](https://github.com/MobSlicer152/FalseKing/actions/workflows/build.yml/badge.svg)](https://github.com/MobSlicer152/FalseKing/actions/workflows/build.yml)

![False King logo](build/windows/GdkAssets/Logo150x150.png)

## About
The game is basically about a guy who kills kings with elemental powers to become the False King and take over, which was my friend's idea.

## Things likely to be in game
- 2D graphics with 3D space (so I can just implement drawing textures and pretty much nothing else, and still have it work)
- Maybe random generation, but only enough that it's fun, plus maybe more as NG+ or something, I find some games overdo the randomness
- Skill tree
- Elements: fire, air, water, earth
- Dashing
- Multiple types of attack: normal, special, ranged

I use C++ syntax to represent things that could be done with function pointer tables and preprocessor macros in C, more so than for OOP (game uses ECS).

## General build information
- Have `curl` (always available on supported Windows, may need to install on others)
- Have `7z` available in your path (install 7-zip/p7zip)
- Run `scripts\pulldeps.bat` or `scripts/pulldeps.sh` depending on your platform
- On Unix-like platforms (macOS, Linux, etc), run `chmod +x scripts/*.sh` in order for the build process to work
- Cloning submodules is only to get legally dubious platform support (currently Switch)

## Platform-specific instructions
- [Windows (Windows XP or later)](build/windows/BUILD.md)
- [UWP/Xbox](build/winrt/BUILD.md)
- [macOS](build/darwin/BUILD.md)
- [Unix](build/unix/BUILD.md)
- [Nintendo Switch](build/switch/BUILD.md) (submodule)
