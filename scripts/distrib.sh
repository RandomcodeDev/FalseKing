#!/usr/bin/env sh

ROOT=$(dirname $0)/..

rm $4.tar.xz
$ROOT/scripts/copyfiles.sh $1 $2 $3
cp $ROOT/$2/$3/Game.$2 $1
tar cvJf $4.tar.xz -C $1 $(ls $1)

