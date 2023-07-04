#!/usr/bin/env sh
ROOT=$(realpath $(dirname $0)/..)
rm -rf $ROOT/build/unix/{ARM64,x64,Debug,Release}
