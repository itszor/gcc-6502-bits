#!/bin/bash

CC65_PATH=${CC65_PATH:-/usr/bin}
AR65_PATH=${CC65_PATH}/ar65

cd "$(dirname $0)"

if [ ! "$PREFIX" ]; then
  echo "\$PREFIX must be set to build libtinyc."
  exit 1
fi

TARGET=6502

declare -a MULTILIBS
SAVED_IFS=$IFS
IFS=$'
'
MULTILIBS=( $($TARGET-gcc -print-multi-lib) )
IFS=$SAVED_IFS

OBJECTS=(
  exit
  abort
  stdfiles
  fputc
  fputs
  putchar
  puts
  vfprintf
  fprintf
  vprintf
  printf
  sprintf
  vsprintf
  fflush
  memset
  memcpy
  memcmp
  strcmp
  strlen
  strcpy
  strchr
  strrchr
  memmove
  strncmp
  strncpy
  ftoa
  alloc
  fwrite
)

set -e
#set -x

mkdir -p $PREFIX/$TARGET/usr/include
cp -rf include/* $PREFIX/$TARGET/usr/include

# Return the OS-specific source directory to use for a given multilib "OS" dir
# given in $1.

machine_specific_dir ()
{
  local osdir=$1
  
  case "$osdir" in
    .)
      echo semi65x
      ;;
    bbc*)
      echo bbc
      ;;
    c64)
      echo c64
      ;;
    *)
      echo "Unknown OS dir: $osdir" 1>&2
      exit 1
  esac
}

# Given a machine (multilib OS dir) in $1 and the name of an object in $2,
# return the source file to use.  If a machine-specific version is present,
# use that, else use the default.  Also prefer assembly source over C source
# if available.

src_for_machine ()
{
  local osdir=$1
  local obj=$2
  local machinedir
  machinedir="$(machine_specific_dir "$osdir")"
  
  if [ -e "$machinedir/$obj.S" ]; then
    echo "$machinedir/$obj.S"
  elif [ -e "$machinedir/$obj.c" ]; then
    echo "$machinedir/$obj.c"
  elif [ -e "$obj.S" ]; then
    echo "$obj.S"
  elif [ -e "$obj.c" ]; then
    echo "$obj.c"
  else
    echo "No source for '$obj' for $machinedir!" 1>&2
    exit 1
  fi
}

expand_opts ()
{
  local opts=$1
  echo $opts | sed 's/@/ -/g'
}

for mlib in "${MULTILIBS[@]}"; do
  osdir="${mlib%;*}"
  opts="${mlib#*;}"
  opts=$(expand_opts $opts)

  mkdir -p "$osdir"

  # Build tiny C library.
  rm -f "$osdir"/*.o "$osdir/libtinyc.a"
  for obj in "${OBJECTS[@]}"
  do
    src="$(src_for_machine "$osdir" "$obj")"
    case "$src" in
      *.c)
        echo "$osdir: compile: $src"
	;;
      *.S)
        echo "$osdir: assemble: $src"
	;;
      *)
        echo "Unknown source type $src"
	exit 1
	;;
    esac
    $TARGET-gcc -Os -Wall -nostdlib -I include $opts "$src" -c -o "$osdir/$obj.o"
    ${AR65_PATH} a "$osdir/libtinyc.a" "$osdir/$obj.o"
  done

  # Build tiny maths library.
  rm -f "$osdir"/libm.a
  src="$(src_for_machine "$osdir" "math")"
  $TARGET-gcc -Os -nostdlib $opts "$src" -c -o "$osdir/math.o"
  ${AR65_PATH} a "$osdir/libm.a" "$osdir/math.o"

  mkdir -p "$PREFIX/$TARGET/$osdir/usr/lib/"
  cp -f "$osdir/libtinyc.a" "$PREFIX/$TARGET/$osdir/usr/lib"
  cp -f "$osdir/libm.a" "$PREFIX/$TARGET/$osdir/usr/lib"
done
