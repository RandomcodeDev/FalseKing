#!/usr/bin/env sh

ROOT="$(realpath $(dirname $0)/..)"

echo Copying libraries and assets
mkdir -p "$1"
cp "$ROOT/deps/lib/$2/$3/"*.so "$1"
cp "$ROOT/deps/lib/$2/"*.so "$1"
cp -r "$ROOT/assets" "$1"
