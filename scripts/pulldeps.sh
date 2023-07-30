#!/usr/bin/env sh

ROOT="$(realpath $(dirname $0)/..)"

if [ $# -lt 1 ]; then exit; fi

ORIG=$CD

if [ ! -d "$ROOT/deps-$1" ]; then
    git clone --depth=1 "https://git.randomcode.dev/mobslicer152/FalseKing-deps-$1" "$ROOT/deps-$1"
else
    cd "$ROOT/deps-$1"
    git pull
    cd $ORIG
fi
