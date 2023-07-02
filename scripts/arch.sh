#!/usr/bin/env sh

case $(uname -m) in
x86_64)
    echo x64;;
aarch64)
    echo ARM64;;
*)
    echo unknown;;
esac

