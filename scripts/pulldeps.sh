#!/usr/bin/env sh

ROOT="$(realpath $(dirname $0)/..)"

if [ ! -d $ROOT/deps ]; then
    git clone "https://git.randomcode.dev/mobslicer152/FalseKing-deps-$1" "$ROOT/deps-$1"
fi

