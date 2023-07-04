#!/usr/bin/env sh

ROOT="$(realpath $(dirname $0)/..)"

rm -r "$1"
rm "$4.zip"
mkdir -p "$1"
$ROOT/scripts/copyfiles.sh "$1" "$2" "$3"
cp "$ROOT/build/unix/$2/$3/Game.$2" "$1"
OUTDIR=$(pwd)
cd "$1"
7z a -tzip "$OUTDIR/$4.zip" *
cd "$OUTDIR"
rm -r "$1"
