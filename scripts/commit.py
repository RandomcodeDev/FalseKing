#!/usr/bin/env python3

# Generated from commit.sh by ChatGPT

import sys
import subprocess
import os

if len(sys.argv) < 2:
    print(f"{sys.argv[0]} <output directory>")
    sys.exit()

commit = subprocess.check_output(["git", "log", "-n", "1", "--pretty=format:'%H'"], text=True).strip("'")
commit = f"\"{commit}\""
last = ""

if os.path.exists(f"{sys.argv[1]}/commit.txt"):
    with open(f"{sys.argv[1]}/commit.txt", "r") as f:
        last = f.read().strip()

if commit != last:
    print(f"Updating commit hash ({commit} vs {last})")
    os.makedirs(sys.argv[1], exist_ok=True)
    with open(f"{sys.argv[1]}/commit.txt", "w") as f:
        f.write(commit)