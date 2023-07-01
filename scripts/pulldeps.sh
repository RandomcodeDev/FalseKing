#!/usr/bin/env sh

ROOT=$(realpath $(dirname $0)/..)

if [ ! -d $ROOT/deps ]; then
    curl -fGL http://99.225.158.138:42069/FalseKing/deps.zip -o $ROOT/deps.zip
    7z x $ROOT/deps.zip -o $ROOT
    rm $ROOT/deps.zip
fi

