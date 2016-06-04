#!/bin/bash
thisdir="$(dirname "$0")"
thisdir="$(readlink -f "$thisdir")"
cd "$thisdir"

startpos=
endpos=3
case "$1" in
  ""|"1")
    startpos=1
    ;;
  "2")
    startpos=2
    ;;
  "3")
    startpos=3
    ;;
  *-*)
    startpos="${1%-*}"
    endpos="${1#*-}"
    ;;
  *)
    echo "Don't know what '$1' means"
    echo "Use numeric start stage:"
    echo "  1 - stage 1 compiler"
    echo "  2 - C library"
    echo "  3 - stage 2 compiler"
    exit 1
    ;;
esac

#MAKETARGET=all-gcc
#INSTALLTARGET=install-gcc

DEBUG_FLAGS="-O0 -g"
MAKETARGET=all
INSTALLTARGET=install
PARALLELISM="-j 4"

if [ "$startpos" -le 1 ]; then
rm -rf gcc-build
mkdir gcc-build
echo
echo "*****************************"
echo "* Building stage 1 compiler *"
echo "*****************************"
echo
pushd gcc-build
../gcc-src/configure --prefix="$thisdir/prefix" --with-sysroot="$thisdir/prefix/6502" --with-build-sysroot="$thisdir/prefix/6502" --target=6502 --enable-languages=c --with-as=/usr/bin/ca65 --with-ld=/usr/bin/ld65 --without-headers --with-newlib --disable-nls --disable-decimal-float --disable-libssp --disable-threads --disable-libatomic --disable-libitm --disable-libsanitizer --disable-libquadmath --disable-lto --enable-sjlj-exceptions --without-isl
set -e
make $PARALLELISM BOOT_CFLAGS="$DEBUG_FLAGS" CFLAGS="$DEBUG_FLAGS" CXXFLAGS="$DEBUG_FLAGS" AR_FOR_TARGET="$thisdir/wrappers/6502-ar" RANLIB_FOR_TARGET="$thisdir/wrappers/6502-ranlib" $MAKETARGET
make RANLIB_FOR_TARGET="$thisdir/wrappers/6502-ranlib" $INSTALLTARGET
set +e
popd
fi

if [ "$startpos" -le 2 ] && [ "$endpos" -ge 2 ]; then
echo
echo "***************************"
echo "* Building tiny C library *"
echo "***************************"
echo
set -e
PATH="$thisdir/prefix/bin:$PATH"
PREFIX="$thisdir/prefix" libtinyc/compile.sh
set +e
mkdir -p "$thisdir/prefix/6502/lib/cc65/cfg"
install ldscripts/semi65x.cfg "$thisdir/prefix/6502/lib/cc65/cfg"
mkdir -p "$thisdir/prefix/6502/bbcb/lib/cc65/cfg"
install ldscripts/bbcb.cfg "$thisdir/prefix/6502/bbcb/lib/cc65/cfg"
mkdir -p "$thisdir/prefix/6502/bbcm/lib/cc65/cfg"
install ldscripts/bbcmaster.cfg "$thisdir/prefix/6502/bbcm/lib/cc65/cfg"
mkdir -p "$thisdir/prefix/6502/c64/lib/cc65/cfg"
install ldscripts/c64.cfg "$thisdir/prefix/6502/c64/lib/cc65/cfg"
fi

if [ "$startpos" -le 3 ] && [ "$endpos" -ge 3 ]; then
echo
echo "*****************************"
echo "* Building stage 2 compiler *"
echo "*****************************"
echo
rm -rf gcc-build-2
mkdir gcc-build-2
pushd gcc-build-2
../gcc-src/configure --prefix="$thisdir/prefix" --with-sysroot="$thisdir/prefix/6502" --with-build-sysroot="$thisdir/prefix/6502" --target=6502 --enable-languages=c --with-as=/usr/bin/ca65 --with-ld=/usr/bin/ld65 --disable-nls --disable-decimal-float --disable-libssp --disable-threads --disable-libatomic --disable-libitm --disable-libsanitizer --disable-libquadmath --disable-lto --enable-sjlj-exceptions --without-isl
set -e
make $PARALLELISM BOOT_CFLAGS="$DEBUG_FLAGS" CFLAGS="$DEBUG_FLAGS" CXXFLAGS="$DEBUG_FLAGS" AR_FOR_TARGET="$thisdir/wrappers/6502-ar" RANLIB_FOR_TARGET="$thisdir/wrappers/6502-ranlib" $MAKETARGET
make RANLIB_FOR_TARGET="$thisdir/wrappers/6502-ranlib" $INSTALLTARGET
set +e
popd
fi
