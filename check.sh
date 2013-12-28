#!/bin/sh
thisdir="$(dirname "$0")"
thisdir="$(readlink -f "$thisdir")"
pushd "$thisdir"/gcc-build-2 >& /dev/null
export DEJAGNU="$thisdir/semi65x/site.exp"
make check-gcc RUNTESTFLAGS="--target-board=semi65x-sim $@"
popd >& /dev/null
