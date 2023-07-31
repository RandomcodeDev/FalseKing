#!/usr/bin/env bash

ROOT="$(realpath $(dirname $0)/..)"

if [ $# -lt 3 ]; then exit; fi

ENDING=".so"
case $(uname) in
    Darwin)
        ENDING=".dylib"
        ;;
esac

echo Copying libraries and assets
mkdir -pv "$1"
cp -v "$ROOT/deps-public/lib/$2/$3/"*$ENDING "$1"
cp -v "$ROOT/deps-public/lib/$2/"*$ENDING "$1"
if [ $(uname) == Darwin ]; then
    for i in $(ls -d "$ROOT/deps-public/Frameworks" "$ROOT/deps-public/Frameworks/$3" | grep '\.framework'); do cp -rv "$i" "$1"; done
fi
cp -rv "$ROOT/assets" "$1"
rm "$1/assets/.git"
