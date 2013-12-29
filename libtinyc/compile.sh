#!/bin/sh
cd "$(dirname $0)"

# Build tiny C library.
rm -f *.o libtinyc.a
6502-gcc -O2 -nostdlib exit.c -c
ar65 a libtinyc.a exit.o

# Build tiny maths library.
rm -f libm.a
6502-gcc -O2 -nostdlib math.c -c
ar65 a libm.a math.o

mkdir -p $PREFIX/usr/lib
cp -f libtinyc.a $PREFIX/usr/lib
cp -f libm.a $PREFIX/usr/lib
mkdir -p $PREFIX/usr/include
cp -rf include/* $PREFIX/usr/include
