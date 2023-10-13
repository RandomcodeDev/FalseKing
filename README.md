# False King

![False King logo](gdk/Assets/Logo150x150.png)

[![Build](https://github.com/MobSlicer152/FalseKing/actions/workflows/build.yml/badge.svg)](https://github.com/MobSlicer152/FalseKing/actions/workflows/build.yml)

## About

The game is basically about a guy who kills kings with elemental powers to
become the False King and take over, which was my friend's idea.

This is only my second Rust project, but I'm (kind of) trying to make it
at least a little safe and kind of idiomatic.

See [](DESIGN.md) for an overview of the game's design and plan.

## Requirements

- An OS that isn't totally cursed
- Rust (probably some version)
- Currently, `x86_64-pc-windows-{msvc,gnu}`, `x86_64-pc-linux-gnu`, and `i586-pc-windows-msvc` seem to work.
- To run the game on Windows XP, I haven't quite nailed down the CPU 
  requirements, but I'm trying to get it to work on a Pentium (i586)

## Build instructions

- Do `cargo build`
- Hope it works
- Do `cargo run`
- Hope it doesn't crash

## Assets

Right now, the [assets](https://git.randomcode.dev/mobslicer152/FalseKing-assets)
are freely available under the same license as the code. However, that might
change once I release the game, similar to what games like DOOM have done. They
are stored in folders or in Valve Pack Files (not implemented again yet). To
create those pack files, you can use the `vpk` Python module, or the official
Source Engine `vpk.exe`. They both offer a bit more functionality than
`vpktool`, but it also works well enough. The game looks in `assets/` and
`assets.vpk` by default, plus some places dependant on the platform. Possibly,
some way to include other locations without modifying the binary might be added.

## Third-party code and files

See [here](https://git.randomcode.dev/mobslicer152/FalseKing-deps-public) for
the dependencies in use. Additionally, various snippets from other projects
have been used. Where this is the case, I've done my best to put a link to the
source of the snippet.
