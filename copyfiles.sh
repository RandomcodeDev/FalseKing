#!/usr/bin/env sh

ROOT=$(dirname $0)

mkdir -p $1
cp $ROOT/deps/lib/$2/$3/lib*.so $1
cp $ROOT/deps/lib/$2/lib*.so $1
cp -r $ROOT/assets $1
