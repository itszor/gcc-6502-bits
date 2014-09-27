#!/bin/sh

get_test_results_dir () {
  local base
  local n=1
  base="testresults-$(date +%F)"
  while [ -d "$base-$n" ]; do
    n=$(( n + 1 ))
  done
  mkdir "$base-$n"
  echo "$base-$n"
}

thisdir="$(dirname "$0")"
thisdir="$(readlink -f "$thisdir")"
pushd "$thisdir"/gcc-build-2 >& /dev/null
sed "s,BOARD_DIR_LOCATION,$thisdir/semi65x," < "$thisdir/semi65x/site-orig.exp" > "$thisdir/semi65x/site.exp"
export DEJAGNU="$thisdir/semi65x/site.exp"
export PATH="$thisdir/semi65x":$PATH
make check-gcc RUNTESTFLAGS="--target_board=semi65x-sim $*"
popd >& /dev/null
resdir=$(get_test_results_dir)
echo "Copying results to $resdir"
cp "$thisdir/gcc-build-2/gcc/testsuite/gcc/gcc.sum" "$thisdir/gcc-build-2/gcc/testsuite/gcc/gcc.log" "$resdir"
