import os

from typing import AnyStr

def script_name(name: AnyStr, system: AnyStr, platform: AnyStr):
    script = f"{name}.txt"
    print(f"Checking for {script}")
    if not os.path.exists(script):
        script = f"{name}-{system}-{platform}.txt"
        print(f"Checking for {script}")
        if not os.path.exists(script):
            script = f"{name}-generic.txt"
            print(f"Checking for {script}")
            if not os.path.exists(script):
                script = "<not-found>"
                print(f"Could not find any expected name for script {name}")
    return script

"""
Basically, this is a way to more consistently copy arbitrary files for
build/distribution

Variables, these are replaced if encountered:
    $GENARCH$=the target architecture, e.g. x86_64, x86, Universal, ARM64, etc
    $BLDARCH$=the target architecture as the build system sees it, e.g. x64,
              Gaming.Desktop.x64
    $CONFIG$=the build configuration, e.g. Debug, Release
    $PREFIX$=library prefix, e.g. lib
    $SUFFIX$=library suffix, e.g. -darwin, -switch
    $SLIBEXT$=static library extension, e.g. .lib, .a
    $DLIBEXT$=dynamic library extension, e.g. .dll, .dylib, .so

Syntax:
    !deplist
    [script name]
    [script name]
    ...

    or

    !depscript
    [platform]:[architecture]:[configuration]~<source>=<destination>
"""
class DepScript:
    MAIN_SEPARATOR='~'
    CONDITION_SEPARATOR=':'
    EXPRESSION_SEPARATOR='='

    def __init__(self, path: os.PathLike[any], system: AnyStr,
                 platform: AnyStr, architecture: AnyStr,
                 configuration: AnyStr):
        with open(path, "r") as f:
            script = f.read()

        # TODO: Maybe make this if a little better
        if system in ["windows", "gaming_desktop", "scarlett"]:
            prefix = ""
            suffix = ""
            slibext = ".lib"
            dlibext = ".dll"
        elif system in ["macosx"]:
            prefix = "lib"
            suffix = "-darwin"
            slibext = ".a"
            dlibext = ".dylib"
        elif system in ["linux"]:
            prefix = "lib"
            suffix = ""
            slibext = ".a"
            dlibext = ".dylib"
        else:
            prefix = "lib"
            suffix = f"-{system}"
            slibext = ".a"
            dlibext = ".so"

        variables = [
            ["$GENARCH$", architecture],
            ["$BLDARCH$", platform],
            ["$CONFIG$", configuration],
            ["$PREFIX$", prefix],
            ["$SUFFIX$", suffix],
            ["$SLIBEXT$", slibext],
            ["$DLIBEXT$", dlibext],
        ]
        
        for variable in variables:
            script = script.replace(variable[0], variable[1])

        lines = script.split('\n')
        type = lines.pop(0)
        if type == "!deplist":
            self.deps: list[DepScript] = []
            for script in lines:
                if len(script) > 0:
                    path = os.path.normpath(
                                    os.path.abspath(
                                        os.path.join(
                                            os.path.dirname(path), script
                                        )
                                    )
                                )
                    self.deps.append(
                        DepScript(script_name(path, system, architecture),
                            system, platform, architecture, configuration))
        elif type == "!depscript":
            self.deps: list[self.Dependency] = []
            for dep in lines:
                if len(dep) > 0:
                    self.deps.append(self.Dependency(dep))

    def __str__(self) -> str:
        _ret = ''
        for dep in self.deps:
            _ret = f"{_ret}, {dep}" if len(_ret) > 0 else f"{dep}"
        return _ret

    class Dependency:
        def __init__(self, system: AnyStr = "", platform: AnyStr = "",
                     architecture: AnyStr = "", configuration: AnyStr = "",
                     source: AnyStr = "", destination: AnyStr = ""):
            self.system = system
            self.platform = platform
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
            self.destination = expr[1] if len(expr[1]) else expr[0]

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
