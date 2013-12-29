#!/bin/sh
thisdir="$(dirname "$0")"
thisdir="$(readlink -f "$thisdir")"
pushd "$thisdir"/gcc-build-2 >& /dev/null
sed "s,BOARD_DIR_LOCATION,$thisdir/semi65x," < "$thisdir/semi65x/site-orig.exp" > "$thisdir/semi65x/site.exp"
export DEJAGNU="$thisdir/semi65x/site.exp"
export PATH="$thisdir/semi65x":$PATH
make check-gcc RUNTESTFLAGS="--target_board=semi65x-sim $@"
popd >& /dev/null
