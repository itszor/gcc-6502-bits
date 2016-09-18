#!/bin/bash

declare -a BOARDS

PARALLELISM=8

# DejaGNU's "schedule of variations" multi-board support appears to not be
# very useful for us, so just run one board at a time.
BOARD="semi65x-sim"
#BOARD="semi65cx-sim"

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
make -j "$PARALLELISM" check-gcc RUNTESTFLAGS="--target_board=$BOARD $*"
popd >& /dev/null
resdir=$(get_test_results_dir)
echo "Copying results to $resdir"
cp "$thisdir/gcc-build-2/gcc/testsuite/gcc/gcc.sum" "$thisdir/gcc-build-2/gcc/testsuite/gcc/gcc.log" "$resdir"
