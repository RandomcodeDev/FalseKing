#!/usr/bin/env sh

ROOT=$(realpath $(dirname $0)/..)

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
FreeBSD)
    DLIBEXT=-freebsd.so
    SLIBEXT=-freebsd.a
esac

PWD=$(pwd)
BUILDDIR=$ROOT/build/unix/$ARCH/$CONF

echo Building $CONF for $ARCH in $BUILDDIR
mkdir -p $BUILDDIR
cd $BUILDDIR
bmake -j$(nproc) -f $ROOT/build/unix/Game.mak ARCH=$ARCH CONFIG=$CONF ROOT=$ROOT DLIBEXT=$DLIBEXT SLIBEXT=$SLIBEXT CC=clang++ CXX=clang++ LDFLAGS=-fuse-ld=lld $3 $4 $5 $6 $7
$ROOT/scripts/copyfiles.sh $BUILDDIR $ARCH $CONF || true
cd $PWD
echo Done
