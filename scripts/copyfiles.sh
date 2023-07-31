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
mkdir -pv "$1"
cp -v "$ROOT/deps-public/lib/$2/$3/"*$ENDING "$1"
cp -v "$ROOT/deps-public/lib/$2/"*$ENDING "$1"
if [ $(uname) == Darwin ]; then
    cp -rv "$ROOT/deps-public/Frameworks/$3/*.framework" "$1"
    cp -rv "$ROOT/deps-public/Frameworks/*.framework" "$1"
fi
cp -rv "$ROOT/assets" "$1"
