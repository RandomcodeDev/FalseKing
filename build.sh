#!/usr/bin/env sh

CONF=$1
if [ ! $CONF ]; then
    CONF=Debug
fi

mkdir -p $(./arch.sh)/$CONF
bmake -f BSDMakefile CONFIG=$CONF CC=clang++ CXX=clang++ LDFLAGS=-fuse-ld=lld MAKEOBJDIR=$(./arch.sh)/$CONF $2 $3 $4 $5 $6

