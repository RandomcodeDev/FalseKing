#!/usr/bin/env sh

ROOT=$(dirname $0)/..

CONF=$1
if [ ! $CONF ]; then
    CONF=Debug
fi

mkdir -p $($ROOT/scripts/arch.sh)/$CONF
bmake -C $ROOT -f BSDMakefile CONFIG=$CONF CC=clang++ CXX=clang++ LDFLAGS=-fuse-ld=lld MAKEOBJDIR=$($ROOT/scripts/arch.sh)/$CONF $2 $3 $4 $5 $6

