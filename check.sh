#!/bin/bash

declare -a BOARDS

PARALLELISM=8

# DejaGNU's "schedule of variations" multi-board support appears to not be
# very useful for us, so just run one board at a time.
BOARDS=("semi65x-sim" "semi65cx-sim")

get_test_results_dir () {
  local board=$1
  local base
  local n=1
  base="testresults-$(date +%F)"
  while [ -d "$base-$n" ]; do
    n=$(( n + 1 ))
  done
  mkdir -p "$base-$n"
  echo "$base-$n"
}

thisdir="$(dirname "$0")"
thisdir="$(readlink -f "$thisdir")"

resdir=$(get_test_results_dir)

for board in "${BOARDS[@]}"; do
    pushd "$thisdir"/gcc-build-2 >& /dev/null
    sed "s,BOARD_DIR_LOCATION,$thisdir/semi65x," < "$thisdir/semi65x/site-orig.exp" > "$thisdir/semi65x/site.exp"
    export DEJAGNU="$thisdir/semi65x/site.exp"
    export PATH="$thisdir/semi65x":$PATH
    make -j "$PARALLELISM" check-gcc RUNTESTFLAGS="--target_board=$board $*"
    popd >& /dev/null
    echo "Copying results to $resdir/$board"
    mkdir -p "$resdir"/"$board"
    cp "$thisdir/gcc-build-2/gcc/testsuite/gcc/gcc.sum" "$thisdir/gcc-build-2/gcc/testsuite/gcc/gcc.log" "$resdir"/"$board"
done
