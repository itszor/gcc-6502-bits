#!/bin/sh
thisdir="$(dirname "$0")"
thisdir="$(readlink -f "$thisdir")"
cd "$thisdir"
rm -rf gcc-build
mkdir gcc-build
pushd gcc-build
../gcc-src/configure --prefix="$thisdir/prefix" --target=6502 --enable-languages=c --with-as=/usr/bin/ca65 --with-ld=/usr/bin/ld65 --without-headers --with-newlib --disable-nls --disable-decimal-float --disable-libssp --disable-threads --disable-libatomic --disable-libitm --disable-libsanitizer --disable-libquadmath --disable-lto --enable-sjlj-exceptions
DEBUG_FLAGS="-O0 -g"
make -j 8 BOOT_CFLAGS="$DEBUG_FLAGS" CFLAGS="$DEBUG_FLAGS" CXXFLAGS="$DEBUG_FLAGS" all-gcc
make install-gcc
popd
