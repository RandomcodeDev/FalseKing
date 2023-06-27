## Requirements
- BSD make
- Clang and LLD

## Instructions
- Run `scripts/build.sh` in the root of the repo

## Notes
- This _should_ build on most Unixes once I build libraries for them, but only Linux is supported right now
- `scripts/distrib.sh` creates a convenient tar file
- You need to set `LD_LIBRARY_PATH`, i.e. `LD_LIBRARY_PATH=. ./Game.x64` (replace `.` with whatever path it's in if not the current directory)