#!/bin/sh
cd "$(dirname $0)"
6502-gcc -O2 -nostdlib tinyc.c -c
ar65 a libtinyc.a tinyc.o
mkdir -p $PREFIX/usr/lib
cp -f libtinyc.a $PREFIX/usr/lib
cp -rf include/* $PREFIX/include
