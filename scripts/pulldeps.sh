#!/usr/bin/env sh

ROOT="$(realpath $(dirname $0)/..)"

if [ ! -d $ROOT/deps ]; then
    git clone "http://99.225.158.138:3000/mobslicer152/FalseKing-deps-$1" "$ROOT/deps-$1"
fi

