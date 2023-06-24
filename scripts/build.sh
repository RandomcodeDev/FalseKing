#!/usr/bin/env sh

ROOT=$(dirname $0)/..

ARCH=$1
if [ ! $ARCH ]; then
    ARCH=$($ROOT/scripts/arch.sh)
fi

CONF=$2
if [ ! $CONF ]; then
    CONF=Debug
fi

case $(uname) in
Linux)
    DLIBEXT=.so
    SLIBEXT=.a
    ;;
Darwin)
    DLIBEXT=.dylib
    SLIBEXT=-darwin.a
    ;;
FreeBSD)
    DLIBEXT=-freebsd.so
    SLIBEXT=-freebsd.a
esac

mkdir -p $ARCH/$CONF
bmake -C $ROOT -f Game-unix.mak ARCH=$ARCH CONFIG=$CONF DLIBEXT=$DLIBEXT SLIBEXT=$SLIBEXT CC=clang++ CXX=clang++ LDFLAGS=-fuse-ld=lld MAKEOBJDIR=$($ROOT/scripts/arch.sh)/$CONF $3 $4 $5 $6 $7

