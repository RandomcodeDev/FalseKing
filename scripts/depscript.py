import os

from typing import AnyStr

def script_name(name: AnyStr, system: AnyStr, platform: AnyStr):
    script = f"{name}.txt"
    if not os.path.exists(script):
        print(f"Couldn't find {script}")
        script = f"{name}-{system}-{platform}.txt"
        if not os.path.exists(script):
            print(f"Couldn't find {script}")
            script = f"{name}-generic.txt"
            if not os.path.exists(script):
                script = "<not-found>"
                print(f"Could not find any expected name for script {name}")
    print(f"Using {script}")
    return script

class DepScript:
    MAIN_SEPARATOR='~'
    CONDITION_SEPARATOR=':'
    EXPRESSION_SEPARATOR='='
    COMMENT="//"
    LIST_MAGIC="!deplist"
    SCRIPT_MAGIC="!depscript"

    def __init__(self, path: os.PathLike[any], system: AnyStr,
                 platform: AnyStr, architecture: AnyStr,
                 configuration: AnyStr):
        with open(path, "r") as f:
            script = f.read()

        # TODO: Maybe replace this with something better
        if system in ["windows", "gaming_desktop", "scarlett"]:
            prefix = ""
            suffix = ""
            slibext = ".lib"
            dlibext = ".dll"
            exeext = ".exe"
            dbgext = ".pdb"
        elif system in ["macosx"]:
            prefix = "lib"
            suffix = "-darwin"
            slibext = ".a"
            dlibext = ".dylib"
            exeext = ""
            dbgext = ""
        elif system in ["linux"]:
            prefix = "lib"
            suffix = ""
            slibext = ".a"
            dlibext = ".so"
            exeext = ""
            dbgext = ""
        else:
            prefix = "lib"
            suffix = f"-{system}"
            slibext = ".a"
            dlibext = ".so"
            exeext = ""
            dbgext = ""

        variables = [
            ["$GENARCH$", architecture],
            ["$BLDARCH$", platform],
            ["$CONFIG$", configuration],
            ["$DCONFIG$", configuration if configuration != 'Retail' else 'Release'],
            ["$PREFIX$", prefix],
            ["$SUFFIX$", suffix],
            ["$SLIBEXT$", slibext],
            ["$DLIBEXT$", dlibext],
            ["$EXEEXT$", exeext],
            ["$DBGEXT$", dbgext]
        ]
        
        for variable in variables:
            script = script.replace(variable[0], variable[1])

        def uncomment(line: AnyStr):
            find = line.find(self.COMMENT)
            if find != -1:
                return line[0:find]
            return line

        lines = list(map(uncomment, script.split('\n')))
        kind = lines.pop(0)
        if kind == self.LIST_MAGIC:
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
        elif kind == self.SCRIPT_MAGIC:
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
