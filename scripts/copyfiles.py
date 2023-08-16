import os
import shutil
import sys

import depscript

def main():
    if len(sys.argv) < 7:
        print(f"{sys.argv[0]} <output directory> <system> <platform>",
              f" <architecture> <configuration> <depscript 1> [depscript 2]"
              f" ...")

    root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    output_dir = os.path.abspath(sys.argv[1])
    system = sys.argv[2]
    platform = sys.argv[3]
    architecture = sys.argv[4]
    configuration = sys.argv[5]

    print(f"Copying dependencies for {system} {configuration} "
          f"{platform} {architecture} from {root_dir} to {output_dir}")
    deps = []
    for i in range(6, len(sys.argv)):
        script = depscript.script_name(
            os.path.normpath(
                os.path.join(root_dir, "depscripts", sys.argv[i])
            ), system, architecture
        )
        print(f"Processing depscript {script}")
        deps += depscript.DepScript(script, system, platform, architecture,
                                    configuration).deps

    def copydeps(deps):
        print(f"Copying {len(deps)} dependencies")
        for dep in deps:
            if type(dep) == depscript.DepScript.Dependency:
                if dep.matches(system, platform, configuration):
                    source = os.path.normpath(os.path.join(root_dir, dep.source))
                    destination = os.path.normpath(os.path.join(output_dir,
                                                                dep.destination))
                    print(f"Copying {source} -> {destination}")
                    shutil.copy2(source, destination)
            elif type(dep) == depscript.DepScript:
                copydeps(dep.deps)
    
    copydeps(deps)

    print("Done")

if __name__ == "__main__":
    main()
