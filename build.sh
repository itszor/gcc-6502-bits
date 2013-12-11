#!/bin/sh
thisdir="$(dirname "$0")"
thisdir="$(readlink -f "$thisdir")"
cd "$thisdir"
rm -rf gcc-build
mkdir gcc-build
pushd gcc-build
../gcc-src/configure --prefix="$thisdir/prefix" --with-sysroot="$thisdir/prefix" --target=6502 --enable-languages=c --with-as=/usr/bin/ca65 --with-ld=/usr/bin/ld65 --without-headers --with-newlib --disable-nls --disable-decimal-float --disable-libssp --disable-threads --disable-libatomic --disable-libitm --disable-libsanitizer --disable-libquadmath --disable-lto --enable-sjlj-exceptions
DEBUG_FLAGS="-O0 -g"
#MAKETARGET=all-gcc
MAKETARGET=all
#INSTALLTARGET=install-gcc
INSTALLTARGET=install
set -e
make -j 8 BOOT_CFLAGS="$DEBUG_FLAGS" CFLAGS="$DEBUG_FLAGS" CXXFLAGS="$DEBUG_FLAGS" AR_FOR_TARGET="$thisdir/wrappers/6502-ar" RANLIB_FOR_TARGET="$thisdir/wrappers/6502-ranlib" $MAKETARGET
make RANLIB_FOR_TARGET="$thisdir/wrappers/6502-ranlib" $INSTALLTARGET
popd
PREFIX="$thisdir/prefix" libtinyc/compile.sh
