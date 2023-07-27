# False King

![False King logo](build/windows/GdkAssets/Logo150x150.png)

[![Build](https://github.com/MobSlicer152/FalseKing/actions/workflows/build.yml/badge.svg)](https://github.com/MobSlicer152/FalseKing/actions/workflows/build.yml)

## About
The game is basically about a guy who kills kings with elemental powers to
become the False King and take over, which was my friend's idea.

I use C++ syntax for organization and to more conveniently represent things
that would be done with function pointer tables and preprocessor macros in C,
more so than for OOP (the game uses an ECS).

See [DESIGN.md]() for an overview of the game's design and plan.

## General build information
- Have `curl` (always available on supported Windows, may need to install on others)
- Have `7z` available in your path (install 7-zip/p7zip)
- Run `scripts\pulldeps.bat public` or `scripts/pulldeps.sh public` depending on your platform
- On Unix-like platforms (macOS, Linux, etc), run `chmod +x scripts/*.sh` in order for the build process to work

## Platform-specific instructions
- [Windows (Windows XP or later)](build/windows/BUILD.md)
- [UWP/Xbox](build/winrt/BUILD.md)
- [macOS](build/darwin/BUILD.md)
- [Unix](build/unix/BUILD.md)

This project uses FreeType.
