#!/usr/bin/env python
import argparse
import os
import platform
import shutil

import depscript

# Note: this script was polished a little by ChatGPT

def copy_dependency(source, destination):
    if os.path.isdir(source):
        shutil.copytree(source, destination, True, dirs_exist_ok=True)
    elif os.path.exists(source):
        if not os.path.isdir(os.path.dirname(destination)):
            os.makedirs(os.path.dirname(destination))
        shutil.copy2(source, destination)
    else:
        print(f"Couldn't find {source} to copy to {destination}")

def get_default_architecture(system):
    machine = platform.uname().machine.lower()

    if system == "windows":
        return "x86"
    elif system == "gaming_desktop":
        return "x86_64"
    elif system == "scarlett":
        return "x86_64"
    elif system == "macosx":
        if "arm64" in machine:
            return "ARM64"
        else:
            return "Universal"
    elif "armv7" in machine or "armv6" in machine:
        return "ARM"
    elif "arm64" in machine:
        return "ARM64"
    elif "x86_64" in machine:
        return "x86_64"
    elif "i386" in machine or "i686" in machine:
        return "x86"
    else:
        return machine

def get_default_platform(system):
    if system == "gaming_desktop":
        return "Gaming.Desktop.x64"
    elif system == "scarlett":
        return "Gaming.Xbox.Scarlett.x64"
    else:
        return ""
    
def get_system():
    if platform.system() == "Windows":
        return "gaming_desktop"
    elif platform.system() == "Darwin":
        return "macosx"
    else:
        return platform.system().lower()
    
def main():
    parser = argparse.ArgumentParser(description="Copy dependencies to an output directory.")
    parser.add_argument("output_directory", metavar="output_directory", help="Output directory")
    parser.add_argument("depscripts", metavar="depscripts", nargs="+", help="List of dep scripts (see depscripts/, you just need the base name of it)")
    parser.add_argument("--system", "-s", metavar="system", help="System (see premake5 --os)", default="")
    parser.add_argument("--platform", "-p", metavar="platform", help="Platform (what the build system sees)", default="")
    parser.add_argument("--architecture", "-a", metavar="architecture", help="Architecture (x86, x86_64, ARM64, Universal)", default="")
    parser.add_argument("--configuration", "-c", metavar="configuration", help="Configuration", default="Debug")
    parser.add_argument("--dirty", "-d", action="store_true", help="Skip copying if file exists at destination")
    
    args = parser.parse_args()

    root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    output_dir = os.path.abspath(args.output_directory)
    system = args.system if args.system else get_system()
    platform = args.platform if args.platform else get_default_platform(system)
    architecture = args.architecture if args.architecture else get_default_architecture(system)
    configuration = args.configuration

    print(f"Copying dependencies for {system} {configuration} "
          f"{platform} {architecture} from {root_dir} to {output_dir}")

    deps = []
    for dep_script in args.depscripts:
        script = depscript.script_name(
            os.path.normpath(
                os.path.join(root_dir, "depscripts", dep_script)
            ), system, architecture
        )
        print(f"Processing depscript {script}")
        deps += depscript.DepScript(script, system, platform, architecture,
                                    configuration).deps

    def copy_deps(deps):
        print(f"Copying {len(deps)} dependencies")
        for dep in deps:
            if isinstance(dep, depscript.DepScript.Dependency):
                if dep.matches(system, platform, configuration):
                    source = os.path.normpath(os.path.join(root_dir, dep.source))
                    destination = os.path.normpath(os.path.join(output_dir, dep.destination))
                    if not args.dirty or not os.path.exists(destination):
                        print(f"Copying {source} -> {destination}")
                        copy_dependency(source, destination)
                    if args.dirty and os.path.exists(destination):
                        print(f"Skipping {source} -> {destination}")
            elif isinstance(dep, depscript.DepScript):
                copy_deps(dep.deps)

    copy_deps(deps)

    print("Done")

if __name__ == "__main__":
    main()