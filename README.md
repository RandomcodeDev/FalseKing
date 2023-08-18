# False King

![False King logo](gdk/Assets/Logo150x150.png)

[![Build](https://github.com/MobSlicer152/FalseKing/actions/workflows/build.yml/badge.svg)](https://github.com/MobSlicer152/FalseKing/actions/workflows/build.yml)

## About

The game is basically about a guy who kills kings with elemental powers to
become the False King and take over, which was my friend's idea.

I use C++ syntax for organization and to more conveniently represent things
that would be done with function pointer tables and preprocessor macros in C,
more so than for OOP (the game uses an ECS).

See [](DESIGN.md) for an overview of the game's design and plan.

## Requirements

- Python 3
- An OS that isn't totally cursed
- A C++ compiler with C++17 support. For Windows, I recommend Visual Studio
  2022, for macOS I recommend Xcode, and for Linux and others I recommend
  Clang. However, most popular compilers should work.
- Currently, Windows x86 and Gaming.Desktop.x64 are supported, Linux x86-64,
  and macOS Universal are supported
- To run the game on Windows XP, you need a CPU that can run the latest Visual
  C++ runtime it supports

## Build instructions

- Run `python3 scripts/pulldeps.py`
- Add `deps-public/bin` to your `PATH`
- Use Premake to generate the appropriate project files for your platform (e.g.
  `premake5 --os=gaming_desktop vs2022` or `premake5 gmake2`)
- On Windows, open `build\Game-vs2022.sln`, on everything else, the Makefile
  will be `build/Game-gmake2.mak`
- On Linux, in order to link correctly, you need to use LLD because it does
  partial linking of static libraries by default, which is necessary to link
  to one of the PhysX libraries. Example: `make -C build -f Game-gmake2.mak
  LDFLAGS=-fuse-ld=lld`
- There is also a separate solution/Makefile for tools (`Tools-vs2022.sln`/
  `Tools-gmake2.mak`), which you might want if you're making a mod or
  something.

## Assets

Right now, the [assets](https://git.randomcode.dev/mobslicer152/FalseKing-assets)
are freely available under the same license as the code. However, that might
change once I release the game, similar to what games like DOOM have done. They
are stored in folders or in Valve Pack Files. To create those pack files, you
can use `vpktool` (you have to build the tools), the `vpk` Python module, or
the official Source Engine `vpk.exe`. They both offer a bit more functionality
than `vpktool`, but it also works well enough. The game looks in `assets/` and
`assets.vpk` by default, plus some places dependant on the platform. Possibly,
some way to include other locations without modifying the binary might be
added.

## Third-party code and files

See [here](https://git.randomcode.dev/mobslicer152/FalseKing-deps-public) for
the dependencies in use. Additionally, various snippets from other projects
have been used. Where this is the case, I've done my best to put a link to the
source of the snippet.
