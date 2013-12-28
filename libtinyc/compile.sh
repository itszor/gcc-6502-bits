#!/bin/sh
cd "$(dirname $0)"
rm -f *.o libtinyc.a
6502-gcc -O2 -nostdlib exit.c -c
ar65 a libtinyc.a exit.o
mkdir -p $PREFIX/usr/lib
cp -f libtinyc.a $PREFIX/usr/lib
mkdir -p $PREFIX/usr/include
cp -rf include/* $PREFIX/usr/include
