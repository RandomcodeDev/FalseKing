#!/usr/bin/env sh

ROOT="$(realpath $(dirname $0)/..)"

if [ $# -lt 1 ]; then exit; fi

if [ ! -d "$ROOT/deps-$1" ]; then
    git clone --depth=1 "https://git.randomcode.dev/mobslicer152/FalseKing-deps-$1" "$ROOT/deps-$1"
else
    pushd "$ROOT/deps-$1"
    git pull
    popd
fi

chmod a+x "$ROOT/deps-$1/bin/*"

