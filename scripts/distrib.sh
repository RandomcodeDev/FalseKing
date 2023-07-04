#!/usr/bin/env sh

ROOT="$(realpath $(dirname $0)/..)"

rm -r "$1"
rm "$4.zip"
mkdir -p "$1"
$ROOT/scripts/copyfiles.sh "$1" "$2" "$3"
cp "$ROOT/build/unix/$2/$3/Game.$2" "$1"
OUTDIR=$(pwd)
pushd "$1"
7z a -tzip "$OUTDIR/$4.zip" *
popd
rm -r "$1"
