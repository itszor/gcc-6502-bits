#!/bin/sh
DIR=$(dirname "$1")
while read line; do
  echo $line | sed 's,\([^.]*.o\):,'"$DIR"'/\1:,g'
done
