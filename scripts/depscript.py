import os

from typing import AnyStr

"""
Basically, this is a way to more consistently copy arbitrary files for
build/distribution

Variables:
    $GENARCH$=the target architecture, e.g. x86_64, x86, Universal, ARM64, etc
    $BLDARCH$=the target architecture as the build system sees it, e.g. x64,
              Gaming.Desktop.x64
    $CONFIG$=the build configuration, e.g. Debug, Release
    $PREFIX$=library prefix, e.g. lib
    $SUFFIX$=library suffix, e.g. -darwin, -switch
    $SLIBEXT$=static library extension, e.g. .lib, .a
    $DLIBEXT$=dynamic library extension, e.g. .dll, .dylib, .so

Syntax:
    [platform]:[architecture]:[configuration]~<source>=<destination>
"""
class DepScript:
    MAIN_SEPARATOR='~'
    CONDITION_SEPARATOR=':'
    EXPRESSION_SEPARATOR='='

    def __init__(self, path: os.PathLike[any]):
        with open(path, "r") as f:
            script = f.read()

        self.lines: list[self.Line] = []
        for line in script.split('\n'):
            self.lines.append(self.Line(line))

    def __str__(self) -> str:
        _ret = ''
        for line in self.lines:
            _ret = f"{_ret}, {line}" if len(_ret) > 0 else f"{line}"
        return _ret

    class Line:
        def __init__(self, system: AnyStr = "", architecture: AnyStr = "",
                     configuration: AnyStr = "", source: AnyStr = "",
                     destination: AnyStr = ""):
            self.system = system
            self.architecture = architecture
            self.configuration = configuration
            self.source = source
            self.destination = destination

        def __init__(self, line: AnyStr):
            real = ""
            for part in line.split():
                if len(part) > 0:
                    real += part
            cond, expr = real.split(DepScript.MAIN_SEPARATOR)
            cond = cond.split(DepScript.CONDITION_SEPARATOR)
            expr = expr.split(DepScript.EXPRESSION_SEPARATOR)

            self.system = cond[0]
            self.architecture = cond[1]
            self.configuration = cond[2]
            self.source = expr[0]
            self.destination = expr[1]

        def _match(self, a: AnyStr, b: AnyStr) -> bool:
            return ((len(a) == 0 or len(b) == 0) or a == b)

        def matches(self, system: AnyStr, architecture: AnyStr,
                    configuration: AnyStr) -> bool:
            return (self._match(self.system, system)
                and self._match(self.architecture, architecture)
                and self._match(self.configuration, configuration))
        
        def __str__(self) -> str:
            return (f"{self.system}:{self.architecture}:{self.configuration}"
                    f"~{self.source}={self.destination}")
