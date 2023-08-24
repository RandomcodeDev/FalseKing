# Dependency scripts

Basically, this is a way to more consistently copy arbitrary files for
build/distribution

## Variables
These are replaced if encountered:
 - `$GENARCH$`: the target architecture (copyfiles sees this as the
   architecture), e.g. `x86_64`, `x86`, `Universal` (macOS fat binary),
   `ARM64`, etc
 - `$BLDARCH$`: the target architecture as the build system sees it
   (`copyfiles.py` sees this as the platform), e.g. `x64`, `Gaming.Desktop.x64`
 - `$CONFIG$`: the build configuration, e.g. `Debug`, `Release`, `Retail`
 - `$DCONFIG$`: the build configuration mapped to what's available in the
   dependency tree, e.g `Debug` or `Release`
 - `$PREFIX$`: library prefix, e.g. `lib`
 - `$SUFFIX$`: library suffix, e.g. `-darwin`, `-switch`
 - `$SLIBEXT$`: static library extension, e.g. `.lib`, `.a`
 - `$DLIBEXT$`: dynamic library extension, e.g. `.dll`, `.dylib`, `.so`
 - `$EXEEXT$`: executable extension, e.g. `.exe`
 - `$DBGEXT$`: separate debug information extension, e.g. `.pdb`

## Syntax
The syntax is pretty simple, it's basically just a list of other scripts or a
filter to set what condition to copy a source to a destination. If `//` is
encountered, the rest of the line after that is ignored.

You can list other scripts:
```
!deplist
[script name]
[script name]
...
```

Or files to copy:
```
!depscript
[platform]:[architecture]:[configuration]~<source>=<destination>
```

## Naming of scripts
See `script_name()` in `scripts/depscript.py` for a sense of how it works
