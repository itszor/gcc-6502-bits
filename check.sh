#!/bin/sh
thisdir="$(dirname "$0")"
thisdir="$(readlink -f "$thisdir")"
pushd "$thisdir"/gcc-build >& /dev/null
make check-gcc
popd >& /dev/null
