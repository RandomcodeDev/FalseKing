#!/usr/bin/env sh

ROOT="$(realpath $(dirname $0)/..)"

if [ $# -lt 3 ]; then exit; fi

ENDING=".so"
case $(uname) in
    Darwin)
        ENDING=".dylib"
        ;;
esac

echo Copying libraries and assets
mkdir -p "$1"
cp "$ROOT/deps-public/lib/$2/$3/"*$ENDING "$1"
cp "$ROOT/deps-public/lib/$2/"*$ENDING "$1"
if [ $(uname) == Darwin ];
    cp -r "$ROOT/deps-public/Frameworks/$3/*.framework" "$1"
    cp -r "$ROOT/deps-public/Frameworks/*.framework" "$1"
fi
cp -r "$ROOT/assets" "$1"
