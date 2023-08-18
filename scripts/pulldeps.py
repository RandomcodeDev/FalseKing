#!/usr/bin/env python3

import argparse
import os
import shutil
import stat
import subprocess

def main():
    parser = argparse.ArgumentParser(description="Download and set up dependencies")
    parser.add_argument("--dependency-kind", "-k", metavar="dependency_kind", help="The dependency repository to download", default="public")
    parser.add_argument("--base-url", "-b", metavar="base_url", help="The base URL to use", default="https://git.randomcode.dev/mobslicer152/FalseKing-deps-")
    parser.add_argument("--clean", "-c", action="store_true", help="Re-clone the repository instead of just pulling it")

    args = parser.parse_args()

    kind = args.dependency_kind
    base_url = args.base_url
    clean = args.clean

    root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), os.path.pardir))
    deps_path = os.path.join(root_dir, f"deps-{kind}")
    deps_url = f"{base_url}{kind}"

    git_clone_command = ["git", "clone", "--recursive", "--depth=1", deps_url, deps_path]
    git_update_commands = [["git", "pull"], ["git", "submodule", "foreach", "git pull origin HEAD"]]

    def del_rw(action, name, exc):
        os.chmod(name, stat.S_IWRITE)
        os.remove(name)

    results = []
    if os.path.exists(deps_path):
        if clean:
            print(f"Re-cloning {deps_url} into {deps_path}")
            shutil.rmtree(deps_path, onerror=del_rw)
            results.append(subprocess.Popen(git_clone_command, cwd=root_dir).wait())
        else:
            print(f"Pulling {deps_url} in {deps_path}")
            for command in git_update_commands:
                results.append(subprocess.Popen(command, cwd=deps_path).wait())
    else:
        print(f"Cloning {deps_url} into {deps_path}")
        results.append(subprocess.Popen(git_clone_command, cwd=root_dir).wait())

    for root, _, files in os.walk(os.path.join(deps_path, "bin")):
        for file in files:
            file = os.path.join(root, file)
            print(f"Making {file} executable")
            os.chmod(file, stat.S_IEXEC)

    print(f"Git results: {results}")

if __name__ == "__main__":
    main()
