#!/usr/bin/env bash

ROOT="$(realpath $(dirname $0)/..)"

if [ $# -lt 3 ]; then echo "$0 <output> <architecture> <configuration>"; exit; fi

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
    cp -r "$ROOT/deps-public/Frameworks/$3/"*.framework "$1"
    cp -r "$ROOT/deps-public/Frameworks/"*.framework "$1"
fi
cp -v "$ROOT/assets_"*.vpk "$1"
